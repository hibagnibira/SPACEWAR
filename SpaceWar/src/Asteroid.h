// Asteroid.h
// Purpose: Defines the Asteroid class representing environmental obstacles
// that the player must avoid or destroy.
//
// Attributes:
// - position: Current (x, y) position of the asteroid.
// - velocity: Movement direction and speed vector.
// - size: Radius or scale factor of the asteroid.
// - rotation: Current rotation angle in degrees.
// - rotationSpeed: Speed of rotation in degrees per second.
// - health: Current durability of the asteroid.
// - texture: Sprite (Texture2D) used to render the asteroid.
// - active: Whether the asteroid is currently active in the game world.
//
// Methods:
// - Update(float deltaTime): Updates asteroid position and rotation.
// - Draw(): Renders the rotating asteroid sprite.
// - GetBounds(): Returns the collision rectangle for hit detection.
// - IsOffScreen(): Returns true if the asteroid has left the visible screen.

#pragma once

#include "raylib.h"

class Asteroid
{
private:
    Vector2 position;
    Vector2 velocity;
    float size;
    float rotation;
    float rotationSpeed;
    int health;
    Texture2D texture;
    bool active;

public:
    // Constructor and Destructor
    Asteroid();
    ~Asteroid();

    // Lifecycle methods
    void Update(float deltaTime);
    void Draw() const;

    // Utility and collision methods
    Rectangle GetBounds() const;
    bool IsOffScreen() const;
};
