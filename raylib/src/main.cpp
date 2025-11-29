#include "raylib.h"
#include "Game.h"
#include <iostream>

using namespace std;

int main() {
    const int SCREEN_WIDTH  = 1280;
    const int SCREEN_HEIGHT = 720;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "BlastForge - Tactical Tank Warfare");
    SetTargetFPS(60);
         
    Game game(SCREEN_WIDTH, SCREEN_HEIGHT);
    game.Init();

    while (!WindowShouldClose() && !game.ShouldQuit()) {
        float dt = GetFrameTime();
        game.Update(dt);
        BeginDrawing();
        game.Draw();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}