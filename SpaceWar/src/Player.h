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

// ----------------------------------------------------------------------------
// Dash / Boost Tuning
// ----------------------------------------------------------------------------
constexpr float DASH_DURATION = 1.0f;          // Dash lasts exactly 1 second
constexpr float DASH_SPEED_MULTIPLIER = 2.0f;  // Dash speed = 2x normal max speed (840 px/s)
constexpr float DASH_MIN_DIRECTION_DISTANCE = 1.0f; // Min mouse distance to compute dash direction

// PROVISIONAL: cooldown duration has not been finalized yet and will be
// tuned later. Kept as a single constant so it is easy to adjust.
constexpr float DASH_COOLDOWN_DURATION = 3.0f;

// Shooting interval between automatic shots while continuous fire is on.
// ~4 shots per second — medium frequency, tune after testing.
constexpr float FIRE_INTERVAL = 0.25f;

// ----------------------------------------------------------------------------
// Health Configuration
// ----------------------------------------------------------------------------
constexpr float PLAYER_MAX_HEALTH = 100.0f;      // Health is a percentage: 0-100
constexpr float PLAYER_STARTING_HEALTH = 100.0f; // Ships spawn at full health

// Invulnerability duration granted after each non-lethal hit (spec: 1 second)
constexpr float INVULNERABILITY_DURATION = 1.0f;

class Player
{
    // Grant Game direct access to private attributes for camera tracking
    // This allows Game to center the camera without exposing any getters or setters.
    friend class Game;

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

    // Dash / Boost State (Space key)
    bool    m_isDashing;          // True while a dash is currently active
    Vector2 m_dashDirection;      // Stored dash heading (locked at dash start)
    float   m_dashTimer;          // Remaining dash time in seconds
    float   m_dashCooldownTimer;  // Remaining cooldown time in seconds

    // Shooting State (left mouse button toggle)
    bool  m_isShooting;       // True while continuous fire is toggled on
    float m_fireTimer;        // Countdown to the next automatic shot
    bool  m_shotRequested;    // One-frame flag: Game spawns a bullet when set

    // Health & Death State
    float m_health;   // Current health, clamped between 0 and PLAYER_MAX_HEALTH
    bool  m_isDead;   // True once health reaches zero
    Vector2 m_spawnPosition; // Original spawn point, restored on respawn
    float m_invulnerabilityTimer; // Seconds of invulnerability remaining (>0 = protected)

    // Ends the active dash and starts the cooldown timer
    void EndDash();


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
    // Player reads the camera only to convert mouse screen coordinates to world coordinates
    void Update(float dt, const Camera2D& camera);

    // Applies damage from any source (enemy, asteroid, boss, debug).
    // Clamps health to [0, MAX] and marks the player dead at zero.
    void TakeDamage(float damageAmount);

    // Renders the HUD health bar in screen space (fixed position).
    // Reads m_health internally — no getters needed.
    void DrawHealthBar() const;

    // Resets the player to its spawn state: position, health, rotation
    // toward the mouse, and clears dash/shooting state. Only valid when dead.
    void Respawn(const Camera2D& camera);

    // ------------------------------------------------------------------------
    // Draw
    // Renders the spaceship using 2D geometric primitives.
    // Also renders thruster exhaust when accelerating.
    // ------------------------------------------------------------------------
    void Draw() const;

    // no use of setters and getters to keep highly security and respect the encapsulation fondumental
};