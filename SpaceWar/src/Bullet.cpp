// Bullet.cpp
// Purpose: Implements the methods declared in Bullet.h.
//
// This file will contain the logic for projectile movement, off-screen
// deactivation, collision bounds calculation, and sprite rendering.

#include "Bullet.h"

Bullet::Bullet()
    : position{ 0.0f, 0.0f },
      velocity{ 0.0f, 0.0f },
      speed(0.0f),
      damage(10),
      texture{},
      active(false)
{
    // TODO: Initialize default projectile properties.
}

Bullet::~Bullet()
{
    // TODO: Clean up any bullet-specific resources if needed.
}

void Bullet::Update(float deltaTime)
{
    // TODO: Move bullet using velocity * speed * deltaTime and deactivate if off-screen.
    (void)deltaTime;
}

void Bullet::Draw() const
{
    // TODO: Render the bullet texture at its current position if active.
}

bool Bullet::IsOffScreen() const
{
    // TODO: Check if position is outside the game window boundaries.
    return false;
}

Rectangle Bullet::GetBounds() const
{
    // TODO: Return a Rectangle matching the bullet's position and texture size.
    return Rectangle{ position.x, position.y, 0.0f, 0.0f };
}
