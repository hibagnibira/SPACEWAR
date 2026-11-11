// Enemy.h
// Purpose: Defines the Enemy class representing standard enemy spaceships.
//
// Attributes:
// - position: Current (x, y) position of the enemy ship.
// - speed: Movement speed in pixels per second.
// - health: Current enemy health.
// - maxHealth: Maximum enemy health.
// - bounds: Collision boundaries (Rectangle) for hit detection.
// - texture: Sprite (Texture2D) used to render the enemy ship.
// - active: Whether the enemy is currently active in the game world.
// - shootCooldown: Time remaining before the enemy can fire again.
//
// Methods:
// - Init(): Initializes enemy starting position, health, and properties.
// - Update(float deltaTime): Updates enemy movement, AI, and shooting timers.
// - Draw(): Renders the enemy spaceship.
// - Shoot(): Fires an enemy projectile.
// - TakeDamage(int damage): Reduces enemy health when hit by player bullets.
// - IsAlive(): Returns true if the enemy still has health remaining.
// - GetBounds(): Returns the collision rectangle for hit detection.

#pragma once

#include "raylib.h"

class Enemy
{
private:
    Vector2 position;
    float speed;
    int health;
    int maxHealth;
    Rectangle bounds;
    Texture2D texture;
    bool active;
    float shootCooldown;

public:
    // Constructor and Destructor
    Enemy();
    ~Enemy();

    // Lifecycle methods
    void Init();
    void Update(float deltaTime);
    void Draw() const;

    // Gameplay action methods
    void Shoot();
    void TakeDamage(int damage);

    // State and collision methods
    bool IsAlive() const;
    Rectangle GetBounds() const;
};
