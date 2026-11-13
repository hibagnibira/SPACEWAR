// ============================================================================
// File: main.cpp
// Description: Test harness for the mouse-following movement mechanics in SPACEWAR.
//              Tests autonomous steering, smooth arrival deceleration,
//              stopping at the cursor without oscillation, and boundary clamping.
// ============================================================================

#include "raylib.h"
#include "Game.h"

int main()
{
    constexpr int screenWidth = 1280;
    constexpr int screenHeight = 720;

    // 1. Initialize window
    InitWindow(screenWidth, screenHeight, "SPACEWAR");
    SetTargetFPS(60);

    // 2. Create Game object and initialize
    Game game(screenWidth, screenHeight);
    game.Init();

    // 3. Main game loop (clean delegation with zero camera/player code)
    while (!WindowShouldClose())
    {
        const float dt = GetFrameTime();
        game.Update(dt);
        game.Draw();
    }

    // 4. Shutdown
    CloseWindow();
    return 0;
}