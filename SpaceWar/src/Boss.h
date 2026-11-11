// Boss.h
// Purpose: Defines the Boss class representing the main boss enemy
// encountered at the end of a level or wave sequence.
//
// Attributes:
// - position: Current (x, y) position of the boss on screen.
// - health: Current boss health.
// - maxHealth: Maximum boss health.
// - speed: Movement speed in pixels per second.
// - texture: Sprite (Texture2D) used to render the boss.
// - currentPhase: Current combat phase (e.g., Phase 1, Phase 2) affecting attack patterns.
// - active: Whether the boss is currently active in the game world.
// - attackCooldown: Time remaining before the boss can perform its next attack.
//
// Methods:
// - Init(): Initializes boss starting position, health, and phase.
// - Update(float deltaTime): Updates boss movement, phase transitions, and attack timers.
// - Draw(): Renders the boss sprite.
// - Attack(): Executes the boss's current phase attack pattern.
// - TakeDamage(int damage): Reduces boss health and triggers phase changes if needed.
// - ChangePhase(): Advances or updates the boss's combat phase.
// - IsAlive(): Returns true if the boss still has health remaining.
// - DrawHealthBar(): Renders the boss's HUD health bar on screen.

#pragma once

#include "raylib.h"

class Boss
{
private:
    Vector2 position;
    int health;
    int maxHealth;
    float speed;
    Texture2D texture;
    int currentPhase;
    bool active;
    float attackCooldown;

public:
    // Constructor and Destructor
    Boss();
    ~Boss();

    // Lifecycle methods
    void Init();
    void Update(float deltaTime);
    void Draw() const;

    // Combat and phase methods
    void Attack();
    void TakeDamage(int damage);
    void ChangePhase();
    bool IsAlive() const;
    void DrawHealthBar() const;
};
