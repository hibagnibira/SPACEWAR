// ============================================================================
// File: main.cpp
// Description: Test harness for the mouse-following movement mechanics in SPACEWAR.
//              Tests autonomous steering, smooth arrival deceleration,
//              stopping at the cursor without oscillation, and boundary clamping.
// ============================================================================

#include "raylib.h"
#include "Player.h"

int main()
{
    // ------------------------------------------------------------------------
    // 1. Initialization
    // ------------------------------------------------------------------------
    constexpr int screenWidth = 1280;
    constexpr int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "SPACEWAR — Mouse-Following Test");
    SetTargetFPS(60);

    // Instantiate player at screen center with screen boundaries
    const Vector2 spawnPosition = {
        static_cast<float>(screenWidth) / 2.0f,
        static_cast<float>(screenHeight) / 2.0f
    };

    Player player(spawnPosition,
        static_cast<float>(screenWidth),
        static_cast<float>(screenHeight));

    // Deep space background color
    const Color spaceColor = Color{ 8, 10, 18, 255 };

    // ------------------------------------------------------------------------
    // 2. Main Game Loop
    // ------------------------------------------------------------------------
    while (!WindowShouldClose())
    {
        // --------------------------------------------------------------------
        // Update Phase
        // --------------------------------------------------------------------
        const float dt = GetFrameTime();

        // The player automatically handles:
        // - Fetching GetMousePosition()
        // - Steering toward cursor
        // - Smoothly decelerating as it arrives (no overshoot / no oscillation)
        // - Stopping completely at STOP_DISTANCE
        // - Clamping to world boundaries
        player.Update(dt);

        // --------------------------------------------------------------------
        // Render Phase
        // --------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(spaceColor);

        // Draw a subtle reference grid to make movement and deceleration visible
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

        // Draw a visual target reticle at the mouse cursor
        const Vector2 mousePos = GetMousePosition();
        DrawCircleLinesV(mousePos, 10.0f, Color{ 80, 160, 240, 120 });
        DrawCircleV(mousePos, 2.0f, Color{ 120, 200, 255, 200 });

        // Draw the spaceship and thruster exhaust
        player.Draw();

        // ----------------------------------------------------------------
        // Diagnostic & Instruction Overlay
        // ----------------------------------------------------------------
        DrawText("SPACEWAR: Autonomous Mouse-Following Test", 20, 20, 20, RAYWHITE);
        DrawText("- Move mouse to guide the spaceship", 20, 50, 16, LIGHTGRAY);
        DrawText("- Ship accelerates up to 420 px/s", 20, 72, 16, LIGHTGRAY);
        DrawText("- Ship smoothly decelerates and stops at cursor (no overshoot)", 20, 94, 16, LIGHTGRAY);
        DrawText("- Thruster flames extinguish automatically upon arrival", 20, 116, 16, GRAY);

        DrawFPS(screenWidth - 100, 20);

        EndDrawing();
    }

    // ------------------------------------------------------------------------
    // 3. De-initialization
    // ------------------------------------------------------------------------
    CloseWindow();

    return 0;
}