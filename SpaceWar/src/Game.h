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

enum class GameState
{
    Menu,
    Playing,
    Paused,
    GameOver,
    Victory
};

class Game
{
private:
    int screenWidth;
    int screenHeight;
    GameState currentState;

    Player player;
    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;
    std::vector<Asteroid> asteroids;
    Boss boss;
    AssetManager assetManager;

public:
    // Constructor and Destructor
    Game();
    ~Game();

    // Core game loop methods
    void Init();
    void Run();
    void Update(float deltaTime);
    void Draw() const;

    // Coordination and state management methods
    void HandleInput();
    void CheckCollisions();
    void SpawnEnemies();
    void ChangeGameState(GameState newState);
};
