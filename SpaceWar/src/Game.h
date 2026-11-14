// Game.h
// Purpose: Defines the Game class, which is the central coordinator for SPACEWAR.
// It owns the main game state, the player, all active projectiles, enemies,
// asteroids, the boss, and the AssetManager.
//
// GameState Enum:
// - Menu: Title screen / main menu.
// - Playing: Active gameplay.
// - Paused: Game paused by the player.
// - GameOver: Player defeated.
// - Victory: Boss defeated.
//
// Attributes:
// - screenWidth: Width of the game window in pixels.
// - screenHeight: Height of the game window in pixels.
// - currentState: Current state of the game (Menu, Playing, Paused, GameOver, Victory).
// - player: The player-controlled spaceship.
// - bullets: Collection of active player and enemy bullets.
// - enemies: Collection of active standard enemy spaceships.
// - asteroids: Collection of active asteroid obstacles.
// - boss: The main boss enemy.
// - assetManager: Centralized manager for textures, fonts, and audio.
//
// Methods:
// - Init(): Initializes the window, assets, and starting game objects.
// - Run(): Executes the main game loop.
// - Update(float deltaTime): Updates all active game objects for the current frame.
// - Draw(): Renders the current game state and all active objects.
// - HandleInput(): Processes global input (e.g., pause, menu navigation).
// - CheckCollisions(): Checks and resolves collisions between entities.
// - SpawnEnemies(): Spawns new enemy waves and asteroids.
// - ChangeGameState(GameState newState): Transitions between game states.

#pragma once

#include <vector>
#include "Player.h"
#include "Bullet.h"
#include "Enemy.h"
#include "Asteroid.h"
#include "Boss.h"
#include "AssetManager.h"
#include "Bullet.h"
#include "HealingStone.h"
#include "EnemyInterceptor.h"

enum class GameState
{
    Menu,
    Playing,
    Paused,
    GameOver,
    Victory
};

#pragma once

#include "raylib.h"
#include "Player.h"

class Game
{

private:
    int   m_screenWidth;
    int   m_screenHeight;
    float m_worldWidth;
    float m_worldHeight;

    Player   m_player; // Game owns the Player instance
    Camera2D m_camera; // Game owns and manages the Camera2D instance

    // Starfield decoration scattered across the world
    struct Star
    {
        Vector2 position;
        float   radius;
        unsigned char alpha;
    };
    std::vector<Star> m_stars;
    std::vector<Bullet> m_bullets; // Active player projectiles
    std::vector<HealingStone> m_healingStones; // Active healing pickups
    float m_stoneSpawnTimer;                   // Countdown to next spawn attempt
    Enemy m_testEnemy; // Milestone 1: single test enemy (rear chaser)
    EnemyInterceptor m_testEnemy2; // Front interceptor test enemy

    // Exclusive contact engagement: only ONE enemy may overlap the player.
    // m_contactOwner: 0 = none, 1 = Enemy Type 1, 2 = Enemy Type 2.
    int m_contactOwner;

    // Enemy activation delay (game start / player respawn) and per-enemy
    // respawn timers after destruction.
    float m_enemyActivateTimer;
    float m_enemy1RespawnTimer;
    float m_enemy2RespawnTimer;

    // Scoring: total score + per-type kill counters (persist across respawns)
    int m_score;
    int m_t1Kills;
    int m_t2Kills;

    // Internal helper to smoothly move camera.target toward m_player.m_position
    void UpdateCamera(float dt);

    // Projectile management
    void SpawnPlayerBullet();
    void UpdateBullets(float dt);

    // Healing stone management
    void UpdateHealingStones(float dt);
    void TrySpawnHealingStone();
    bool TryFindStoneSpawnPos(Vector2& outPos) const;

    // Enemy spawn repositioning + exclusive contact engagement
    void RepositionEnemiesInView();
    bool TryFindEnemySpawnPos(Vector2 viewCenter, const std::vector<Vector2>& avoidPos,
                              Vector2& outPos) const;
    void UpdateEnemyContacts();
    void CheckBulletEnemyHits();
    void UpdateEnemyRespawns(float dt);

public:
    Game(int screenWidth = 1280, int screenHeight = 720);
    ~Game() = default;

    // Initializes the player, world dimensions, and camera parameters
    void Init();

    // Runs per-frame logic: updates player and smoothly interpolates the camera
    void Update(float dt);

    // Coordinates rendering: world inside BeginMode2D, HUD/UI outside
    void Draw();
};