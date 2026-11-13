// Enemy.cpp
// Purpose: Implements the methods declared in Enemy.h.

#include "Enemy.h"
#include <algorithm>
#include <cmath>

Enemy::Enemy(Vector2 spawnPos, float size)
    : position(spawnPos)
    , radius(size)
    , m_destination(spawnPos)   // first Update() immediately picks a real one
    , m_isChasing(false)
{
}

// ----------------------------------------------------------------------------
// PickNewDestination
// Random point inside the given bounds (world area or camera view).
// ----------------------------------------------------------------------------
void Enemy::PickNewDestination(float minX, float minY, float maxX, float maxY)
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
// Wander until the enemy enters the camera's visible world area, then
// chase the player permanently. All movement uses delta time.
// ----------------------------------------------------------------------------
void Enemy::Update(float dt, Vector2 playerPos, const Camera2D& camera,
                   float worldWidth, float worldHeight,
                   bool playerAccelerating, bool playerDead)
{
    if (dt <= 0.0f) return;

    // --- Player detection: latches permanently on first entry into view ---
    // While the player is dead, chasing is suspended entirely: the latch
    // resets so enemies wander and must re-detect after respawn.
    if (playerDead)
    {
        m_isChasing = false;
    }
    else if (!m_isChasing)
    {
        // camera.offset = screen size / 2, so offset/zoom = visible half extents
        const float halfViewW = camera.offset.x / camera.zoom;
        const float halfViewH = camera.offset.y / camera.zoom;

        const bool inView =
            position.x >= camera.target.x - halfViewW &&
            position.x <= camera.target.x + halfViewW &&
            position.y >= camera.target.y - halfViewH &&
            position.y <= camera.target.y + halfViewH;

        if (inView)
        {
            m_isChasing = true;
        }
    }

    // --- Choose this frame's movement target ---
    Vector2 target;
    if (m_isChasing)
    {
        target = playerPos; // continuously track the player's live position
    }
    else
    {
        // Destination bounds: whole world normally; camera-visible area
        // (clamped to the world) while the player is dead.
        float minX = ENEMY_BOUNDARY_MARGIN + radius;
        float minY = ENEMY_BOUNDARY_MARGIN + radius;
        float maxX = worldWidth - ENEMY_BOUNDARY_MARGIN - radius;
        float maxY = worldHeight - ENEMY_BOUNDARY_MARGIN - radius;

        if (playerDead)
        {
            const float halfViewW = camera.offset.x / camera.zoom;
            const float halfViewH = camera.offset.y / camera.zoom;

            minX = std::max(minX, camera.target.x - halfViewW);
            maxX = std::min(maxX, camera.target.x + halfViewW);
            minY = std::max(minY, camera.target.y - halfViewH);
            maxY = std::min(maxY, camera.target.y + halfViewH);

            // Degenerate case (view fully outside world): fall back to world
            if (maxX <= minX) { minX = ENEMY_BOUNDARY_MARGIN + radius; maxX = worldWidth - ENEMY_BOUNDARY_MARGIN - radius; }
            if (maxY <= minY) { minY = ENEMY_BOUNDARY_MARGIN + radius; maxY = worldHeight - ENEMY_BOUNDARY_MARGIN - radius; }
        }

        const float toDestX = m_destination.x - position.x;
        const float toDestY = m_destination.y - position.y;
        const bool arrived = std::hypot(toDestX, toDestY) <= ENEMY_ARRIVE_DISTANCE;
        const bool destOutside = m_destination.x < minX || m_destination.x > maxX ||
                                 m_destination.y < minY || m_destination.y > maxY;

        // Repick on arrival, or when dead-wandering leaves the view area
        if (arrived || destOutside)
        {
            PickNewDestination(minX, minY, maxX, maxY);
        }
        target = m_destination;
    }

    // --- Direction toward target (normalized) ---
    float dirX = target.x - position.x;
    float dirY = target.y - position.y;
    const float len = std::hypot(dirX, dirY);
    if (len > 0.0001f)
    {
        dirX /= len;
        dirY /= len;
    }

    // --- Smooth boundary steering: bias direction inward near edges ---
    if (position.x < ENEMY_BOUNDARY_MARGIN)
    {
        dirX += ENEMY_BOUNDARY_STEER * (ENEMY_BOUNDARY_MARGIN - position.x) / ENEMY_BOUNDARY_MARGIN;
    }
    else if (position.x > worldWidth - ENEMY_BOUNDARY_MARGIN)
    {
        dirX -= ENEMY_BOUNDARY_STEER * (position.x - (worldWidth - ENEMY_BOUNDARY_MARGIN)) / ENEMY_BOUNDARY_MARGIN;
    }
    if (position.y < ENEMY_BOUNDARY_MARGIN)
    {
        dirY += ENEMY_BOUNDARY_STEER * (ENEMY_BOUNDARY_MARGIN - position.y) / ENEMY_BOUNDARY_MARGIN;
    }
    else if (position.y > worldHeight - ENEMY_BOUNDARY_MARGIN)
    {
        dirY -= ENEMY_BOUNDARY_STEER * (position.y - (worldHeight - ENEMY_BOUNDARY_MARGIN)) / ENEMY_BOUNDARY_MARGIN;
    }

    // Re-normalize after adding the steering bias
    const float steerLen = std::hypot(dirX, dirY);
    if (steerLen > 0.0001f)
    {
        dirX /= steerLen;
        dirY /= steerLen;
    }

    // --- Move (frame-rate independent) ---
    // Enemies speed up only while the player is alive AND accelerating
    const float currentSpeed = (playerAccelerating && !playerDead)
        ? ENEMY_SPEED_PLAYER_ACCEL : ENEMY_SPEED_NORMAL;
    position.x += dirX * currentSpeed * dt;
    position.y += dirY * currentSpeed * dt;

    // --- Hard safety clamp inside world bounds (radius margin) ---
    position.x = std::max(radius, std::min(position.x, worldWidth - radius));
    position.y = std::max(radius, std::min(position.y, worldHeight - radius));
}

void Enemy::Draw() const
{
    // Placeholder: red circle, clearly distinct from the cyan player ship
    DrawCircleV(position, radius, Color{ 220, 60, 60, 255 });
    DrawCircleLinesV(position, radius, Color{ 120, 20, 20, 255 });
}
