// main.cpp
// Purpose: Entry point of the SPACEWAR application.
//
// Currently contains a minimal raylib integration test that:
// 1. Initializes a 1280x720 window titled "SPACEWAR".
// 2. Displays a simple dark background and the text "SPACEWAR".
// 3. Closes the window cleanly when the user exits.
//
// Future TODO:
// - Replace the minimal window loop with an instance of the Game class
//   (e.g., Game game; game.Init(); game.Run();).

#include "raylib.h"

int main()
{
    // Window dimensions for the initial integration test
    const int screenWidth = 1280;
    const int screenHeight = 720;

    // Initialize the raylib window and set target FPS
    InitWindow(screenWidth, screenHeight, "SPACEWAR");
    SetTargetFPS(60);

    // Main application loop: runs until the user closes the window or presses ESC
    while (!WindowShouldClose())
    {
        // Begin drawing frame
        BeginDrawing();

        // Clear the screen with a dark space-like background color
        ClearBackground(BLACK);

        // Draw the game title centered near the middle of the screen
        const char* titleText = "SPACEWAR";
        const int fontSize = 48;
        const int textWidth = MeasureText(titleText, fontSize);
        DrawText(titleText, (screenWidth - textWidth) / 2, (screenHeight - fontSize) / 2, fontSize, RAYWHITE);

        // End drawing frame
        EndDrawing();
    }

    // Close the window and unload OpenGL context cleanly
    CloseWindow();

    return 0;
}
