#include "Game.h"
#include "Theme.h"
#include <cmath>
#include <iostream>

using namespace std;

// Helper: Grid Background
void DrawSchematicBG(int w, int h) {
    ClearBackground(Theme::MenuBG);
    for (int i = 0; i < w; i += 40) DrawLine(i, 0, i, h, Fade(WHITE, 0.05f));
    for (int i = 0; i < h; i += 40) DrawLine(0, i, w, i, Fade(WHITE, 0.05f));
    DrawRectangleGradientV(0, h/2, w, h/2, Fade(BLACK, 0.0f), Fade(BLACK, 0.4f));
}

// Helper: Draw Cactus
void DrawCactus(int x, int y) {
    Color c = { 34, 139, 34, 255 }; // Forest Green
    DrawRectangle(x, y - 60, 16, 60, c);           // Main Stem
    DrawRectangle(x - 12, y - 45, 12, 10, c);      // Left arm connector
    DrawRectangle(x - 12, y - 55, 6, 10, c);       // Left arm tip
    DrawRectangle(x + 16, y - 40, 10, 10, c);      // Right arm connector
    DrawRectangle(x + 20, y - 50, 6, 10, c);       // Right arm tip
}

static float GroundY(int screenH) {
    return screenH - 80.0f;
}

Game::Game(int W, int H)
    : w(W), h(H),
      state(GameState::Menu),
      bg(BackgroundType::Desert),
      turn(0),
      power{0.0f, 0.0f},
      paused(false),
      winner(-1),
      wantQuit(false)
{
}

void Game::Init() {
    float gy = GroundY(h);
    tank[0].Init({ 150.0f, gy });
    tank[1].Init({ (float)w - 150.0f, gy });

    float cx = w / 2.0f;

    // Buttons
    btnPlay = Button({ cx - 100.0f, h/2.0f - 40.0f, 200.0f, 80.0f }, "PLAY", 'P', KEY_P);
    
    btnDesert = Button({ cx - 260.0f, h/2.0f - 40.0f, 220.0f, 80.0f }, "DESERT", 'D', KEY_D);
    btnMoon   = Button({ cx + 40.0f, h/2.0f - 40.0f, 220.0f, 80.0f }, "MOON", 'M', KEY_M);

    // In-Game UI Buttons
    btnSwitchTheme = Button({ cx - 290.0f, 20.0f, 120.0f, 40.0f }, "THEME", 'T', KEY_T);
    btnPause  = Button({ cx - 160.0f, 20.0f, 120.0f, 40.0f }, "PAUSE", 'U', KEY_U);
    btnRestart= Button({ cx + 40.0f, 20.0f, 120.0f, 40.0f }, "RESTART", 'N', KEY_N);
    
    btnResume = Button({ cx - 80.0f, h/2.0f + 40.0f, 160.0f, 40.0f }, "RESUME", 'C', KEY_C);
    
    btnExit   = Button({ cx - 60.0f, (float)h - 50.0f, 120.0f, 30.0f }, "EXIT", 'X', KEY_X);
}

void Game::Reset() {
    float gy = GroundY(h);
    tank[0].Init({ 150.0f, gy });
    tank[1].Init({ (float)w - 150.0f, gy });
    shots.clear();
    power[0] = 0.0f; power[1] = 0.0f;
    paused = false; winner = -1; turn = 0;
    state = GameState::Playing;
}

// ---------------- UPDATE --------------------
void Game::Update(float dt) {
    if (IsKeyPressed(KEY_H)) { wantQuit = true; return; }

    switch (state) {
        case GameState::Menu:             UpdateMenu();      break;
        case GameState::BackgroundSelect: UpdateBGSelect();  break;
        case GameState::Playing:          UpdateGame(dt);    break;
        case GameState::GameOver:         UpdateGameOver();  break;
    }
}

void Game::UpdateMenu() {
    if (btnPlay.WasClicked()) state = GameState::BackgroundSelect;
    if (btnExit.WasClicked()) wantQuit = true;
}

void Game::UpdateBGSelect() {
    if (btnDesert.WasClicked()) { bg = BackgroundType::Desert; Reset(); }
    if (btnMoon.WasClicked())   { bg = BackgroundType::MoonNight; Reset(); }
    if (btnExit.WasClicked())   wantQuit = true;
}

void Game::UpdateGame(float dt) {
    if (btnExit.WasClicked()) { wantQuit = true; return; }
    if (btnPause.WasClicked()) paused = !paused;
    if (paused) {
        if (btnResume.WasClicked()) paused = false;
        return;
    }
    
    // Toggle Theme Button
    if (btnSwitchTheme.WasClicked()) {
        if (bg == BackgroundType::Desert) bg = BackgroundType::MoonNight;
        else bg = BackgroundType::Desert;
    }

    if (btnRestart.WasClicked()) { Reset(); return; }

    float& curPower = power[turn];
    if (IsKeyDown(KEY_Q)) curPower -= dt * 0.5f;
    if (IsKeyDown(KEY_E)) curPower += dt * 0.5f;
    if (curPower < 0.0f) curPower = 0.0f;
    if (curPower > 1.0f) curPower = 1.0f;

    const float BASE_SPEED = 300.0f;
    const float POWER_MULT = 300.0f;

    if (turn == 0) {
        tank[0].Update(true, KEY_A, KEY_D, KEY_W, KEY_S, dt);
        tank[1].Update(false, 0, 0, 0, 0, dt);

        if (IsKeyPressed(KEY_SPACE)) {
            float usedPower = (curPower < 0.1f) ? 0.1f : curPower;
            Vector2 tip = tank[0].GetBarrelTip();
            float ang   = tank[0].GetBarrelAngleRad();
            float spd   = BASE_SPEED + POWER_MULT * usedPower;
            
            Projectile p;
            p.Fire(tip, { cosf(ang)*spd, -sinf(ang)*spd }, WeaponType::Rocket, 0);
            shots.push_back(p);
            curPower = 0.0f; turn = 1;
        }
    } else {
        tank[1].Update(true, KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN, dt);
        tank[0].Update(false, 0, 0, 0, 0, dt);

        if (IsKeyPressed(KEY_ENTER)) {
            float usedPower = (curPower < 0.1f) ? 0.1f : curPower;
            Vector2 tip = tank[1].GetBarrelTip();
            float ang   = tank[1].GetBarrelAngleRad();
            float spd   = BASE_SPEED + POWER_MULT * usedPower;

            Projectile p;
            p.Fire(tip, { cosf(ang)*spd, -sinf(ang)*spd }, WeaponType::Rocket, 1);
            shots.push_back(p);
            curPower = 0.0f; turn = 0;
        }
    }

    for (auto& s : shots) if (s.Active()) s.Update(dt);
    CheckCollisions();
}

void Game::CheckCollisions() {
    for (auto& s : shots) {
        if (!s.Active()) continue;
        int owner  = s.GetOwner();
        int target = (owner == 0 ? 1 : 0);

        if (CheckCollisionRecs(s.GetRect(), tank[target].GetBody())) {
            tank[target].TakeDamage(35);
            s.Deactivate();
            if (tank[target].IsDead()) {
                winner = owner;
                state  = GameState::GameOver;
            }
        }
    }
}

// ---------------- DRAW --------------------
void Game::Draw() {
    switch (state) {
        case GameState::Menu:             DrawMenu();      break;
        case GameState::BackgroundSelect: DrawBGSelect();  break;
        case GameState::Playing:          DrawGame();      break;
        case GameState::GameOver:         DrawGameOver();  break;
    }
}

void Game::DrawMenu() {
    DrawSchematicBG(w, h);
    
    // Updated: Ground is now YELLOW (Theme::Accent) to contrast with MenuBG
    DrawRectangle(0, (int)GroundY(h), w, 80, Theme::Accent);
    
    // Title
    const char* title = "BlastForge";
    int font = 70;
    int tw = MeasureText(title, font);
    int tx = w/2 - tw/2;
    int ty = 100;
    
    DrawText(title, tx + 6, ty + 6, font, BLACK); // Shadow
    DrawText(title, tx, ty, font, Theme::Accent); // Main
    DrawText("Tactical Tank Warfare", tx + 20, ty + 80, 20, LIGHTGRAY);

    btnPlay.Draw();
    btnExit.Draw();
}

void Game::DrawBGSelect() {
    DrawSchematicBG(w, h);
    const char* txt = "SELECT BATTLEFIELD";
    int font = 40;
    int tw   = MeasureText(txt, font);

    DrawText(txt, w/2 - tw/2 + 3, 100 + 3, font, BLACK);
    DrawText(txt, w/2 - tw/2, 100, font, Theme::Text);

    btnDesert.Draw();
    btnMoon.Draw();
    btnExit.Draw();
}

void Game::DrawGame() {
    // 1. Determine Colors dynamically based on Background
    Color p1C = (bg == BackgroundType::Desert) ? Theme::P1_Color_Desert : Theme::P1_Color_Moon;
    Color p2C = (bg == BackgroundType::Desert) ? Theme::P2_Color_Desert : Theme::P2_Color_Moon;

    float gy = GroundY(h);

    if (bg == BackgroundType::Desert) {
        ClearBackground(Theme::DesertSky);
        DrawCircleV({ 120.0f, 100.0f }, 40.0f, ORANGE); 
        
        // DRAW CACTI
        DrawCactus(250, (int)gy);
        DrawCactus(800, (int)gy);
        DrawCactus(w - 200, (int)gy);

    } else {
        ClearBackground(Theme::MoonSky);
        DrawCircleV({ (float)w - 150.0f, 120.0f }, 60, RAYWHITE); 
        DrawCircle(100, 80, 2, WHITE); DrawCircle(500, 110, 2, WHITE);
        DrawCircle(800, 150, 3, WHITE); DrawCircle(950, 90, 2, WHITE);
    }

    DrawRectangle(0, (int)gy, w, 80, Theme::Ground);

    // 2. Draw Tanks with the dynamic colors
    tank[0].Draw(p1C);
    tank[1].Draw(p2C);

    // Trajectory
    const float GRAVITY = 300.0f;
    Tank& active = (turn == 0 ? tank[0] : tank[1]);
    float usedPower = (power[turn] < 0.1f) ? 0.1f : power[turn];

    Vector2 start = active.GetBarrelTip();
    float   ang   = active.GetBarrelAngleRad();
    float   speed = 300.0f + 300.0f * usedPower;
    Vector2 v0    = { cosf(ang)*speed, -sinf(ang)*speed };

    // Trajectory matches player color
    Color arcColor = (turn == 0 ? p1C : p2C);

    for (int i = 0; i < 7; ++i) {
        float t = (0.6f + 0.2f * usedPower) * (float)i / 6.0f;
        float x = start.x + v0.x * t;
        float y = start.y + v0.y * t + 0.5f * GRAVITY * t * t;
        if (y > gy) break;
        DrawCircleV({ x, y }, 3, arcColor);
    }

    for (auto& s : shots) s.Draw();

    // UI HUD
    float maxW = 220.0f;
    
    // P1 HUD
    float hp1 = tank[0].GetHealth() / 100.0f;
    DrawText("PLAYER 1", 20, 20, 20, p1C); 
    DrawRectangle(20, 45, (int)maxW, 18, Theme::BarBG);
    DrawRectangle(20, 45, (int)(maxW * hp1), 18, p1C); 
    DrawRectangleLines(20, 45, (int)maxW, 18, Theme::BarBorder);
    
    // P2 HUD
    float hp2 = tank[1].GetHealth() / 100.0f;
    float x2 = w - maxW - 20.0f;
    DrawText("PLAYER 2", (int)x2, 20, 20, p2C); 
    DrawRectangle((int)x2, 45, (int)maxW, 18, Theme::BarBG);
    DrawRectangle((int)x2, 45, (int)(maxW * hp2), 18, p2C); 
    DrawRectangleLines((int)x2, 45, (int)maxW, 18, Theme::BarBorder);

    // Power Bars
    float barW = 180.0f; float barH = 15.0f; float yBar = h - 80.0f;
    
    DrawText("POWER", 20, (int)yBar - 20, 16, Theme::Text);
    DrawRectangle(20, (int)yBar, (int)barW, (int)barH, Theme::BarBG);
    DrawRectangle(20, (int)yBar, (int)(barW * power[0]), (int)barH, p1C);
    DrawRectangleLines(20, (int)yBar, (int)barW, (int)barH, Theme::BarBorder);

    float p2x = w - barW - 20.0f;
    DrawText("POWER", (int)p2x, (int)yBar - 20, 16, Theme::Text);
    DrawRectangle((int)p2x, (int)yBar, (int)barW, (int)barH, Theme::BarBG);
    DrawRectangle((int)p2x, (int)yBar, (int)(barW * power[1]), (int)barH, p2C);
    DrawRectangleLines((int)p2x, (int)yBar, (int)barW, (int)barH, Theme::BarBorder);

    // UI BUTTONS
    btnSwitchTheme.Draw(); // Draw new button
    btnPause.Draw();
    btnRestart.Draw();
    btnExit.Draw();

    // Turn Text (Top Center)
    const char* turnTxt = (turn == 0 ? "Turn: Player 1" : "Turn: Player 2");
    DrawText(turnTxt, w/2 - MeasureText(turnTxt, 20)/2, 75, 20, Theme::Text);

    if (paused) {
        DrawRectangle(0, 0, w, h, Fade(BLACK, 0.7f));
        const char* txt = "PAUSED";
        int fs = 60;
        DrawText(txt, w/2 - MeasureText(txt, fs)/2, h/2 - 100, fs, Theme::Text);
        btnResume.Draw();
    }
}

void Game::UpdateGameOver() {
    if (btnRestart.WasClicked() || IsKeyPressed(KEY_R)) Reset();
    if (btnExit.WasClicked()) wantQuit = true;
}

void Game::DrawGameOver() {
    DrawGame();
    DrawRectangle(0, 0, w, h, Fade(BLACK, 0.7f));

    int boxW = 400; int boxH = 250;
    int bx = w/2 - boxW/2; int by = h/2 - boxH/2;
    DrawRectangle(bx, by, boxW, boxH, Theme::MenuBG);
    DrawRectangleLines(bx, by, boxW, boxH, Theme::Accent);

    const char* msg = (winner == 0 ? "PLAYER 1 WINS!" : (winner == 1 ? "PLAYER 2 WINS!" : "DRAW!"));
    DrawText(msg, w/2 - MeasureText(msg, 30)/2, by + 40, 30, Theme::Accent);
    DrawText("Press R to Restart", w/2 - MeasureText("Press R to Restart", 20)/2, by + 100, 20, WHITE);

    btnExit.Draw();
}