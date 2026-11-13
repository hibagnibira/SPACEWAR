// ============================================================================
// File: Player.cpp
// Description: Implementation of the Player spaceship class for SPACEWAR.
//              All movement physics, rotation math, collision constraints,
//              and rendering logic are entirely self-contained.
// ============================================================================
#include "Player.h"
#include <algorithm>
#include <cmath>

// ----------------------------------------------------------------------------
// Helper: 2D Vector Rotation
// Transforms a local point by rotating it around the origin (0, 0).
// ----------------------------------------------------------------------------
static inline Vector2 RotateLocalPoint(Vector2 point, float angleRadians)
{
    const float cosAngle = std::cos(angleRadians);
    const float sinAngle = std::sin(angleRadians);


    return Vector2{
        point.x * cosAngle - point.y * sinAngle,
        point.x * sinAngle + point.y * cosAngle
    };
}

// ----------------------------------------------------------------------------
// Constructor
// Initializes the spaceship's internal state completely at creation.
// ----------------------------------------------------------------------------
Player::Player(Vector2 startPosition, float worldWidth, float worldHeight)
    : m_position(startPosition)
    , m_velocity{ 0.0f, 0.0f }
    , m_rotation(0.0f)
    , m_acceleration(DEFAULT_ACCELERATION)
    , m_maxSpeed(DEFAULT_MAX_SPEED)
    , m_drag(DEFAULT_DRAG)
    , m_worldWidth(worldWidth)
    , m_worldHeight(worldHeight)
    , m_isThrustingForward(false)
    , m_energy(0.0f)              // starts empty -> auto-recharges
    , m_isAccelerating(false)
    , m_speedCap(DEFAULT_MAX_SPEED)
    , m_isAgainstBoundary(false)
    , m_lastClickTime(-1.0)
    , m_shootingTogglePending(false)
    , m_isShooting(false)
    , m_fireTimer(0.0f)
    , m_shotRequested(false)
    , m_frontShooting(false)   // default: Back Shooting
    , m_health(PLAYER_STARTING_HEALTH)
    , m_isDead(false)
    , m_spawnPosition(startPosition)
    , m_invulnerabilityTimer(0.0f)
{
}

// ----------------------------------------------------------------------------
// TakeDamage
// General-purpose damage entry point for all future damage sources.
// Dead players ignore further damage; health is clamped to [0, MAX].
// ----------------------------------------------------------------------------
void Player::TakeDamage(float damageAmount)
{
    // Dead players and invulnerable players ignore all damage.
    // Ignored hits do NOT restart the invulnerability timer.
    if (m_isDead || m_invulnerabilityTimer > 0.0f || damageAmount <= 0.0f)
    {
        return;
    }

    m_health -= damageAmount;

    if (m_health <= 0.0f)
    {
        m_health = 0.0f;
        m_isDead = true;

        // Death cleanup: stop all motion and cancel any active acceleration
        m_velocity = { 0.0f, 0.0f };
        m_isThrustingForward = false;
        m_isAccelerating = false;
    }
    else
    {
        // Survived the hit: grant the invulnerability window
        m_invulnerabilityTimer = INVULNERABILITY_DURATION;
    }
}

// ----------------------------------------------------------------------------
// Heal
// Restores health up to the maximum. Independent of the invulnerability
// system: healing neither grants nor resets protection.
// ----------------------------------------------------------------------------
void Player::Heal(float healAmount)
{
    if (m_isDead || healAmount <= 0.0f)
    {
        return;
    }

    m_health += healAmount;
    if (m_health > PLAYER_MAX_HEALTH)
    {
        m_health = PLAYER_MAX_HEALTH;
    }
}

// ----------------------------------------------------------------------------
// DrawHealthBar
// Screen-space HUD element drawn by the player itself so health stays
// encapsulated. Fixed position: it never moves with the camera.
// ----------------------------------------------------------------------------
void Player::DrawHealthBar() const
{
    constexpr int BAR_X = 20;
    constexpr int BAR_Y = 50;   // below the "SPACEWAR" title
    constexpr int BAR_WIDTH = 220;
    constexpr int BAR_HEIGHT = 16;

    // m_health is already clamped to [0, MAX], so fraction stays in [0,1]
    const float fraction = m_health / PLAYER_MAX_HEALTH;

    // Empty portion background + border
    DrawRectangle(BAR_X, BAR_Y, BAR_WIDTH, BAR_HEIGHT, Color{ 35, 40, 55, 255 });
    DrawRectangleLines(BAR_X, BAR_Y, BAR_WIDTH, BAR_HEIGHT, Color{ 90, 100, 130, 255 });

    // Filled portion, colored by remaining health
    const int fillWidth = static_cast<int>(BAR_WIDTH * fraction);
    Color fillColor = Color{ 80, 220, 100, 255 };   // green: healthy
    if (fraction <= 0.5f) fillColor = Color{ 240, 200, 60, 255 };  // yellow: hurt
    if (fraction <= 0.25f) fillColor = Color{ 235, 70, 70, 255 };  // red: critical

    if (fillWidth > 0)
    {
        DrawRectangle(BAR_X, BAR_Y, fillWidth, BAR_HEIGHT, fillColor);
    }

    // Numerical percentage next to the bar
    DrawText(TextFormat("%d%%", static_cast<int>(m_health)), BAR_X + BAR_WIDTH + 10, BAR_Y - 2, 16, RAYWHITE);
}

// ----------------------------------------------------------------------------
// DrawEnergyBar
// Screen-space HUD element for the acceleration energy meter.
// Fixed position below the health bar; never moves with the camera.
// ----------------------------------------------------------------------------
void Player::DrawEnergyBar() const
{
    constexpr int BAR_X = 20;
    constexpr int BAR_Y = 78;   // below the health bar
    constexpr int BAR_WIDTH = 220;
    constexpr int BAR_HEIGHT = 16;

    const float fraction = m_energy / ENERGY_MAX; // always in [0,1]

    // Empty portion background + border
    DrawRectangle(BAR_X, BAR_Y, BAR_WIDTH, BAR_HEIGHT, Color{ 35, 40, 55, 255 });
    DrawRectangleLines(BAR_X, BAR_Y, BAR_WIDTH, BAR_HEIGHT, Color{ 90, 100, 130, 255 });

    const int fillWidth = static_cast<int>(BAR_WIDTH * fraction);

    // Color communicates state: blue = charging, orange = boosting,
    // green = ready to activate
    Color fillColor = Color{ 80, 140, 240, 255 };
    if (m_isAccelerating)            fillColor = Color{ 255, 160, 60, 255 };
    else if (m_energy >= ENERGY_MAX) fillColor = Color{ 80, 220, 100, 255 };

    if (fillWidth > 0)
    {
        DrawRectangle(BAR_X, BAR_Y, fillWidth, BAR_HEIGHT, fillColor);
    }

    // Percentage + READY hint when the bar is full
    DrawText(TextFormat("%d%%", static_cast<int>(m_energy)),
             BAR_X + BAR_WIDTH + 10, BAR_Y - 2, 16, RAYWHITE);
    if (m_energy >= ENERGY_MAX)
    {
        DrawText("READY [Space]", BAR_X, BAR_Y + BAR_HEIGHT + 4, 12, GREEN);
    }
}

// ----------------------------------------------------------------------------
// Respawn
// Full player-only reset after death. Does not touch bullets, enemies,
// camera, or world state — those belong to Game.
// ----------------------------------------------------------------------------
void Player::Respawn(const Camera2D& camera)
{
    // Position & health
    m_position = m_spawnPosition;
    m_health = PLAYER_STARTING_HEALTH;
    m_isDead = false;

    // Movement state
    m_velocity = { 0.0f, 0.0f };
    m_isThrustingForward = false;

    // Acceleration state: energy restarts empty, speed cap back to normal
    m_isAccelerating = false;
    m_energy = 0.0f;
    m_speedCap = m_maxSpeed;
    m_isAgainstBoundary = false;

    // Mouse click state: no pending toggles after respawn
    m_lastClickTime = -1.0;
    m_shootingTogglePending = false;

    // Shooting state: off until the player toggles it again
    m_isShooting = false;
    m_fireTimer = 0.0f;
    m_shotRequested = false;

    // Shooting mode resets to Back; continuous fire stays off
    m_frontShooting = false;

    // Invulnerability: immediately vulnerable again after respawn
    m_invulnerabilityTimer = 0.0f;

    // Immediately face the current mouse world position
    const Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
    const float faceX = mouseWorldPos.x - m_position.x;
    const float faceY = mouseWorldPos.y - m_position.y;
    if (std::hypot(faceX, faceY) > 0.001f)
    {
        m_rotation = std::atan2(faceY, faceX);
    }
}

// ----------------------------------------------------------------------------
// Update
// Encapsulates all per-frame input handling, physics integration, and bounds.
// ----------------------------------------------------------------------------
//modify update 
// Automatically steers toward the mouse cursor with smooth arrival deceleration,
// eliminating overshooting and oscillation without any keyboard input.
// ----------------------------------------------------------------------------
// Update
// Autonomous execution of Option A movement, world boundary clamping,
// and smooth camera target following without exposing private state.
// ----------------------------------------------------------------------------
void Player::Update(float dt, const Camera2D& camera)
{
    if (dt <= 0.0f)
    {
        return;
    }

    // Invulnerability countdown runs regardless of dead/alive state
    if (m_invulnerabilityTimer > 0.0f)
    {
        m_invulnerabilityTimer -= dt;
    }

    // Dead players ignore all gameplay input: no movement, acceleration, or shooting.
    // The ship stays frozen at its final position (Draw still renders it).
    if (m_isDead)
    {
        return;
    }

    // ------------------------------------------------------------------------
    // 1. Mouse Screen-to-World Coordinate Conversion
    // ------------------------------------------------------------------------
    const Vector2 mouseScreenPos = GetMousePosition();
    const Vector2 mouseWorldPos = GetScreenToWorld2D(mouseScreenPos, camera);

    const float diffX = mouseWorldPos.x - m_position.x;
    const float diffY = mouseWorldPos.y - m_position.y;
    const float dist = std::hypot(diffX, diffY);

    // ------------------------------------------------------------------------
    // 2. Acceleration Energy & Activation (Space = single press)
    // ------------------------------------------------------------------------
    // Space only activates when the energy bar is completely full.
    // Once active it runs until energy hits 0 — no holding required.
    if (IsKeyPressed(KEY_SPACE) && !m_isAccelerating && m_energy >= ENERGY_MAX)
    {
        m_isAccelerating = true;
    }

    if (m_isAccelerating)
    {
        // Energy drain pauses while the ship is pinned against a wall —
        // acceleration state and speed progression continue unchanged.
        if (!m_isAgainstBoundary)
        {
            m_energy -= ENERGY_CONSUMPTION_RATE * dt;
        }
        if (m_energy <= 0.0f)
        {
            m_energy = 0.0f;
            m_isAccelerating = false;
        }
    }
    else
    {
        m_energy += ENERGY_RECHARGE_RATE * dt;
        if (m_energy > ENERGY_MAX)
        {
            m_energy = ENERGY_MAX;
        }
    }

    // Ramp the speed ceiling toward its target — gradual, frame-rate
    // independent, and symmetric for spin-up and spin-down.
    const float targetSpeedCap = m_isAccelerating ? ACCEL_MAX_SPEED : m_maxSpeed;
    const float rampStep = ACCELERATION_RATE * dt;
    if (m_speedCap < targetSpeedCap)
    {
        m_speedCap = std::min(m_speedCap + rampStep, targetSpeedCap);
    }
    else
    {
        m_speedCap = std::max(m_speedCap - rampStep, targetSpeedCap);
    }

    // ------------------------------------------------------------------------
    // 3. Option A Movement (Direct Velocity, Zero Inertia/Drag)
    // ------------------------------------------------------------------------
    if (dist <= STOP_DISTANCE)
    {
        m_velocity = { 0.0f, 0.0f };
        m_isThrustingForward = false;
    }
    else
    {
        m_rotation = std::atan2(diffY, diffX);

        // Smooth speed ramp-down inside the slowing radius, capped by m_speedCap
        float currentSpeed = m_speedCap;
        if (dist < SLOWING_RADIUS)
        {
            currentSpeed = m_speedCap * (dist / SLOWING_RADIUS);
        }

        m_velocity.x = (diffX / dist) * currentSpeed;
        m_velocity.y = (diffY / dist) * currentSpeed;

        m_isThrustingForward = (currentSpeed > 20.0f);
    }

    // ------------------------------------------------------------------------
    // 4. Position Integration & Arrival Step Clamping
    // ------------------------------------------------------------------------
    const float stepDist = std::hypot(m_velocity.x * dt, m_velocity.y * dt);
    if (stepDist >= dist && dist > 0.0f)
    {
        m_position = mouseWorldPos;
        m_velocity = { 0.0f, 0.0f };
        m_isThrustingForward = false;
    }
    else
    {
        m_position.x += m_velocity.x * dt;
        m_position.y += m_velocity.y * dt;
    }

    // ------------------------------------------------------------------------
    // 5. World Boundary Enforcement
    // ------------------------------------------------------------------------
    const Vector2 preClampPosition = m_position;

    if (m_position.x < SHIP_COLLISION_RADIUS)
    {
        m_position.x = SHIP_COLLISION_RADIUS;
        m_velocity.x = 0.0f;
    }
    else if (m_position.x > (m_worldWidth - SHIP_COLLISION_RADIUS))
    {
        m_position.x = m_worldWidth - SHIP_COLLISION_RADIUS;
        m_velocity.x = 0.0f;
    }

    if (m_position.y < SHIP_COLLISION_RADIUS)
    {
        m_position.y = SHIP_COLLISION_RADIUS;
        m_velocity.y = 0.0f;
    }
    else if (m_position.y > (m_worldHeight - SHIP_COLLISION_RADIUS))
    {
        m_position.y = m_worldHeight - SHIP_COLLISION_RADIUS;
        m_velocity.y = 0.0f;
    }

    // True only when the clamp actually stopped this frame's movement,
    // i.e. the ship is pressing into a wall (not merely sliding along it)
    m_isAgainstBoundary = (m_position.x != preClampPosition.x ||
                           m_position.y != preClampPosition.y);

    // ------------------------------------------------------------------------
    // 6. Mouse Click Handling (single = shooting, double = acceleration)
    // ------------------------------------------------------------------------
    // A lone click can't toggle shooting immediately: we must wait to see
    // whether a second click arrives within DOUBLE_CLICK_WINDOW.
    const double now = GetTime();

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        if (m_lastClickTime >= 0.0 && (now - m_lastClickTime) <= DOUBLE_CLICK_WINDOW)
        {
            // Second click inside the window -> double-click gesture.
            // Consume it for acceleration and cancel the pending shot toggle
            // so the first click does NOT toggle shooting.
            m_shootingTogglePending = false;
            m_lastClickTime = -1.0;

            // Same activation rule as Space: full energy, not already active
            if (!m_isAccelerating && m_energy >= ENERGY_MAX)
            {
                m_isAccelerating = true;
            }
        }
        else
        {
            // First click of a potential pair: hold the shooting toggle
            m_lastClickTime = now;
            m_shootingTogglePending = true;
        }
    }

    // Window expired with no second click -> it was a genuine single click
    if (m_shootingTogglePending && (now - m_lastClickTime) > DOUBLE_CLICK_WINDOW)
    {
        m_isShooting = !m_isShooting;
        m_shootingTogglePending = false;
        m_lastClickTime = -1.0;
    }

    // S key: switch shooting mode once per press (does not affect firing state)
    if (IsKeyPressed(KEY_S))
    {
        m_frontShooting = !m_frontShooting;
    }

    if (m_isShooting)
    {
        m_fireTimer -= dt;
        if (m_fireTimer <= 0.0f)
        {
            m_fireTimer = FIRE_INTERVAL;
            m_shotRequested = true; // Game reads this flag (friend) and spawns the bullet
        }
    }
    else
    {
        m_fireTimer = 0.0f; // Reset so re-activating fires immediately
    }
}
// ----------------------------------------------------------------------------
// Draw
// Renders the internal visual representation of the ship and active thrusters.
// ----------------------------------------------------------------------------
void Player::Draw() const
{
    // ------------------------------------------------------------------------
    // Spaceship Hull Geometry (Local Space, 0 radians = pointing right)
    // ------------------------------------------------------------------------
    const Vector2 localNose = { 22.0f,   0.0f };
    const Vector2 localLeftWing = { -16.0f, -14.0f };
    const Vector2 localRightWing = { -16.0f,  14.0f };
    const Vector2 localRearIndent = { -10.0f,   0.0f };

    // Transform local vertices into world space based on position and rotation
    const Vector2 worldNose = { m_position.x + RotateLocalPoint(localNose, m_rotation).x,
                                      m_position.y + RotateLocalPoint(localNose, m_rotation).y };
    const Vector2 worldLeftWing = { m_position.x + RotateLocalPoint(localLeftWing, m_rotation).x,
                                      m_position.y + RotateLocalPoint(localLeftWing, m_rotation).y };
    const Vector2 worldRightWing = { m_position.x + RotateLocalPoint(localRightWing, m_rotation).x,
                                      m_position.y + RotateLocalPoint(localRightWing, m_rotation).y };
    const Vector2 worldRearIndent = { m_position.x + RotateLocalPoint(localRearIndent, m_rotation).x,
                                      m_position.y + RotateLocalPoint(localRearIndent, m_rotation).y };

    // ------------------------------------------------------------------------
    // Thruster Exhaust Rendering
    // ------------------------------------------------------------------------
    if (m_isThrustingForward)
    {
        // Outer flame
        const Vector2 localFlameTipOuter = { -28.0f, 0.0f };
        const Vector2 localFlameLeft = { -12.0f, -6.0f };
        const Vector2 localFlameRight = { -12.0f,  6.0f };

        const Vector2 worldFlameTipOuter = { m_position.x + RotateLocalPoint(localFlameTipOuter, m_rotation).x,
                                             m_position.y + RotateLocalPoint(localFlameTipOuter, m_rotation).y };
        const Vector2 worldFlameLeft = { m_position.x + RotateLocalPoint(localFlameLeft, m_rotation).x,
                                             m_position.y + RotateLocalPoint(localFlameLeft, m_rotation).y };
        const Vector2 worldFlameRight = { m_position.x + RotateLocalPoint(localFlameRight, m_rotation).x,
                                             m_position.y + RotateLocalPoint(localFlameRight, m_rotation).y };

        // Inner flame core
        const Vector2 localFlameTipInner = { -22.0f, 0.0f };
        const Vector2 localFlameLeftIn = { -12.0f, -3.0f };
        const Vector2 localFlameRightIn = { -12.0f,  3.0f };

        const Vector2 worldFlameTipInner = { m_position.x + RotateLocalPoint(localFlameTipInner, m_rotation).x,
                                             m_position.y + RotateLocalPoint(localFlameTipInner, m_rotation).y };
        const Vector2 worldFlameLeftIn = { m_position.x + RotateLocalPoint(localFlameLeftIn, m_rotation).x,
                                             m_position.y + RotateLocalPoint(localFlameLeftIn, m_rotation).y };
        const Vector2 worldFlameRightIn = { m_position.x + RotateLocalPoint(localFlameRightIn, m_rotation).x,
                                             m_position.y + RotateLocalPoint(localFlameRightIn, m_rotation).y };

        DrawTriangle(worldFlameTipOuter, worldFlameRight, worldFlameLeft, ORANGE);
        DrawTriangle(worldFlameTipInner, worldFlameRightIn, worldFlameLeftIn, YELLOW);
    }
   

    // ------------------------------------------------------------------------
    // Draw Spaceship Hull & Cockpit
    // High-contrast cyan body with crisp white-blue contour.
    // ------------------------------------------------------------------------
    const Color shipHullColor = Color{ 70, 190, 240, 255 }; // Bright Cyan
    const Color shipLineColor = Color{ 220, 245, 255, 255 }; // Crisp white/blue outline
    const Color shipCockpitColor = Color{ 15,  30,  60, 255 }; // Dark cockpit window

    // Two triangles form the arrowhead body
    DrawTriangle(worldNose, worldLeftWing, worldRearIndent, shipHullColor);
    DrawTriangle(worldNose, worldRearIndent, worldRightWing, shipHullColor);

    // Contours
    DrawLineV(worldNose, worldLeftWing, shipLineColor);
    DrawLineV(worldLeftWing, worldRearIndent, shipLineColor);
    DrawLineV(worldRearIndent, worldRightWing, shipLineColor);
    DrawLineV(worldRightWing, worldNose, shipLineColor);

    // Cockpit center node
    const Vector2 localCockpit = { 4.0f, 0.0f };
    const Vector2 worldCockpit = { m_position.x + RotateLocalPoint(localCockpit, m_rotation).x,
                                   m_position.y + RotateLocalPoint(localCockpit, m_rotation).y };
    DrawCircleV(worldCockpit, 3.5f, shipCockpitColor);
}
