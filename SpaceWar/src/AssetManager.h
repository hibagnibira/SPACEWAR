// AssetManager.h
// Purpose: Declares the AssetManager class, which centralizes loading,
// storing, and unloading all game resources (textures, fonts, sounds, music).
//
// Centralizing assets prevents loading the same file multiple times during
// gameplay and ensures all resources are properly unloaded when the game exits.
//
// Attributes:
// - playerTextures: Collection of textures for the player spaceship.
// - enemyTextures: Collection of textures for standard enemy ships.
// - bossTextures: Collection of textures for the boss enemy.
// - bulletTextures: Collection of textures for player and enemy projectiles.
// - backgroundTextures: Collection of textures for parallax backgrounds.
// - fonts: Collection of loaded fonts for UI and HUD text.
// - soundEffects: Collection of loaded sound effects (SFX).
// - musicTracks: Collection of loaded background music streams.
//
// Methods:
// - LoadTextures(): Loads all sprite and background textures.
// - LoadFonts(): Loads all fonts used in menus and HUD.
// - LoadSounds(): Loads all sound effects and music tracks.
// - LoadAllAssets(): Calls all individual load methods.
// - UnloadAllAssets(): Unloads all loaded resources from memory.

#pragma once

#include <vector>
#include "raylib.h"

class AssetManager
{
private:
    std::vector<Texture2D> playerTextures;
    std::vector<Texture2D> enemyTextures;
    std::vector<Texture2D> bossTextures;
    std::vector<Texture2D> bulletTextures;
    std::vector<Texture2D> backgroundTextures;
    std::vector<Font> fonts;
    std::vector<Sound> soundEffects;
    std::vector<Music> musicTracks;

public:
    // Constructor and Destructor
    AssetManager();
    ~AssetManager();

    // Resource loading methods
    void LoadTextures();
    void LoadFonts();
    void LoadSounds();
    void LoadAllAssets();

    // Resource cleanup method
    void UnloadAllAssets();
};
