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

    //basic maths duh
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
    , m_isThrustingReverse(false)
{
}

// ----------------------------------------------------------------------------
// Update
// Encapsulates all per-frame input handling, physics integration, and bounds.
// ----------------------------------------------------------------------------
void Player::Update(float dt)
{
    // Guard against non-positive delta times (e.g. paused execution or frame hitch)
    if (dt <= 0.0f)
    {
        return;
    }

    // ------------------------------------------------------------------------
    // 1. Instantaneous Mouse Aiming
    // The ship continuously calculates the directional vector to the mouse cursor.
    // ------------------------------------------------------------------------
    const Vector2 mousePos = GetMousePosition();
    const float diffX = mousePos.x - m_position.x;
    const float diffY = mousePos.y - m_position.y;

    m_rotation = std::atan2(diffY, diffX);

    // ------------------------------------------------------------------------
    // 2. Facing Unit Vector
    // Determines the forward axis along which thrust is applied.
    // ------------------------------------------------------------------------
    const Vector2 forward = { std::cos(m_rotation), std::sin(m_rotation) };

    // ------------------------------------------------------------------------
    // 3. Thrust Input & Acceleration
    // W: Accelerate forward.
    // S: Accelerate backward (reverse thrusters with identical force).
    // ------------------------------------------------------------------------
    m_isThrustingForward = IsKeyDown(KEY_W);
    m_isThrustingReverse = IsKeyDown(KEY_S);

    if (m_isThrustingForward)
    {
        m_velocity.x += forward.x * m_acceleration * dt;
        m_velocity.y += forward.y * m_acceleration * dt;
    }

    if (m_isThrustingReverse)
    {
        m_velocity.x -= forward.x * m_acceleration * dt;
        m_velocity.y -= forward.y * m_acceleration * dt;
    }

    // ------------------------------------------------------------------------
    // 4. Natural Drag / Velocity Damping
    // Gradually diminishes velocity over time without sudden stops, preserving
    // drift momentum when thrust keys are released.
    // ------------------------------------------------------------------------
    const float dragFactor = 1.0f - (m_drag * dt);
    if (dragFactor > 0.0f)
    {
        m_velocity.x *= dragFactor;
        m_velocity.y *= dragFactor;
    }
    else
    {
        m_velocity = { 0.0f, 0.0f };
    }

    // ------------------------------------------------------------------------
    // 5. Maximum Speed Clamping
    // Enforces the top speed limit without altering the direction of movement.
    // ------------------------------------------------------------------------
    const float speedSquared = (m_velocity.x * m_velocity.x) + (m_velocity.y * m_velocity.y);
    if (speedSquared > (m_maxSpeed * m_maxSpeed))
    {
        const float currentSpeed = std::sqrt(speedSquared);
        const float normalizer = m_maxSpeed / currentSpeed;

        m_velocity.x *= normalizer;
        m_velocity.y *= normalizer;
    }

    // ------------------------------------------------------------------------
    // 6. Position Integration
    // Integrates current velocity into position over the elapsed frame time.
    // ------------------------------------------------------------------------
    m_position.x += m_velocity.x * dt;
    m_position.y += m_velocity.y * dt;

    // ------------------------------------------------------------------------
    // 7. World Boundary Enforcement
    // The player prevents its own boundary violation autonomously.
    // ------------------------------------------------------------------------
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
    else if (m_isThrustingReverse)
    {
        // Forward-facing retro thruster jets
        const Vector2 localRevLeftTip = { 4.0f, -12.0f };
        const Vector2 localRevLeftA = { -2.0f, -10.0f };
        const Vector2 localRevLeftB = { -2.0f, -14.0f };

        const Vector2 localRevRightTip = { 4.0f,  12.0f };
        const Vector2 localRevRightA = { -2.0f,  14.0f };
        const Vector2 localRevRightB = { -2.0f,  10.0f };

        const Vector2 wRevLTip = { m_position.x + RotateLocalPoint(localRevLeftTip, m_rotation).x,
                                   m_position.y + RotateLocalPoint(localRevLeftTip, m_rotation).y };
        const Vector2 wRevLA = { m_position.x + RotateLocalPoint(localRevLeftA, m_rotation).x,
                                   m_position.y + RotateLocalPoint(localRevLeftA, m_rotation).y };
        const Vector2 wRevLB = { m_position.x + RotateLocalPoint(localRevLeftB, m_rotation).x,
                                   m_position.y + RotateLocalPoint(localRevLeftB, m_rotation).y };

        const Vector2 wRevRTip = { m_position.x + RotateLocalPoint(localRevRightTip, m_rotation).x,
                                   m_position.y + RotateLocalPoint(localRevRightTip, m_rotation).y };
        const Vector2 wRevRA = { m_position.x + RotateLocalPoint(localRevRightA, m_rotation).x,
                                   m_position.y + RotateLocalPoint(localRevRightA, m_rotation).y };
        const Vector2 wRevRB = { m_position.x + RotateLocalPoint(localRevRightB, m_rotation).x,
                                   m_position.y + RotateLocalPoint(localRevRightB, m_rotation).y };

        DrawTriangle(wRevLTip, wRevLB, wRevLA, SKYBLUE);
        DrawTriangle(wRevRTip, wRevRB, wRevRA, SKYBLUE);
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
