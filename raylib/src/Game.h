#pragma once
#include "raylib.h"
#include <vector>
#include "Tank.h"
#include "Projectile.h"
#include "Button.h"
#include "Types.h"

class Game {
private:
    int w, h;

    GameState      state;
    BackgroundType bg;

    Tank tank[2];
    std::vector<Projectile> shots;

    // UI buttons
    Button btnPlay;
    Button btnDesert;
    Button btnMoon;
    Button btnPause;
    Button btnRestart;
    Button btnSwitchTheme; // NEW: Theme Switch Button
    Button btnResume;   
    Button btnExit;     

    int   turn;        
    float power[2];     
    bool  paused;
    int   winner;
    bool  wantQuit;     

    void Reset();
    void UpdateMenu();
    void UpdateBGSelect();
    void UpdateGame(float dt);
    void UpdateGameOver();
    void CheckCollisions();

    void DrawMenu();
    void DrawBGSelect();
    void DrawGame();
    void DrawGameOver();

public:
    Game(int W, int H);
    void Init();
    void Update(float dt);
    void Draw();

    bool ShouldQuit() const { return wantQuit; }
};