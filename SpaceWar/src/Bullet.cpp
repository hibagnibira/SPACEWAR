// Bullet.cpp
// Purpose: Implements the methods declared in Bullet.h.
//
// This file contains projectile movement, lifetime/distance expiry,
// placeholder rendering, and collision bounds for future enemy integration.

#include "Bullet.h"
#include <cmath>

Bullet::Bullet(Vector2 spawnPos, Vector2 bulletVelocity)
    : position(spawnPos)
    , velocity(bulletVelocity)
    , spawnPosition(spawnPos)
    , lifetimeRemaining(BULLET_LIFETIME)
    , distanceTraveled(0.0f)
{
}

void Bullet::Update(float deltaTime)
{
    position.x += velocity.x * deltaTime;
    position.y += velocity.y * deltaTime;

    lifetimeRemaining -= deltaTime;
    distanceTraveled += std::hypot(velocity.x, velocity.y) * deltaTime;
}

void Bullet::Draw() const
{
    DrawCircleV(position, BULLET_RADIUS, Color{ 255, 220, 80, 255 }); // placeholder: yellow orb
}

bool Bullet::IsExpired() const
{
    // Whichever limit is hit first removes the projectile
    return lifetimeRemaining <= 0.0f || distanceTraveled >= BULLET_MAX_DISTANCE;
}

Rectangle Bullet::GetBounds() const
{
    // Square bounds centered on the projectile, ready for future collision
    return Rectangle{ position.x - BULLET_RADIUS,
                      position.y - BULLET_RADIUS,
                      BULLET_RADIUS * 2.0f,
                      BULLET_RADIUS * 2.0f };
}
