// AssetManager.cpp
// Purpose: Implements the methods declared in AssetManager.h.
//
// This file is responsible for loading and unloading all textures, fonts,
// sound effects, and music tracks from the assets/ directory.

#include "AssetManager.h"

AssetManager::AssetManager()
{
    // TODO: Initialize any default asset manager state if needed.
}

AssetManager::~AssetManager()
{
    // TODO: Ensure UnloadAllAssets() is called to prevent memory leaks.
}

void AssetManager::LoadTextures()
{
    // TODO: Load player, enemy, boss, bullet, and background textures from assets/.
}

void AssetManager::LoadFonts()
{
    // TODO: Load pixel-art fonts from assets/fonts/.
}

void AssetManager::LoadSounds()
{
    // TODO: Load sound effects from assets/audio/sfx/ and music from assets/audio/music/.
}

void AssetManager::LoadAllAssets()
{
    // TODO: Call LoadTextures(), LoadFonts(), and LoadSounds() in order.
}

void AssetManager::UnloadAllAssets()
{
    // TODO: Unload all textures, fonts, sounds, and music tracks using raylib Unload* functions.
}
