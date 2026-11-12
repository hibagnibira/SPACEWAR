// ============================================================================
// File: Player.cpp
// Description: Implementation of the Player spaceship class for SPACEWAR.
//              All movement physics, rotation math, collision constraints,
//              and rendering logic are entirely self-contained.
// ============================================================================
#include "Player.h"
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
    , m_isDashing(false)
    , m_dashDirection{ 0.0f, 0.0f }
    , m_dashTimer(0.0f)
    , m_dashCooldownTimer(0.0f)
    , m_isShooting(false)
    , m_fireTimer(0.0f)
    , m_shotRequested(false)
    , m_health(PLAYER_STARTING_HEALTH)
    , m_isDead(false)
{
}

// ----------------------------------------------------------------------------
// EndDash
// Terminates the active dash (timer expiry or boundary hit) and begins the
// cooldown period during which another dash cannot be triggered.
// ----------------------------------------------------------------------------
void Player::EndDash()
{
    m_isDashing = false;
    m_dashTimer = 0.0f;
    m_dashCooldownTimer = DASH_COOLDOWN_DURATION;
}

// ----------------------------------------------------------------------------
// TakeDamage
// General-purpose damage entry point for all future damage sources.
// Dead players ignore further damage; health is clamped to [0, MAX].
// ----------------------------------------------------------------------------
void Player::TakeDamage(float damageAmount)
{
    if (m_isDead || damageAmount <= 0.0f)
    {
        return; // Already dead or invalid amount: no state change
    }

    m_health -= damageAmount;

    if (m_health <= 0.0f)
    {
        m_health = 0.0f;
        m_isDead = true;
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

    // ------------------------------------------------------------------------
    // 1. Mouse Screen-to-World Coordinate Conversion
    // ------------------------------------------------------------------------
    const Vector2 mouseScreenPos = GetMousePosition();
    const Vector2 mouseWorldPos = GetScreenToWorld2D(mouseScreenPos, camera);

    const float diffX = mouseWorldPos.x - m_position.x;
    const float diffY = mouseWorldPos.y - m_position.y;
    const float dist = std::hypot(diffX, diffY);

    // ------------------------------------------------------------------------
    // 2. Dash Input & Timers (Space = single press to activate)
    // ------------------------------------------------------------------------
    if (IsKeyPressed(KEY_SPACE) && !m_isDashing && m_dashCooldownTimer <= 0.0f)
    {
        m_isDashing = true;
        m_dashTimer = DASH_DURATION;

        // Lock the dash heading at activation time. If the mouse is too
        // close to compute a direction, fall back to current facing.
        if (dist > DASH_MIN_DIRECTION_DISTANCE)
        {
            m_dashDirection = { diffX / dist, diffY / dist };
        }
        else
        {
            m_dashDirection = { std::cos(m_rotation), std::sin(m_rotation) };
        }
    }

    if (m_isDashing)
    {
        m_dashTimer -= dt;
        if (m_dashTimer <= 0.0f)
        {
            EndDash(); // Dash finished -> cooldown begins
        }
    }

    if (m_dashCooldownTimer > 0.0f)
    {
        m_dashCooldownTimer -= dt;
    }

    // ------------------------------------------------------------------------
    // 3. Movement (Dash overrides Option A while active)
    // ------------------------------------------------------------------------
    if (m_isDashing)
    {
        // Spaceship still faces the mouse, but dash travel uses the stored
        // direction so it moves in a straight line.
        if (dist > STOP_DISTANCE)
        {
            m_rotation = std::atan2(diffY, diffX);
        }

        m_velocity.x = m_dashDirection.x * m_maxSpeed * DASH_SPEED_MULTIPLIER;
        m_velocity.y = m_dashDirection.y * m_maxSpeed * DASH_SPEED_MULTIPLIER;
        m_isThrustingForward = true;
    }
    else if (dist <= STOP_DISTANCE)
    {
        m_velocity = { 0.0f, 0.0f };
        m_isThrustingForward = false;
    }
    else
    {
        m_rotation = std::atan2(diffY, diffX);

        // Smooth speed ramp-down inside the slowing radius (capped at 420 px/s)
        float currentSpeed = m_maxSpeed;
        if (dist < SLOWING_RADIUS)
        {
            currentSpeed = m_maxSpeed * (dist / SLOWING_RADIUS);
        }

        m_velocity.x = (diffX / dist) * currentSpeed;
        m_velocity.y = (diffY / dist) * currentSpeed;

        m_isThrustingForward = (currentSpeed > 20.0f);
    }

    // ------------------------------------------------------------------------
    // 4. Position Integration & Arrival Step Clamping
    // ------------------------------------------------------------------------
    if (m_isDashing)
    {
        // Dash ignores the cursor: always travels its full step.
        m_position.x += m_velocity.x * dt;
        m_position.y += m_velocity.y * dt;
    }
    else
    {
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

    // If the boundary clamp moved the ship during a dash, the dash is cut
    // short immediately and the cooldown starts.
    if (m_isDashing &&
        (m_position.x != preClampPosition.x || m_position.y != preClampPosition.y))
    {
        EndDash();
    }

    // ------------------------------------------------------------------------
    // 6. Shooting (Left Mouse Button Toggle)
    // ------------------------------------------------------------------------
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        m_isShooting = !m_isShooting;
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
