// EnemyProjectile.cpp
// Purpose: Implements projectile motion, fading trail, expiry, and hit test.

#include "EnemyProjectile.h"
#include "Player.h"
#include <algorithm>
#include <cmath>

EnemyProjectile::EnemyProjectile(Vector2 spawnPos, Vector2 projVelocity, Color color,
                                 float damage, float trailLifeScale)
    : position(spawnPos)
    , velocity(projVelocity)
    , spawnPosition(spawnPos)
    , lifetimeRemaining(ENEMY_PROJ_LIFETIME)
    , distanceTraveled(0.0f)
    , distanceSinceTrail(0.0f)
    , m_trail{}
    , m_trailHead(0)
    , m_trailCount(0)
    , m_color(color)
    , m_trailLife(ENEMY_PROJ_TRAIL_LIFE * trailLifeScale)
    , m_damage(damage)
{
}

void EnemyProjectile::Update(float dt, float worldWidth, float worldHeight)
{
    if (dt <= 0.0f) return;

    position.x += velocity.x * dt;
    position.y += velocity.y * dt;

    lifetimeRemaining -= dt;

    const float speed = std::hypot(velocity.x, velocity.y);
    const float step = speed * dt;
    distanceTraveled += step;
    distanceSinceTrail += step;

    // Emit a trail point every TRAIL_SPACING pixels of travel — the trail
    // therefore follows the actual trajectory at any projectile speed.
    if (distanceSinceTrail >= ENEMY_PROJ_TRAIL_SPACING)
    {
        distanceSinceTrail -= ENEMY_PROJ_TRAIL_SPACING;
        m_trail[m_trailHead] = { position, m_trailLife };
        m_trailHead = (m_trailHead + 1) % ENEMY_PROJ_TRAIL_POINTS;
        if (m_trailCount < ENEMY_PROJ_TRAIL_POINTS)
        {
            ++m_trailCount;
        }
    }

    // Age all live trail points (ring order doesn't matter — all fade evenly)
    for (int i = 0; i < ENEMY_PROJ_TRAIL_POINTS; ++i)
    {
        if (m_trail[i].life > 0.0f)
        {
            m_trail[i].life -= dt;
        }
    }

    // Shots that leave the world are done — no point letting them fly forever
    if (position.x < 0.0f || position.x > worldWidth ||
        position.y < 0.0f || position.y > worldHeight)
    {
        lifetimeRemaining = 0.0f;
    }
}

void EnemyProjectile::Draw() const
{
    // Fading trail: oldest-to-newest order is irrelevant; alpha & radius
    // scale with remaining life so the tail dissolves smoothly.
    for (int i = 0; i < ENEMY_PROJ_TRAIL_POINTS; ++i)
    {
        const TrailPoint& p = m_trail[i];
        if (p.life <= 0.0f) continue;

        const float frac = p.life / m_trailLife;
        const float radius = ENEMY_PROJ_RADIUS * (0.4f + 0.6f * frac);
        const unsigned char alpha = static_cast<unsigned char>(140.0f * frac);
        DrawCircleV(p.pos, radius, Fade(m_color, alpha));
    }

    // Projectile head: small plasma energy orb — soft outer glow, colored
    // body, and a bright near-white core so it stays distinct from trail.
    DrawCircleV(position, ENEMY_PROJ_RADIUS * 2.4f, Fade(m_color, 45));
    DrawCircleV(position, ENEMY_PROJ_RADIUS, m_color);
    DrawCircleV(position, ENEMY_PROJ_RADIUS * 0.45f, Fade(WHITE, 220));
}

bool EnemyProjectile::IsExpired() const
{
    return lifetimeRemaining <= 0.0f || distanceTraveled >= ENEMY_PROJ_MAX_DISTANCE;
}

void EnemyProjectile::Destroy()
{
    lifetimeRemaining = 0.0f; // removed by Game's expiry sweep
}

void EnemyProjectile::ApplyToPlayer(Player& player) const
{
    player.TakeDamage(m_damage);
}

bool EnemyProjectile::Touches(Vector2 otherPos, float otherRadius) const
{
    const float dx = position.x - otherPos.x;
    const float dy = position.y - otherPos.y;
    const float reach = ENEMY_PROJ_RADIUS + otherRadius;
    return (dx * dx + dy * dy) <= (reach * reach);
}
