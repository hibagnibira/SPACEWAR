// Player.cpp
// Purpose: Implements the methods declared in Player.h.
//
// This file will contain the logic for player movement, health management,
// shooting projectiles, collision bounds updates, and sprite rendering.

#include "Player.h"

Player::Player()
    : position{ 0.0f, 0.0f },
      speed(0.0f),
      health(100),
      maxHealth(100),
      lives(3),
      bounds{ 0.0f, 0.0f, 0.0f, 0.0f },
      texture{},
      shootCooldown(0.0f)
{
    // TODO: Set default starting values or defer initialization to Init().
}

Player::~Player()
{
    // TODO: Clean up any player-specific resources if needed.
}

void Player::Init()
{
    // TODO: Initialize player starting position, speed, health, lives, and collision bounds.
}

void Player::Update(float deltaTime)
{
    // TODO: Handle input, update movement, decrement shootCooldown, and update bounds.
    (void)deltaTime;
}

void Player::Draw() const
{
    // TODO: Render the player spaceship texture at its current position.
}

void Player::HandleInput()
{
    // TODO: Read player movement keys (WASD / Arrow keys) and shoot key (Space).
}

void Player::Move(float deltaTime)
{
    // TODO: Update position based on input direction, speed, and deltaTime; clamp to screen bounds.
    (void)deltaTime;
}

void Player::Shoot()
{
    // TODO: Spawn a new player Bullet when shootCooldown <= 0.
}

void Player::TakeDamage(int damage)
{
    // TODO: Subtract damage from health, handle life loss, and trigger invulnerability frames.
    (void)damage;
}

bool Player::IsAlive() const
{
    // TODO: Return true if health > 0 or lives > 0.
    return health > 0;
}

int Player::GetHealth() const
{
    // TODO: Return current player health.
    return health;
}
