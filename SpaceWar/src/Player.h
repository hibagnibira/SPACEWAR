// Player.h 
// Purpose: Defines the Player class representing the spaceship controlled by the user.
//
// Attributes
// 
// Methods
// 

// ============================================================================
// File: Player.h
// Description: Declaration of the Player spaceship class for SPACEWAR.
//              Handles rendering primitives, mouse-guided rotation,
//              Newtonian physics (acceleration, drag, momentum), and
//              world-boundary clamping.
// ============================================================================

#pragma once

#include "raylib.h"

// ----------------------------------------------------------------------------
// Default Gameplay & Physics Constants
// These values provide a responsive, drifting feel in zero gravity.
// ----------------------------------------------------------------------------
constexpr float DEFAULT_ACCELERATION = 450.0f; // Pixels per second squared
constexpr float DEFAULT_MAX_SPEED = 420.0f; // Maximum linear velocity (pixels/sec)
constexpr float DEFAULT_DRAG = 0.45f;  // Weak drag coefficient to slow down gradually
constexpr float SHIP_COLLISION_RADIUS = 18.0f;  // Approximate radius used for boundary checks

constexpr float STOP_DISTANCE = 6.0f;   // Distance to the cursor at which the ship comes to a full stop
constexpr float SLOWING_RADIUS = 180.0f; // Radius at which the ship begins decelerating to eliminate overshoot

class Player
{

private:
    // Transform & Physics State
    Vector2 m_position;  // Position in world space (X, Y)
    Vector2 m_velocity;  // Current velocity vector (pixels per second)
    float   m_rotation;  // Angle in radians facing the mouse cursor

    // Configurable Physics Parameters
    float m_acceleration; // Forward/Reverse acceleration magnitude
    float m_maxSpeed;     // Hard cap on translational speed
    float m_drag;         // Coefficient of natural velocity dissipation

    // Playable World Dimensions
    float m_worldWidth;
    float m_worldHeight;

    // Input & Thruster State (tracked for rendering thruster exhaust)
    bool m_isThrustingForward;
    //remove bool m_isThrustingReverse; because keyboard reverse movement (S key) is removed in favor of automatic mouse tracking, tracking reverse thrust is no longer needed.


public:
    // ------------------------------------------------------------------------
    // Constructor
    // Initializes the player's position, world limits, and movement physics.
    // ------------------------------------------------------------------------
    Player(Vector2 startPosition = { 400.0f, 300.0f },
        float worldWidth = 800.0f,
        float worldHeight = 600.0f);

    // ------------------------------------------------------------------------
    // Update
    // Call every frame. Performs:
    //  1. Mouse rotation calculations.
    //  2. Input gathering (W = forward, S = reverse).
    //  3. Velocity integration & acceleration.
    //  4. Natural linear drag / damping.
    //  5. Maximum speed clamp.
    //  6. Position integration and boundary enforcement.
    // ------------------------------------------------------------------------
    void Update(float dt);

    // ------------------------------------------------------------------------
    // Draw
    // Renders the spaceship using 2D geometric primitives.
    // Also renders thruster exhaust when accelerating.
    // ------------------------------------------------------------------------
    void Draw() const;

    // no use of setters and getters to keep highly security and respect the encapsulation fondumental
};