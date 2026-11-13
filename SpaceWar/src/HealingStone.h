// HealingStone.h
// Purpose: Defines the HealingStone class — a pickup that restores 1% player
// health on contact and disappears after a short lifetime.
//
// Attributes:
// - position: World-space (x, y) of the stone.
// - lifetimeRemaining: Seconds before the stone expires.
//
// Methods:
// - Update(float dt): Ages the stone.
// - Draw(): Renders a small placeholder shape.
// - IsExpired(): True when lifetime reaches zero.
// - Overlaps(Vector2, float): Circle-vs-circle collection test.

#pragma once

#include "raylib.h"

// ----------------------------------------------------------------------------
// Healing Stone Tuning
// ----------------------------------------------------------------------------
constexpr float HEALING_AMOUNT = 1.0f;          // Health restored per stone
constexpr float HEALING_STONE_LIFETIME = 3.0f;  // Seconds before expiry
constexpr float HEALING_STONE_RADIUS = 8.0f;    // Collision/draw radius

class HealingStone
{
private:
    Vector2 position;
    float   lifetimeRemaining;

public:
    HealingStone(Vector2 spawnPos);

    void Update(float deltaTime);
    void Draw() const;

    bool IsExpired() const;
    bool Overlaps(Vector2 otherPos, float otherRadius) const;
};
