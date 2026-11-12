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

    // Configurable playable world larger than the viewport
    constexpr float worldWidth = 2400.0f;
    constexpr float worldHeight = 1800.0f;

    // const Vector2 spawnPosition = { worldWidth / 2.0f, worldHeight / 2.0f };

    Player player(spawnPosition, worldWidth, worldHeight);

    // ------------------------------------------------------------------------
    // Default Camera Framing (Rectangle Around Player)
    // Define the exact width and height of the world area visible around the ship.
    // Smaller numbers = closer/tighter rectangle; Larger numbers = wider view.
    // ------------------------------------------------------------------------
    constexpr float viewRectWidth = 640.0f; // Width of visible area around ship
    constexpr float viewRectHeight = 360.0f; // Height of visible area around ship

    Camera2D camera = { 0 };
    camera.target = spawnPosition;
    camera.offset = Vector2{ screenWidth / 3.0f, screenHeight / 3.0f }; // Keeps player dead-center
    camera.rotation = 0.0f;

    // Set fixed default zoom to 1.5x magnification
    camera.zoom = 1.5f;
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
        // Tell the player to update its movement and track the camera
        player.Update(dt, camera);

        // --------------------------------------------------------------------
        // Render Phase
        // --------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(spaceColor);

        // ----------------------------------------------------------------
        // 1. World Space Rendering (Inside Camera Transformation)
        // ----------------------------------------------------------------
        BeginMode2D(camera);

        // Playable world reference grid
        constexpr int gridSize = 80;
        const Color gridColor = Color{ 25, 30, 45, 255 };

        for (int x = 0; x <= static_cast<int>(worldWidth); x += gridSize)
        {
            DrawLine(x, 0, x, static_cast<int>(worldHeight), gridColor);
        }
        for (int y = 0; y <= static_cast<int>(worldHeight); y += gridSize)
        {
            DrawLine(0, y, static_cast<int>(worldWidth), y, gridColor);
        }

        // Playable boundary border
        DrawRectangleLinesEx(Rectangle{ 0.0f, 0.0f, worldWidth, worldHeight }, 4.0f, Fade(RED, 0.6f));

        // Mouse cursor target indicator in world coordinates
        const Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
        DrawCircleLinesV(mouseWorldPos, 10.0f, Color{ 80, 160, 240, 140 });
        DrawCircleV(mouseWorldPos, 2.0f, Color{ 120, 200, 255, 200 });

        // Render spaceship
        player.Draw();

        EndMode2D();

        // ----------------------------------------------------------------
        // 2. Screen Space Rendering (Fixed UI / HUD - Outside Camera)
        // ----------------------------------------------------------------
        DrawText("SPACEWAR: Camera Following System", 20, 20, 20, RAYWHITE);
        DrawText("- Encapsulated camera tracking (Zero getters/setters)", 20, 50, 16, LIGHTGRAY);
        DrawText("- Mouse coordinates converted to world space via GetScreenToWorld2D", 20, 72, 16, LIGHTGRAY);
        DrawText("- Red border outlines playable world boundaries", 20, 94, 16, LIGHTGRAY);

        DrawFPS(screenWidth - 100, 20);

        EndDrawing();
    }

    // ------------------------------------------------------------------------
    // 3. De-initialization
    // ------------------------------------------------------------------------
    CloseWindow();

    return 0;
}