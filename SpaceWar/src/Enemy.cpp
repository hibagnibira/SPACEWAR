// Enemy.cpp
// Purpose: Implements the methods declared in Enemy.h.
//
// This file will contain the logic for enemy movement patterns, AI,
// shooting behavior, health management, and sprite rendering.

#include "Enemy.h"

Enemy::Enemy()
    : position{ 0.0f, 0.0f },
      speed(0.0f),
      health(50),
      maxHealth(50),
      bounds{ 0.0f, 0.0f, 0.0f, 0.0f },
      texture{},
      active(false),
      shootCooldown(0.0f)
{
    // TODO: Set default enemy values.
}

Enemy::~Enemy()
{
    // TODO: Clean up any enemy-specific resources if needed.
}

void Enemy::Init()
{
    // TODO: Initialize enemy position, speed, health, and active state.
}

void Enemy::Update(float deltaTime)
{
    // TODO: Update enemy movement, shooting cooldown, and collision bounds.
    (void)deltaTime;
}

void Enemy::Draw() const
{
    // TODO: Render the enemy sprite if active.
}

void Enemy::Shoot()
{
    // TODO: Spawn an enemy projectile toward the player.
}

void Enemy::TakeDamage(int damage)
{
    // TODO: Subtract damage from health and deactivate if health <= 0.
    (void)damage;
}

bool Enemy::IsAlive() const
{
    // TODO: Return true if health > 0 and enemy is active.
    return health > 0;
}

Rectangle Enemy::GetBounds() const
{
    // TODO: Return the enemy's collision rectangle.
    return bounds;
}
