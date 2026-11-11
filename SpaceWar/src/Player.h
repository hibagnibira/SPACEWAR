// Player.h
// Purpose: Defines the Player class representing the spaceship controlled by the user.
//
// Attributes:
// - position: Current (x, y) position of the spaceship on screen.
// - speed: Movement speed of the spaceship in pixels per second.
// - health: Current player health.
// - maxHealth: Maximum possible health.
// - lives: Number of remaining lives.
// - bounds: Collision boundaries (Rectangle) used for hit detection.
// - texture: Sprite (Texture2D) used to render the spaceship.
// - shootCooldown: Time remaining before the player can fire again.
//
// Methods:
// - Init(): Initializes player position, health, and starting values.
// - Update(float deltaTime): Updates player state each frame.
// - Draw(): Renders the player spaceship on screen.
// - HandleInput(): Reads keyboard/controller input for movement and shooting.
// - Move(float deltaTime): Applies movement based on speed and deltaTime.
// - Shoot(): Creates and fires a projectile.
// - TakeDamage(int damage): Reduces player health when hit.
// - IsAlive(): Returns true if the player still has health/lives remaining.
// - GetHealth(): Returns the player's current health.

#pragma once

#include "raylib.h"

class Player
{
private:
    Vector2 position;
    float speed;
    int health;
    int maxHealth;
    int lives;
    Rectangle bounds;
    Texture2D texture;
    float shootCooldown;

public:
    // Constructor and Destructor
    Player();
    ~Player();

    // Lifecycle methods
    void Init();
    void Update(float deltaTime);
    void Draw() const;

    // Gameplay action methods
    void HandleInput();
    void Move(float deltaTime);
    void Shoot();
    void TakeDamage(int damage);

    // State query methods
    bool IsAlive() const;
    int GetHealth() const;
};
