// Bullet.h
// Purpose: Defines the Bullet class representing projectiles fired by the
// player or by enemy spaceships.
//
// Attributes:
// - position: Current (x, y) position of the projectile.
// - velocity: Direction vector of the projectile's movement.
// - speed: Movement speed in pixels per second.
// - damage: Amount of damage dealt upon hitting a target.
// - texture: Sprite (Texture2D) used to render the projectile.
// - active: Whether the projectile is currently active in the game world.
//
// Methods:
// - Update(float deltaTime): Moves the projectile and checks if it left the screen.
// - Draw(): Renders the projectile sprite.
// - IsOffScreen(): Returns true if the projectile is outside the visible screen area.
// - GetBounds(): Returns the collision rectangle for hit detection.

#pragma once

#include "raylib.h"

class Bullet
{
private:
    Vector2 position;
    Vector2 velocity;
    float speed;
    int damage;
    Texture2D texture;
    bool active;

public:
    // Constructor and Destructor
    Bullet();
    ~Bullet();

    // Lifecycle methods
    void Update(float deltaTime);
    void Draw() const;

    // Utility and collision methods
    bool IsOffScreen() const;
    Rectangle GetBounds() const;
};
