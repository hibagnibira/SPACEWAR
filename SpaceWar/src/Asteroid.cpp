// Asteroid.cpp
// Purpose: Implements the methods declared in Asteroid.h.
//
// This file will contain the logic for asteroid movement, continuous rotation,
// off-screen cleanup, collision bounds, and sprite rendering.

#include "Asteroid.h"

Asteroid::Asteroid()
    : position{ 0.0f, 0.0f },
      velocity{ 0.0f, 0.0f },
      size(1.0f),
      rotation(0.0f),
      rotationSpeed(0.0f),
      health(30),
      texture{},
      active(false)
{
    // TODO: Initialize default asteroid properties.
}

Asteroid::~Asteroid()
{
    // TODO: Clean up any asteroid-specific resources if needed.
}

void Asteroid::Update(float deltaTime)
{
    // TODO: Update position by velocity * deltaTime and increment rotation by rotationSpeed * deltaTime.
    (void)deltaTime;
}

void Asteroid::Draw() const
{
    // TODO: Render the asteroid texture with its current rotation if active.
}

Rectangle Asteroid::GetBounds() const
{
    // TODO: Return a Rectangle representing the asteroid's collision area.
    return Rectangle{ position.x, position.y, 0.0f, 0.0f };
}

bool Asteroid::IsOffScreen() const
{
    // TODO: Check if the asteroid has moved completely off the left/bottom/top of the screen.
    return false;
}
