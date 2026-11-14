// Bomb.cpp
// Purpose: Implements the methods declared in Bomb.h.
//
// The bomb locks its direction at construction and travels straight toward
// the target. It never homes, never damages, and outlives its launcher.

#include "Bomb.h"
#include <algorithm>
#include <cmath>

Bomb::Bomb(Vector2 spawnPos, Vector2 targetPos, float speed)
    : m_position(spawnPos)
    , m_target(targetPos)
    , m_speed(speed)
    , m_lifetimeRemaining(BOMB_LIFETIME)
    , m_warningTimer(0.0f)
    , m_inWarning(false)
    , m_exploding(false)
    , m_explosionTimer(0.0f)
    , m_detonated(false)
    , m_finished(false)
{
    // Lock the trajectory once: unit vector from spawn toward target.
    const float dx = m_target.x - spawnPos.x;
    const float dy = m_target.y - spawnPos.y;
    const float len = std::hypot(dx, dy);
    if (len > 0.0001f)
    {
        m_direction = { dx / len, dy / len };
    }
    else
    {
        m_direction = { 0.0f, 0.0f }; // degenerate: already at target
        m_finished = true;
    }
}

void Bomb::Update(float dt)
{
    if (dt <= 0.0f || m_finished) return;

    // --- Warning phase: bomb has arrived; hold position and count down.
    // The warning circle stays fixed on m_target — it never follows
    // the player. When the timer ends the bomb becomes "finished",
    // which is the hook the explosion phase (Phase 6) will use. ---
    if (m_inWarning)
    {
        m_warningTimer -= dt;
        if (m_warningTimer <= 0.0f)
        {
            // Warning over: detonate at the locked target. The detonation
            // flag lets Game apply blast damage exactly once this frame.
            m_inWarning = false;
            m_exploding = true;
            m_explosionTimer = BOMB_EXPLOSION_DURATION;
            m_detonated = true;
        }
        return;
    }

    // --- Explosion phase: animation only; position stays fixed on the
    // target. Damage is Game's job via ConsumeDetonation/IsInsideBlast. ---
    if (m_exploding)
    {
        m_explosionTimer -= dt;
        if (m_explosionTimer <= 0.0f)
        {
            m_exploding = false;
            m_finished = true;
        }
        return;
    }

    m_lifetimeRemaining -= dt;
    if (m_lifetimeRemaining <= 0.0f)
    {
        m_finished = true;
        return;
    }

    // Distance still to travel; if this step reaches/passes the target,
    // land exactly on it and enter the warning phase — no overshoot.
    const float remaining = std::hypot(m_target.x - m_position.x,
                                       m_target.y - m_position.y);
    const float step = m_speed * dt;
    if (step >= remaining)
    {
        m_position = m_target;
        m_inWarning = true;
        m_warningTimer = BOMB_WARNING_DURATION;
        return;
    }

    m_position.x += m_direction.x * step;
    m_position.y += m_direction.y * step;
}

void Bomb::Draw() const
{
    // Explosion: expanding, fading fireball at the fixed target — orange/
    // yellow so it reads differently from the red warning circle.
    if (m_exploding)
    {
        const float progress = std::min(1.0f,
            1.0f - (m_explosionTimer / BOMB_EXPLOSION_DURATION));
        const float radius = BOMB_EXPLOSION_RADIUS * (0.25f + 0.75f * progress);
        const float fade = 1.0f - progress;

        DrawCircleV(m_target, radius, Fade(ORANGE, 0.55f * fade));
        DrawCircleV(m_target, radius * 0.55f, Fade(YELLOW, 0.70f * fade));
        DrawCircleLinesV(m_target, radius, Fade(WHITE, 0.80f * fade));
        return;
    }

    // Warning phase: red circle fixed on the ORIGINAL target position —
    // radius matches the future explosion area. Gentle alpha pulse via
    // GetTime() keeps it readable without any particle system.
    if (m_inWarning)
    {
        const float pulse = 0.5f + 0.5f * std::sin(static_cast<float>(GetTime()) * 6.0f);
        DrawCircleV(m_target, BOMB_EXPLOSION_RADIUS,
                    Fade(RED, 0.10f + 0.10f * pulse));
        DrawCircleLinesV(m_target, BOMB_EXPLOSION_RADIUS,
                         Fade(RED, 0.65f + 0.25f * pulse));
        // Small center marker so the bomb itself stays visible in the zone
        DrawCircleV(m_target, BOMB_RADIUS * 0.6f, Color{ 255, 120, 120, 255 });
        return;
    }

    // Flying: small glowing energy orb — soft outer glow + dark body +
    // bright core. Clearly different silhouette from the warning circle.
    DrawCircleV(m_position, BOMB_RADIUS * 2.4f, Color{ 200, 40, 90, 45 });
    DrawCircleV(m_position, BOMB_RADIUS, Color{ 190, 35, 80, 255 });
    DrawCircleV(m_position, BOMB_RADIUS * 0.45f, Color{ 255, 200, 220, 230 });
}

bool Bomb::IsFinished() const
{
    return m_finished;
}

bool Bomb::ConsumeDetonation()
{
    const bool detonated = m_detonated;
    m_detonated = false;
    return detonated;
}

bool Bomb::IsInsideBlast(Vector2 worldPos) const
{
    const float dx = worldPos.x - m_target.x;
    const float dy = worldPos.y - m_target.y;
    return std::hypot(dx, dy) <= BOMB_EXPLOSION_RADIUS;
}
