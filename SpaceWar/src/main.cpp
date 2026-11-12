// ============================================================================
// File: main.cpp
// Description: Entry point for testing the SPACEWAR Player movement system.
//              Initializes a raylib window, runs the game loop at 60 FPS,
//              passes delta time to Player::Update(), and renders the player.
// ============================================================================

#include "raylib.h"
#include "Player.h"

int main()
{
    // ------------------------------------------------------------------------
    // 1. Window & Environment Configuration
    // ------------------------------------------------------------------------
    constexpr int screenWidth = 1280;
    constexpr int screenHeight = 720;

    // Initialize raylib window with an OpenGL context
    InitWindow(screenWidth, screenHeight, "SPACEWAR — Player Movement Test");

    // Cap the framerate to monitor refresh rate or a steady 60 FPS
    SetTargetFPS(60);

    // ------------------------------------------------------------------------
    // 2. Player Instantiation
    // Spawn the player at the center of our screen and configure the boundaries
    // to match the screen's dimensions.
    // ------------------------------------------------------------------------
    const Vector2 spawnPosition = {
        static_cast<float>(screenWidth) / 2.0f,
        static_cast<float>(screenHeight) / 2.0f
    };

    Player player(spawnPosition,
        static_cast<float>(screenWidth),
        static_cast<float>(screenHeight));

    // Space background color (deep dark navy)
    const Color spaceColor = Color{ 8, 10, 18, 255 };

    // ------------------------------------------------------------------------
    // 3. Main Game Loop
    // Continues until the window close button or the ESC key is pressed.
    // ------------------------------------------------------------------------
    while (!WindowShouldClose())
    {
        // --------------------------------------------------------------------
        // Update Phase
        // GetFrameTime() returns the duration in seconds of the previous frame.
        // --------------------------------------------------------------------
        const float dt = GetFrameTime();

        // The Player class internally handles:
        // - Aiming at GetMousePosition()
        // - Checking 'W' and 'S' key inputs
        // - Applying acceleration, momentum, and drag
        // - Clamping against world boundaries
        player.Update(dt);

        // --------------------------------------------------------------------
        // Render Phase
        // --------------------------------------------------------------------
        BeginDrawing();

        // Clear previous frame buffer with dark space color
        ClearBackground(spaceColor);

        // Draw a subtle background grid to make momentum and drift clearly visible
        constexpr int gridSize = 60;
        const Color gridColor = Color{ 25, 30, 45, 255 };

        for (int x = 0; x < screenWidth; x += gridSize)
        {
            DrawLine(x, 0, x, screenHeight, gridColor);
        }
        for (int y = 0; y < screenHeight; y += gridSize)
        {
            DrawLine(0, y, screenWidth, y, gridColor);
        }

        // Draw the spaceship and its thruster exhaust
        player.Draw();

        // ----------------------------------------------------------------
        // On-screen Testing Instructions & Diagnostics
        // ----------------------------------------------------------------
        DrawText("SPACEWAR: Basic Movement Test", 20, 20, 20, RAYWHITE);
        DrawText("- Mouse: Aim / Rotate", 20, 50, 16, LIGHTGRAY);
        DrawText("- W: Forward Thrust", 20, 72, 16, LIGHTGRAY);
        DrawText("- S: Reverse Thrust", 20, 94, 16, LIGHTGRAY);
        DrawText("- Release keys to test natural drag & momentum", 20, 116, 16, GRAY);

        // Display current FPS in the top right corner
        DrawFPS(screenWidth - 100, 20);

        EndDrawing();
    }

    // ------------------------------------------------------------------------
    // 4. Cleanup & Shutdown
    // ------------------------------------------------------------------------
    CloseWindow();

    return 0;
}