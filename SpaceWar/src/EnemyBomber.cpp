// EnemyBomber.cpp
// Purpose: Implements the methods declared in EnemyBomber.h.
//
// Phase 1 scope: slow world-wide wandering with a large keep-away radius
// around the player. No chasing, no camera dependence, no contact attempts,
// no bomb attacks yet.

#include "EnemyBomber.h"
#include <algorithm>
#include <cmath>

EnemyBomber::EnemyBomber(Vector2 spawnPos, float size)
    : position(spawnPos)
    , radius(size)
    , m_destination(spawnPos) // first Update() immediately picks a real one
    , m_health(ENEMY3_MAX_HEALTH)
    , m_attackTimer(ENEMY3_ATTACK_INTERVAL)
    , m_bombRequested(false)
{
}

// ----------------------------------------------------------------------------
// PickNewDestination
// Random point anywhere inside the world bounds (margin-inset).
// ----------------------------------------------------------------------------
void EnemyBomber::PickNewDestination(float minX, float minY, float maxX, float maxY)
{
    const float rx = static_cast<float>(GetRandomValue(0, 1000000)) / 1000000.0f;
    const float ry = static_cast<float>(GetRandomValue(0, 1000000)) / 1000000.0f;

    m_destination = {
        minX + rx * (maxX - minX),
        minY + ry * (maxY - minY)
    };
}

// ----------------------------------------------------------------------------
// Update
// Wander the whole world at a slow pace. If the player comes inside
// ENEMY3_KEEP_MIN_DIST, add a gentle repulsion bias — the Bomber drifts
// away gradually instead of chasing or fleeing. World-boundary steering
// and a hard clamp keep it inside the arena at all times.
// ----------------------------------------------------------------------------
void EnemyBomber::Update(float dt, Vector2 playerPos, float worldWidth, float worldHeight,
                         bool playerDead)
{
    if (dt <= 0.0f || IsDestroyed()) return;

    const float minX = ENEMY3_BOUNDARY_MARGIN + radius;
    const float minY = ENEMY3_BOUNDARY_MARGIN + radius;
    const float maxX = worldWidth - ENEMY3_BOUNDARY_MARGIN - radius;
    const float maxY = worldHeight - ENEMY3_BOUNDARY_MARGIN - radius;

    // Repick destination on arrival or if it somehow left the bounds
    const float toDestX = m_destination.x - position.x;
    const float toDestY = m_destination.y - position.y;
    const bool arrived = std::hypot(toDestX, toDestY) <= ENEMY3_ARRIVE_DISTANCE;
    const bool destOutside = m_destination.x < minX || m_destination.x > maxX ||
                             m_destination.y < minY || m_destination.y > maxY;
    if (arrived || destOutside)
    {
        PickNewDestination(minX, minY, maxX, maxY);
    }

    // --- Direction toward destination (normalized) ---
    float dirX = m_destination.x - position.x;
    float dirY = m_destination.y - position.y;
    const float len = std::hypot(dirX, dirY);
    if (len > 0.0001f)
    {
        dirX /= len;
        dirY /= len;
    }

    // --- Keep-away: gentle repulsion while the player is inside the
    // standoff radius. Bias grows as the player gets closer, so the
    // Bomber maintains a large distance without aggressive fleeing. ---
    const float pdx = position.x - playerPos.x;
    const float pdy = position.y - playerPos.y;
    const float pdist = std::hypot(pdx, pdy);
    if (pdist > 0.001f && pdist < ENEMY3_KEEP_MIN_DIST)
    {
        const float closeness = 1.0f - (pdist / ENEMY3_KEEP_MIN_DIST);
        dirX += (pdx / pdist) * ENEMY3_KEEPAWAY_STRENGTH * closeness;
        dirY += (pdy / pdist) * ENEMY3_KEEPAWAY_STRENGTH * closeness;
    }

    // --- Bomb attack (Phase 4): one bomb at a time on a cooldown, only
    // while the player is alive and inside attack range. Game reads
    // m_bombRequested (friendship) and spawns the actual Bomb. ---
    if (!playerDead && pdist < ENEMY3_ATTACK_RANGE)
    {
        m_attackTimer -= dt;
        if (m_attackTimer <= 0.0f)
        {
            m_bombRequested = true;
            m_attackTimer = ENEMY3_ATTACK_INTERVAL;
        }
    }

    // --- Smooth boundary steering: bias direction inward near edges ---
    if (position.x < ENEMY3_BOUNDARY_MARGIN)
    {
        dirX += ENEMY3_BOUNDARY_STEER * (ENEMY3_BOUNDARY_MARGIN - position.x) / ENEMY3_BOUNDARY_MARGIN;
    }
    else if (position.x > worldWidth - ENEMY3_BOUNDARY_MARGIN)
    {
        dirX -= ENEMY3_BOUNDARY_STEER * (position.x - (worldWidth - ENEMY3_BOUNDARY_MARGIN)) / ENEMY3_BOUNDARY_MARGIN;
    }
    if (position.y < ENEMY3_BOUNDARY_MARGIN)
    {
        dirY += ENEMY3_BOUNDARY_STEER * (ENEMY3_BOUNDARY_MARGIN - position.y) / ENEMY3_BOUNDARY_MARGIN;
    }
    else if (position.y > worldHeight - ENEMY3_BOUNDARY_MARGIN)
    {
        dirY -= ENEMY3_BOUNDARY_STEER * (position.y - (worldHeight - ENEMY3_BOUNDARY_MARGIN)) / ENEMY3_BOUNDARY_MARGIN;
    }

    // Re-normalize after adding steering biases
    const float steerLen = std::hypot(dirX, dirY);
    if (steerLen > 0.0001f)
    {
        dirX /= steerLen;
        dirY /= steerLen;
    }

    // --- Move (frame-rate independent) ---
    position.x += dirX * ENEMY3_SPEED * dt;
    position.y += dirY * ENEMY3_SPEED * dt;

    // --- Hard safety clamp inside world bounds (radius margin) ---
    position.x = std::max(radius, std::min(position.x, worldWidth - radius));
    position.y = std::max(radius, std::min(position.y, worldHeight - radius));
}

void EnemyBomber::Draw() const
{
    // Placeholder: purple triangle — distinct from red/orange Type 1/2 circles
    const Vector2 p1 = { position.x, position.y - radius * 1.3f };
    const Vector2 p2 = { position.x - radius, position.y + radius };
    const Vector2 p3 = { position.x + radius, position.y + radius };
    DrawTriangle(p1, p2, p3, Color{ 170, 80, 220, 255 });
    DrawTriangleLines(p1, p2, p3, Color{ 90, 30, 130, 255 });
}

bool EnemyBomber::Touches(Vector2 otherPos, float otherRadius) const
{
    const float dx = position.x - otherPos.x;
    const float dy = position.y - otherPos.y;
    const float reach = radius + otherRadius;
    return (dx * dx + dy * dy) <= (reach * reach);
}

void EnemyBomber::TakeDamage(float damageAmount)
{
    if (damageAmount <= 0.0f || IsDestroyed())
    {
        return;
    }
    m_health -= damageAmount;
    if (m_health < 0.0f)
    {
        m_health = 0.0f; // never negative — 0 HP means destroyed
    }
}

bool EnemyBomber::IsDestroyed() const
{
    return m_health <= 0.0f;
}

float EnemyBomber::DistanceTo(Vector2 worldPos) const
{
    return std::hypot(position.x - worldPos.x, position.y - worldPos.y);
}

void EnemyBomber::ResetTo(Vector2 spawnPos)
{
    // Reposition for spawning: new position and wander destination.
    // Health is preserved (unlike a full respawn, which is a later phase).
    position = spawnPos;
    m_destination = spawnPos;
}

void EnemyBomber::RespawnAt(Vector2 spawnPos)
{
    // Full respawn after destruction: new world position, restored health,
    // fresh wander destination. There is never more than one Bomber —
    // Game reuses this same instance.
    position = spawnPos;
    m_destination = spawnPos;
    m_health = ENEMY3_MAX_HEALTH;
    m_attackTimer = ENEMY3_ATTACK_INTERVAL; // fresh bomber: first bomb after interval
    m_bombRequested = false;
}
