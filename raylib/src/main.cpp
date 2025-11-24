#include "raylib.h"
#include <string>
#include <cmath>
#include <vector>

// ---------------------------------------------------------
// General constants
// ---------------------------------------------------------
const int   SCREEN_WIDTH  = 1280;
const int   SCREEN_HEIGHT = 720;
const float GRAVITY       = 350.0f;
const float GROUND_Y      = SCREEN_HEIGHT - 120.0f;

// ---------------------------------------------------------
// Helpers
// ---------------------------------------------------------
float ClampFloat(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

// ---------------------------------------------------------
// Basic Button using a texture
// ---------------------------------------------------------
class ImageButton {
public:
    Texture2D texture{};
    Rectangle bounds{};
    bool loaded = false;

    ImageButton() = default;

    void Load(const std::string &path, Vector2 center, float scale = 1.0f) {
        Image img = LoadImage(path.c_str());
        if (img.data) {
            texture = LoadTextureFromImage(img);
            UnloadImage(img);
            loaded = true;
            bounds.width  = texture.width * scale;
            bounds.height = texture.height * scale;
            bounds.x = center.x - bounds.width/2;
            bounds.y = center.y - bounds.height/2;
        }
    }

    bool Update() {
        if (!loaded) return false;
        Vector2 mouse = GetMousePosition();
        bool hovered = CheckCollisionPointRec(mouse, bounds);
        if (hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            return true;
        }
        return false;
    }

    void Draw() const {
        if (!loaded) return;
        DrawTexturePro(
            texture,
            Rectangle{0,0,(float)texture.width,(float)texture.height},
            bounds,
            Vector2{0,0},
            0.0f,
            WHITE
        );
    }

    void DrawTint(Color tint) const {
        if (!loaded) return;
        DrawTexturePro(
            texture,
            Rectangle{0,0,(float)texture.width,(float)texture.height},
            bounds,
            Vector2{0,0},
            0.0f,
            tint
        );
    }
};

// ---------------------------------------------------------
// Projectile & weapon types
// ---------------------------------------------------------
enum class WeaponType { BULLET = 0, ROCKET, BOMB };

class Projectile {
public:
    Vector2 pos{0,0};
    Vector2 vel{0,0};
    float radius = 10.0f;
    bool active = false;

    WeaponType type = WeaponType::BULLET;
    Texture2D sprite{};
    bool hasSprite = false;
    float spriteScale = 0.6f;
    float rotationDeg = 0.0f;

    void Launch(Vector2 start, float angleDeg, float power, WeaponType wtype) {
        pos = start;
        float rad = angleDeg * DEG2RAD;
        float speed = power;
        vel.x = cosf(rad) * speed;
        vel.y = -sinf(rad) * speed;
        active = true;
        type = wtype;
    }

    void Update(float dt) {
        if (!active) return;
        vel.y += GRAVITY * dt;
        pos.x += vel.x * dt;
        pos.y += vel.y * dt;
        rotationDeg = atan2f(vel.y, vel.x) * RAD2DEG;

        if (pos.y > SCREEN_HEIGHT + 100 || pos.x < -200 || pos.x > SCREEN_WIDTH + 200)
            active = false;
        if (pos.y >= GROUND_Y) {
            pos.y = GROUND_Y;
            active = false;
        }
    }

    void Draw() const {
        if (!active) return;
        if (hasSprite) {
            float w = sprite.width * spriteScale;
            float h = sprite.height * spriteScale;
            Rectangle src{0,0,(float)sprite.width,(float)sprite.height};
            Rectangle dst{pos.x - w/2, pos.y - h/2, w, h};
            DrawTexturePro(sprite, src, dst, Vector2{w/2,h/2}, rotationDeg, WHITE);
        } else {
            DrawCircleV(pos, radius, YELLOW);
        }
    }
};

// ---------------------------------------------------------
// Tank
// ---------------------------------------------------------
class Tank {
public:
    std::string name = "Player";
    Vector2 pos{0,0};
    bool facingRight = true;

    int maxHealth = 100;
    int health    = 100;

    Texture2D normal{};
    Texture2D damaged{};
    Texture2D dead{};
    bool texNormalLoaded=false, texDamagedLoaded=false, texDeadLoaded=false;

    float angleDeg = 45.0f;
    float power    = 350.0f;

    void Draw() const {
        const Texture2D* currentTex = &normal;
        if (!IsAlive() && texDeadLoaded) currentTex = &dead;
        else if (health < maxHealth/2 && texDamagedLoaded) currentTex = &damaged;

        float scale = 0.55f;
        float w = currentTex->width * scale;
        float h = currentTex->height* scale;

        Rectangle src{0,0,(float)currentTex->width,(float)currentTex->height};
        if (!facingRight) src.width = -src.width;

        Rectangle dst{pos.x - w/2, pos.y - h, w, h};
        DrawTexturePro(*currentTex, src, dst, Vector2{0,0}, 0.0f, WHITE);

        // barrel visualization
        Vector2 barrelBase{pos.x, pos.y - h*0.7f};
        float rad = angleDeg * DEG2RAD;
        if (!facingRight) rad = (180.0f - angleDeg)*DEG2RAD;

        float len = 60.0f;
        Vector2 barrelEnd{barrelBase.x + cosf(rad)*len,
                          barrelBase.y - sinf(rad)*len};
        DrawLineEx(barrelBase, barrelEnd, 6.0f, WHITE);
    }

    bool IsAlive() const { return health > 0; }

    Rectangle Hitbox() const {
        float scale = 0.55f;
        float w = normal.width * scale;
        float h = normal.height* scale;
        return Rectangle{pos.x - w/2, pos.y - h, w, h};
    }
};

// ---------------------------------------------------------
// HUD elements (health & power bars)
// ---------------------------------------------------------
class HealthBar {
public:
    Texture2D frame{};
    Texture2D full{};
    Texture2D empty{};
    bool loaded = false;

    Rectangle frameDest{};
    float scale = 0.85f;

    void Load(const std::string& framePath,
              const std::string& fullPath,
              const std::string& emptyPath,
              Vector2 center) {

        Image imgF = LoadImage(framePath.c_str());
        Image imgFull = LoadImage(fullPath.c_str());
        Image imgEmpty = LoadImage(emptyPath.c_str());

        if (imgF.data && imgFull.data && imgEmpty.data) {
            frame = LoadTextureFromImage(imgF);
            full  = LoadTextureFromImage(imgFull);
            empty = LoadTextureFromImage(imgEmpty);
            loaded = true;

            UnloadImage(imgF);
            UnloadImage(imgFull);
            UnloadImage(imgEmpty);

            frameDest.width  = frame.width * scale;
            frameDest.height = frame.height* scale;
            frameDest.x = center.x - frameDest.width/2;
            frameDest.y = center.y - frameDest.height/2;
        }
    }

    void Draw(int hp, int maxHp) const {
        if (!loaded) return;

        float ratio = ClampFloat((float)hp / (float)maxHp, 0.0f, 1.0f);

        // draw empty bar
        Rectangle emptyDst = frameDest;
        DrawTexturePro(empty, Rectangle{0,0,(float)empty.width,(float)empty.height},
                       emptyDst, Vector2{0,0}, 0.0f, WHITE);

        // draw full bar clipped horizontally
        float fullWidth = frameDest.width * ratio;
        Rectangle srcFull{0,0, (float)full.width * ratio, (float)full.height};
        Rectangle dstFull{frameDest.x, frameDest.y, fullWidth, frameDest.height};
        DrawTexturePro(full, srcFull, dstFull, Vector2{0,0}, 0.0f, WHITE);

        // frame
        DrawTexturePro(frame, Rectangle{0,0,(float)frame.width,(float)frame.height},
                       frameDest, Vector2{0,0}, 0.0f, WHITE);
    }
};

class PowerBar {
public:
    Texture2D texture{};
    bool loaded = false;
    Rectangle dest{};
    float scale = 0.7f;

    void Load(const std::string& path, Vector2 center) {
        Image img = LoadImage(path.c_str());
        if (img.data) {
            texture = LoadTextureFromImage(img);
            UnloadImage(img);
            loaded = true;
            dest.width  = texture.width * scale;
            dest.height = texture.height* scale;
            dest.x = center.x - dest.width/2;
            dest.y = center.y - dest.height/2;
        }
    }

    void Draw(float ratio) const {
        if (!loaded) return;
        ratio = ClampFloat(ratio, 0.0f, 1.0f);
        float visibleWidth = dest.width * ratio;
        Rectangle src{0,0,(float)texture.width * ratio,(float)texture.height};
        Rectangle dst{dest.x, dest.y, visibleWidth, dest.height};
        DrawTexturePro(texture, src, dst, Vector2{0,0}, 0.0f, WHITE);
    }
};

// ---------------------------------------------------------
// Game state & main Game class
// ---------------------------------------------------------
enum class GameScreen {
    MENU,
    VS_INTRO,
    PLAYING,
    RESULT
};

class Game {
public:
    GameScreen screen = GameScreen::MENU;

    // Backgrounds
    Texture2D bgDay{};
    Texture2D bgNight{};
    bool bgLoaded = false;
    bool useNight = false;

    // Tanks
    Tank tank1;
    Tank tank2;

    // HUD
    HealthBar hb1, hb2;
    PowerBar  powerBar;
    Texture2D aimIndicator{};
    bool aimLoaded = false;

    // Weapon selection
    Texture2D weaponSlot{};
    Texture2D weaponBulletIcon{};
    Texture2D weaponRocketIcon{};
    Texture2D weaponBombIcon{};
    bool weaponsLoaded = false;
    WeaponType currentWeapon = WeaponType::BULLET;

    // Projectile
    Projectile projectile;
    Texture2D projBullet{};
    Texture2D projRocket{};
    Texture2D projBomb{};
    bool projLoaded=false;

    // FX
    Texture2D explosion{};
    Texture2D cloud{};
    bool fxLoaded = false;

    // Meta UI
    Texture2D vsIcon{};
    Texture2D player1Label{};
    Texture2D victoryBanner{};
    Texture2D defeatBanner{};
    Texture2D chest{};
    Texture2D starFull{};
    Texture2D starEmpty{};
    bool metaLoaded=false;

    // Buttons
    ImageButton btnPlay;
    ImageButton btnQuit;
    ImageButton btnBack;
    ImageButton btnRestart;

    // Game data
    int currentPlayer = 1;
    bool shotInAir = false;
    std::string resultText;

    Game() {
        tank1.name = "Player 1";
        tank2.name = "Player 2";
    }

    void LoadAssets() {
        // backgrounds
        bgDay   = LoadTexture("assets/background_day.png");
        bgNight = LoadTexture("assets/background_night.png");
        bgLoaded = true;

        // tanks
        tank1.normal  = LoadTexture("assets/tank_green.png");
        tank1.damaged = LoadTexture("assets/tank_green.png");
        tank1.dead    = LoadTexture("assets/tank_green_dead.png");
        tank1.texNormalLoaded = tank1.texDamagedLoaded = tank1.texDeadLoaded = true;
        tank1.facingRight = true;

        tank2.normal  = LoadTexture("assets/tank_blue.png");
        tank2.damaged = LoadTexture("assets/tank_blue_damaged.png");
        tank2.dead    = LoadTexture("assets/tank_blue_dead.png");
        tank2.texNormalLoaded = tank2.texDamagedLoaded = tank2.texDeadLoaded = true;
        tank2.facingRight = false;

        tank1.pos = { 250.0f, GROUND_Y };
        tank2.pos = { SCREEN_WIDTH - 250.0f, GROUND_Y };

        tank1.angleDeg = 60.0f;
        tank2.angleDeg = 120.0f;

        // HUD
        hb1.Load("assets/health_bar_frame.png",
                 "assets/health_bar_full.png",
                 "assets/health_bar_empty.png",
                 Vector2{220, 60});
        hb2.Load("assets/health_bar_frame.png",
                 "assets/health_bar_full.png",
                 "assets/health_bar_empty.png",
                 Vector2{SCREEN_WIDTH - 220, 60});

        powerBar.Load("assets/power_bar.png", Vector2{SCREEN_WIDTH/2.0f, SCREEN_HEIGHT - 60.0f});

        Image aimImg = LoadImage("assets/aim_indicator.png");
        if (aimImg.data) {
            aimIndicator = LoadTextureFromImage(aimImg);
            UnloadImage(aimImg);
            aimLoaded = true;
        }

        // weapons & projectiles
        weaponSlot        = LoadTexture("assets/weapon_slot.png");
        weaponBulletIcon  = LoadTexture("assets/bullet_icon.png");
        weaponRocketIcon  = LoadTexture("assets/rocket_icon.png");
        weaponBombIcon    = LoadTexture("assets/bomb_icon.png");
        weaponsLoaded     = true;

        projBullet = LoadTexture("assets/bullet_projectile.png");
        projRocket = LoadTexture("assets/rocket_projectile.png");
        projBomb   = LoadTexture("assets/bomb_projectile.png");
        projLoaded = true;

        // FX
        explosion = LoadTexture("assets/explosion.png");
        cloud     = LoadTexture("assets/cloud.png");
        fxLoaded  = true;

        // meta UI
        vsIcon        = LoadTexture("assets/vs_icon.png");
        player1Label  = LoadTexture("assets/player1_label.png");
        victoryBanner = LoadTexture("assets/victory_banner.png");
        defeatBanner  = LoadTexture("assets/defeat_banner.png");
        chest         = LoadTexture("assets/chest.png");
        starFull      = LoadTexture("assets/star_full.png");
        starEmpty     = LoadTexture("assets/star_empty.png");
        metaLoaded    = true;

        // Buttons
        btnPlay.Load("assets/play_button.png",   Vector2{SCREEN_WIDTH/2.0f, SCREEN_HEIGHT/2.0f - 40});
        btnQuit.Load("assets/quit_button.png",   Vector2{SCREEN_WIDTH/2.0f, SCREEN_HEIGHT/2.0f + 60});
        btnBack.Load("assets/back_button.png",   Vector2{120, SCREEN_HEIGHT - 60});
        btnRestart.Load("assets/restart_button.png", Vector2{SCREEN_WIDTH/2.0f, SCREEN_HEIGHT/2.0f + 80});

        ResetMatch();
    }

    void ResetMatch() {
        tank1.health = tank1.maxHealth;
        tank2.health = tank2.maxHealth;
        tank1.pos = { 250.0f, GROUND_Y };
        tank2.pos = { SCREEN_WIDTH - 250.0f, GROUND_Y };

        tank1.angleDeg = 60.0f;
        tank2.angleDeg = 120.0f;
        tank1.power    = 350.0f;
        tank2.power    = 350.0f;

        projectile.active = false;
        shotInAir = false;
        currentPlayer = 1;
        currentWeapon = WeaponType::BULLET;
        resultText.clear();
    }

    void Run() {
        LoadAssets();
        while (!WindowShouldClose()) {
            float dt = GetFrameTime();
            Update(dt);
            BeginDrawing();
            ClearBackground(BLACK);
            Draw();
            EndDrawing();
        }
        UnloadAll();
    }

    void Update(float dt) {
        switch (screen) {
            case GameScreen::MENU:     UpdateMenu();      break;
            case GameScreen::VS_INTRO: UpdateVsIntro();   break;
            case GameScreen::PLAYING:  UpdatePlaying(dt); break;
            case GameScreen::RESULT:   UpdateResult();    break;
        }
    }

    void Draw() {
        switch (screen) {
            case GameScreen::MENU:     DrawMenu();      break;
            case GameScreen::VS_INTRO: DrawVsIntro();   break;
            case GameScreen::PLAYING:  DrawPlaying();   break;
            case GameScreen::RESULT:   DrawResult();    break;
        }
    }

private:
    // -----------------------------------------------------
    // MENU
    // -----------------------------------------------------
    void UpdateMenu() {
        // toggle day/night with key for fun
        if (IsKeyPressed(KEY_TAB)) useNight = !useNight;

        if (btnPlay.Update()) {
            screen = GameScreen::VS_INTRO;
        }
        if (btnQuit.Update()) {
            CloseWindow();
        }
    }

    void DrawBackground() const {
        const Texture2D* bg = useNight ? &bgNight : &bgDay;
        DrawTexturePro(
            *bg,
            Rectangle{0,0,(float)bg->width,(float)bg->height},
            Rectangle{0,0,(float)SCREEN_WIDTH,(float)SCREEN_HEIGHT},
            Vector2{0,0},
            0.0f,
            WHITE
        );
    }

    void DrawMenu() {
        DrawBackground();

        // title
        const char* title = "TANK DUEL";
        int fontSize = 70;
        int w = MeasureText(title, fontSize);
        DrawText(title, SCREEN_WIDTH/2 - w/2, 120, fontSize, WHITE);

        // loading/power bar as decoration
        powerBar.Draw(1.0f);

        btnPlay.Draw();
        btnQuit.Draw();

        DrawText("Press TAB to toggle background (day/night)",
                 20, SCREEN_HEIGHT - 40, 18, RAYWHITE);
    }

    // -----------------------------------------------------
    // VS INTRO
    // -----------------------------------------------------
    void UpdateVsIntro() {
        if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            screen = GameScreen::PLAYING;
        }
        if (btnBack.Update()) {
            screen = GameScreen::MENU;
        }
    }

    void DrawVsIntro() {
        DrawBackground();

        // tanks big in middle
        float scale = 0.65f;
        float w1 = tank1.normal.width * scale;
        float h1 = tank1.normal.height* scale;
        Rectangle src1{0,0,(float)tank1.normal.width,(float)tank1.normal.height};
        Rectangle dst1{ SCREEN_WIDTH/2 - 300 - w1/2, GROUND_Y - h1, w1, h1 };
        DrawTexturePro(tank1.normal, src1, dst1, Vector2{0,0}, 0.0f, WHITE);

        float w2 = tank2.normal.width * scale;
        float h2 = tank2.normal.height* scale;
        Rectangle src2{0,0,(float)tank2.normal.width,(float)tank2.normal.height};
        src2.width = -src2.width; // flip
        Rectangle dst2{ SCREEN_WIDTH/2 + 300 - w2/2, GROUND_Y - h2, w2, h2 };
        DrawTexturePro(tank2.normal, src2, dst2, Vector2{0,0}, 0.0f, WHITE);

        // VS icon in the center
        float vsScale = 0.8f;
        Rectangle vsDst{SCREEN_WIDTH/2 - vsIcon.width*vsScale/2,
                        SCREEN_HEIGHT/2 - vsIcon.height*vsScale/2,
                        vsIcon.width*vsScale,
                        vsIcon.height*vsScale};
        DrawTexturePro(vsIcon, Rectangle{0,0,(float)vsIcon.width,(float)vsIcon.height},
                       vsDst, Vector2{0,0}, 0.0f, WHITE);

        // player 1 label
        float pScale = 0.7f;
        Rectangle pDst{80, 40, player1Label.width*pScale, player1Label.height*pScale};
        DrawTexturePro(player1Label, Rectangle{0,0,(float)player1Label.width,(float)player1Label.height},
                       pDst, Vector2{0,0}, 0.0f, WHITE);

        DrawText("VS SCREEN - press SPACE or click to start",
                 SCREEN_WIDTH/2 - 260, SCREEN_HEIGHT - 80, 20, RAYWHITE);

        btnBack.Draw();
    }

    // -----------------------------------------------------
    // PLAYING
    // -----------------------------------------------------
    void UpdatePlaying(float dt) {
        Tank* active = (currentPlayer == 1) ? &tank1 : &tank2;
        Tank* enemy  = (currentPlayer == 1) ? &tank2 : &tank1;

        // weapon selection with keys 1/2/3
        if (IsKeyPressed(KEY_ONE)) currentWeapon = WeaponType::BULLET;
        if (IsKeyPressed(KEY_TWO)) currentWeapon = WeaponType::ROCKET;
        if (IsKeyPressed(KEY_THREE)) currentWeapon = WeaponType::BOMB;

        // adjust angle/power if no shot flying
        if (!projectile.active) {
            if (IsKeyDown(KEY_UP))   active->angleDeg += 60.0f * dt;
            if (IsKeyDown(KEY_DOWN)) active->angleDeg -= 60.0f * dt;
            active->angleDeg = ClampFloat(active->angleDeg, 5.0f, 175.0f);

            if (IsKeyDown(KEY_RIGHT)) active->power += 250.0f * dt;
            if (IsKeyDown(KEY_LEFT))  active->power -= 250.0f * dt;
            active->power = ClampFloat(active->power, 200.0f, 600.0f);

            if (IsKeyPressed(KEY_SPACE)) {
                // choose projectile sprite
                switch (currentWeapon) {
                    case WeaponType::BULLET:
                        projectile.sprite = projBullet;
                        projectile.spriteScale = 0.5f;
                        break;
                    case WeaponType::ROCKET:
                        projectile.sprite = projRocket;
                        projectile.spriteScale = 0.6f;
                        break;
                    case WeaponType::BOMB:
                        projectile.sprite = projBomb;
                        projectile.spriteScale = 0.7f;
                        break;
                }
                projectile.hasSprite = true;

                float baseY = active->pos.y - 60.0f;
                projectile.Launch(Vector2{active->pos.x, baseY},
                                  active->angleDeg,
                                  active->power,
                                  currentWeapon);
                shotInAir = true;
            }
        }

        projectile.Update(dt);

        // check collision with enemy
        if (projectile.active && CheckCollisionCircleRec(projectile.pos, 20.0f, enemy->Hitbox())) {
            projectile.active = false;
            shotInAir = false;

            int damage = 25;
            if (currentWeapon == WeaponType::ROCKET) damage = 35;
            if (currentWeapon == WeaponType::BOMB)   damage = 45;

            enemy->health -= damage;
            if (enemy->health < 0) enemy->health = 0;

            if (!enemy->IsAlive()) {
                screen = GameScreen::RESULT;
                resultText = (currentPlayer == 1) ? "Player 1 Wins!" : "Player 2 Wins!";
            } else {
                currentPlayer = (currentPlayer == 1) ? 2 : 1;
            }
        }

        if (!projectile.active && shotInAir) {
            // missed
            shotInAir = false;
            currentPlayer = (currentPlayer == 1) ? 2 : 1;
        }

        if (IsKeyPressed(KEY_ESCAPE)) {
            screen = GameScreen::MENU;
        }
    }

    void DrawWeaponSlot(Texture2D icon, Vector2 center, bool selected) const {
        float slotScale = 0.7f;
        Rectangle slotDst{center.x - weaponSlot.width*slotScale/2,
                          center.y - weaponSlot.height*slotScale/2,
                          weaponSlot.width*slotScale,
                          weaponSlot.height*slotScale};
        DrawTexturePro(weaponSlot,
                       Rectangle{0,0,(float)weaponSlot.width,(float)weaponSlot.height},
                       slotDst, Vector2{0,0}, 0.0f, WHITE);

        float iconScale = 0.65f;
        Rectangle iconDst{center.x - icon.width*iconScale/2,
                          center.y - icon.height*iconScale/2,
                          icon.width*iconScale,
                          icon.height*iconScale};
        DrawTexturePro(icon, Rectangle{0,0,(float)icon.width,(float)icon.height},
                       iconDst, Vector2{0,0}, 0.0f,
                       selected ? WHITE : Color{200,200,200,255});
    }

    void DrawHUD() {
        // health bars
        hb1.Draw(tank1.health, tank1.maxHealth);
        hb2.Draw(tank2.health, tank2.maxHealth);

        // power bar (based on active player's power)
        Tank* active = (currentPlayer == 1) ? &tank1 : &tank2;
        float ratio = (active->power - 200.0f) / (600.0f - 200.0f);
        powerBar.Draw(ratio);

        // aim indicator near top center
        if (aimLoaded) {
            float scale = 0.5f;
            Rectangle dst{SCREEN_WIDTH/2 - aimIndicator.width*scale/2,
                          120,
                          aimIndicator.width*scale,
                          aimIndicator.height*scale};
            DrawTexturePro(aimIndicator, Rectangle{0,0,(float)aimIndicator.width,(float)aimIndicator.height},
                           dst, Vector2{0,0}, 0.0f, WHITE);
        }

        // current player text
        const char* cp = (currentPlayer == 1) ? "PLAYER 1 TURN" : "PLAYER 2 TURN";
        int fs = 26;
        int w = MeasureText(cp, fs);
        DrawText(cp, SCREEN_WIDTH/2 - w/2, 40, fs, RAYWHITE);

        // weapon icons at bottom left
        if (weaponsLoaded) {
            Vector2 c1{130, SCREEN_HEIGHT - 70};
            Vector2 c2{260, SCREEN_HEIGHT - 70};
            Vector2 c3{390, SCREEN_HEIGHT - 70};

            DrawWeaponSlot(weaponBulletIcon, c1, currentWeapon == WeaponType::BULLET);
            DrawWeaponSlot(weaponRocketIcon, c2, currentWeapon == WeaponType::ROCKET);
            DrawWeaponSlot(weaponBombIcon,   c3, currentWeapon == WeaponType::BOMB);

            DrawText("1: Bullet   2: Rocket   3: Bomb", 40, SCREEN_HEIGHT - 115, 18, RAYWHITE);
        }
    }

    void DrawPlaying() {
        DrawBackground();

        // ground
        DrawRectangle(0, (int)GROUND_Y, SCREEN_WIDTH, SCREEN_HEIGHT - (int)GROUND_Y, DARKBROWN);

        // tanks
        tank1.Draw();
        tank2.Draw();

        // projectile
        projectile.Draw();

        // HUD
        DrawHUD();

        DrawText("Controls: UP/DOWN angle | LEFT/RIGHT power | SPACE fire | ESC menu",
                 230, 8, 16, RAYWHITE);
    }

    // -----------------------------------------------------
    // RESULT
    // -----------------------------------------------------
    void UpdateResult() {
        if (btnRestart.Update()) {
            ResetMatch();
            screen = GameScreen::PLAYING;
        }
        if (btnBack.Update()) {
            screen = GameScreen::MENU;
        }
    }

    void DrawResult() {
        DrawPlaying(); // background = last battle frame

        // dark overlay
        DrawRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT, Color{0,0,0,170});

        bool p1Won = (resultText.find("Player 1") != std::string::npos);

        // banner
        Texture2D banner = p1Won ? victoryBanner : defeatBanner;
        float scale = 0.8f;
        Rectangle dst{SCREEN_WIDTH/2 - banner.width*scale/2,
                      140,
                      banner.width*scale,
                      banner.height*scale};
        DrawTexturePro(banner, Rectangle{0,0,(float)banner.width,(float)banner.height},
                       dst, Vector2{0,0}, 0.0f, WHITE);

        // chest
        float cScale = 0.7f;
        Rectangle cDst{SCREEN_WIDTH/2 - chest.width*cScale/2,
                       dst.y + dst.height + 30,
                       chest.width*cScale,
                       chest.height*cScale};
        DrawTexturePro(chest, Rectangle{0,0,(float)chest.width,(float)chest.height},
                       cDst, Vector2{0,0}, 0.0f, WHITE);

        // stars (fake rating)
        float starScale = 0.6f;
        float starSpacing = 90.0f;
        int stars = p1Won ? 3 : 1;
        for (int i=0;i<3;i++) {
            Texture2D& sTex = (i < stars) ? starFull : starEmpty;
            float x = SCREEN_WIDTH/2 - starSpacing + i*starSpacing;
            Rectangle sDst{x - sTex.width*starScale/2,
                           cDst.y + cDst.height + 25,
                           sTex.width*starScale,
                           sTex.height*starScale};
            DrawTexturePro(sTex, Rectangle{0,0,(float)sTex.width,(float)sTex.height},
                           sDst, Vector2{0,0}, 0.0f, WHITE);
        }

        btnRestart.Draw();
        btnBack.Draw();
    }

    // -----------------------------------------------------
    void UnloadAll() {
        // In real project you should unload all textures here.
        // Left empty for brevity because program closes right after.
    }
};

// ---------------------------------------------------------
// main
// ---------------------------------------------------------
int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Tank-Style Game UI (C++ / raylib)");
    SetTargetFPS(60);

    Game game;
    game.Run();

    CloseWindow();
    return 0;
}
