// HealingStone.cpp
// Purpose: Implements the methods declared in HealingStone.h.

#include "HealingStone.h"
#include <cmath>

HealingStone::HealingStone(Vector2 spawnPos)
    : position(spawnPos)
    , lifetimeRemaining(HEALING_STONE_LIFETIME)
{
}

void HealingStone::Update(float deltaTime)
{
    lifetimeRemaining -= deltaTime;
}

void HealingStone::Draw() const
{
    // Placeholder: small cyan diamond
    DrawCircleV(position, HEALING_STONE_RADIUS, Color{ 60, 230, 230, 255 });
    DrawCircleLinesV(position, HEALING_STONE_RADIUS, Color{ 220, 255, 255, 255 });
}

bool HealingStone::IsExpired() const
{
    return lifetimeRemaining <= 0.0f;
}

bool HealingStone::Overlaps(Vector2 otherPos, float otherRadius) const
{
    const float dx = position.x - otherPos.x;
    const float dy = position.y - otherPos.y;
    const float reach = HEALING_STONE_RADIUS + otherRadius;
    return (dx * dx + dy * dy) <= (reach * reach);
}
