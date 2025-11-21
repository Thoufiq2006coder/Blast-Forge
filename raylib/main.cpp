#include "raylib.h"
#include <vector>
#include <cmath>
#include <string>

// --------------------------------------------------
// Global constants
// --------------------------------------------------
const int   SCREEN_WIDTH  = 1280;
const int   SCREEN_HEIGHT = 720;
const float GRAVITY       = 300.0f;

enum WeaponType { WEAPON_NORMAL = 0, WEAPON_BIG = 1, WEAPON_COUNT = 2 };

// --------------------------------------------------
// TERRAIN  (destructible heightfield)
// --------------------------------------------------
class Terrain {
private:
    std::vector<float> heights;   // ground top y at each x

public:
    Terrain() { Generate(); }

    void Generate() {
        heights.resize(SCREEN_WIDTH);
        float base = 520.0f;
        float amp  = 70.0f;

        for (int x = 0; x < SCREEN_WIDTH; ++x) {
            float t = (float)x / (float)SCREEN_WIDTH;
            float h = base
                    + std::sinf(t * PI * 2.0f) * amp * 0.6f
                    + std::sinf(t * PI * 4.0f) * amp * 0.3f;
            if (h > SCREEN_HEIGHT - 40) h = SCREEN_HEIGHT - 40;
            heights[x] = h;
        }
    }

    float GetHeightAt(int x) const {
        if (x < 0) x = 0;
        if (x >= SCREEN_WIDTH) x = SCREEN_WIDTH - 1;
        return heights[x];
    }

    bool Collides(Vector2 pos) const {
        int xi = (int)pos.x;
        if (xi < 0 || xi >= SCREEN_WIDTH) return false;
        return pos.y >= heights[xi];
    }

    // Carve a crater
    void ApplyExplosion(Vector2 center, float radius) {
        int minX = (int)(center.x - radius);
        int maxX = (int)(center.x + radius);
        if (minX < 0) minX = 0;
        if (maxX >= SCREEN_WIDTH) maxX = SCREEN_WIDTH - 1;

        for (int x = minX; x <= maxX; ++x) {
            float dx   = (float)x - center.x;
            float dist = std::fabs(dx);
            if (dist < radius) {
                float depth = std::sqrt(radius * radius - dist * dist);
                heights[x] += depth * 0.8f;          // push ground downward
                if (heights[x] > SCREEN_HEIGHT) heights[x] = SCREEN_HEIGHT;
            }
        }
    }

    void Draw() const {
        Color groundColor = { 139, 69, 19, 255 };
        for (int x = 0; x < SCREEN_WIDTH; ++x) {
            DrawLine(x, (int)heights[x], x, SCREEN_HEIGHT, groundColor);
        }
    }
};

// --------------------------------------------------
// TANK (flat/cartoon, blue/red)
// --------------------------------------------------
class Tank {
private:
    Vector2 pos;     // bottom-center on terrain
    float   angle;   // 0..180 degrees
    float   power;   // shot power
    int     health;
    Color   color;

public:
    Tank(Vector2 startPos, Color c)
        : pos(startPos), angle(45.0f), power(350.0f),
          health(100), color(c) {}

    void UpdateOnTerrain(const Terrain& terrain) {
        int xi = (int)pos.x;
        pos.y = terrain.GetHeightAt(xi);
    }

    void Move(float dir, float dt, const Terrain& terrain) {
        pos.x += dir * 200.0f * dt;
        if (pos.x < 30.0f) pos.x = 30.0f;
        if (pos.x > SCREEN_WIDTH - 30.0f) pos.x = SCREEN_WIDTH - 30.0f;
        UpdateOnTerrain(terrain);
    }

    void ChangeAngle(float delta) {
        angle += delta;
        if (angle < 5.0f) angle = 5.0f;
        if (angle > 175.0f) angle = 175.0f;
    }

    void ChangePower(float delta) {
        power += delta;
        if (power < 150.0f) power = 150.0f;
        if (power > 700.0f) power = 700.0f;
    }

    void ApplyDamage(int dmg) {
        health -= dmg;
        if (health < 0) health = 0;
    }

    bool IsDead() const { return health <= 0; }

    // simple cartoon tank drawing
    void Draw() const {
        float bodyW = 70.0f;
        float bodyH = 24.0f;
        float treadH = 16.0f;

        float x = pos.x;
        float y = pos.y;

        Color treadColor = { 20, 40, 70, 255 };
        Color bodyColor  = color;
        Color turretColor{ (unsigned char)(color.r + 10),
                           (unsigned char)(color.g + 10),
                           (unsigned char)(color.b + 10), 255 };

        // treads
        DrawRectangleRounded({ x - bodyW/2.0f, y - treadH, bodyW, treadH },
                             0.4f, 8, treadColor);

        // body
        DrawRectangleRounded({ x - bodyW/2.0f + 5, y - treadH - bodyH,
                               bodyW - 10, bodyH },
                             0.5f, 8, bodyColor);

        // turret
        DrawRectangleRounded({ x - 25, y - treadH - bodyH - 20,
                               50, 20 },
                             0.6f, 8, turretColor);

        // barrel line
        float rad = angle * PI / 180.0f;
        Vector2 barrelStart = { x + std::cos(rad) * 15.0f,
                                y - treadH - bodyH - 10.0f - std::sin(rad) * 15.0f };
        Vector2 barrelEnd   = { barrelStart.x + std::cos(rad) * 50.0f,
                                barrelStart.y - std::sin(rad) * 50.0f };

        DrawLineEx(barrelStart, barrelEnd, 8.0f, turretColor);

        // little muzzle circle
        DrawCircleV(barrelEnd, 5.0f, turretColor);
    }

    Vector2 GetMuzzle() const {
        float bodyH  = 24.0f;
        float treadH = 16.0f;
        float rad = angle * PI / 180.0f;

        Vector2 barrelStart = { pos.x + std::cos(rad) * 15.0f,
                                pos.y - treadH - bodyH - 10.0f - std::sin(rad) * 15.0f };
        Vector2 barrelEnd   = { barrelStart.x + std::cos(rad) * 50.0f,
                                barrelStart.y - std::sin(rad) * 50.0f };
        return barrelEnd;
    }

    Rectangle GetHitBox() const {
        float bodyW = 70.0f;
        float bodyH = 24.0f;
        float treadH = 16.0f;
        return { pos.x - bodyW/2.0f, pos.y - treadH - bodyH,
                 bodyW, bodyH + treadH };
    }

    bool IsHit(Vector2 explosionCenter, float radius) const {
        return CheckCollisionCircleRec(explosionCenter, radius, GetHitBox());
    }

    // getters
    float   GetAngle() const  { return angle; }
    float   GetPower() const  { return power; }
    int     GetHealth() const { return health; }
    Vector2 GetPos() const    { return pos; }
};

// --------------------------------------------------
// PROJECTILE
// --------------------------------------------------
class Projectile {
private:
    Vector2 pos{};
    Vector2 vel{};
    bool    active{false};

public:
    void Launch(Vector2 start, float angleDeg, float power) {
        pos = start;
        float rad = angleDeg * PI / 180.0f;
        vel.x = std::cos(rad) * power;
        vel.y = -std::sin(rad) * power;
        active = true;
    }

    void Update(float dt) {
        if (!active) return;
        vel.y += GRAVITY * dt;
        pos.x += vel.x * dt;
        pos.y += vel.y * dt;

        if (pos.x < -50 || pos.x > SCREEN_WIDTH + 50 ||
            pos.y > SCREEN_HEIGHT + 50) {
            active = false;
        }
    }

    void Draw() const {
        if (!active) return;
        DrawCircleV(pos, 5.0f, YELLOW);
    }

    bool   IsActive() const { return active; }
    void   Deactivate()     { active = false; }
    Vector2 GetPosition() const { return pos; }
};

// --------------------------------------------------
// EXPLOSION (simple expanding circle)
// --------------------------------------------------
class Explosion {
private:
    Vector2 center{};
    float   radius{0.0f};
    float   maxRadius{60.0f};
    float   timer{0.0f};
    float   duration{0.4f};
    bool    active{false};

public:
    void Trigger(Vector2 c, float maxR) {
        center = c;
        maxRadius = maxR;
        radius = 0.0f;
        timer = 0.0f;
        duration = 0.4f;
        active = true;
    }

    void Update(float dt) {
        if (!active) return;
        timer += dt;
        float t = timer / duration;
        if (t >= 1.0f) {
            active = false;
            return;
        }
        radius = maxRadius * t;
    }

    void Draw() const {
        if (!active) return;
        Color inner = { 255, 220, 80, 180 };
        Color outer = { 255, 140, 0, 140 };
        DrawCircleV(center, radius, inner);
        DrawCircleLines((int)center.x, (int)center.y, (int)radius, outer);
    }

    bool   IsActive() const { return active; }
    Vector2 GetCenter() const { return center; }
    float  GetRadius() const  { return radius; }
};

// --------------------------------------------------
// GAME
// --------------------------------------------------
class Game {
private:
    Terrain    terrain;
    Tank       tank1;
    Tank       tank2;
    Projectile projectile;
    Explosion  explosion;

    int        currentPlayer;
    WeaponType currentWeapon[3];  // index 1 -> P1, 2 -> P2
    bool       gameOver;

public:
    Game()
        : terrain(),
          tank1({ 200.0f, 0.0f }, Color{ 80, 170, 255, 255 }),       // blue
          tank2({ SCREEN_WIDTH - 200.0f, 0.0f }, Color{ 255, 120, 120, 255 }), // red
          currentPlayer(1),
          gameOver(false)
    {
        tank1.UpdateOnTerrain(terrain);
        tank2.UpdateOnTerrain(terrain);
        currentWeapon[1] = WEAPON_NORMAL;
        currentWeapon[2] = WEAPON_NORMAL;
    }

    void Reset() {
        terrain.Generate();
        tank1 = Tank({ 200.0f, 0.0f }, Color{ 80, 170, 255, 255 });
        tank2 = Tank({ SCREEN_WIDTH - 200.0f, 0.0f }, Color{ 255, 120, 120, 255 });
        tank1.UpdateOnTerrain(terrain);
        tank2.UpdateOnTerrain(terrain);
        projectile = Projectile();
        explosion  = Explosion();
        currentPlayer = 1;
        currentWeapon[1] = WEAPON_NORMAL;
        currentWeapon[2] = WEAPON_NORMAL;
        gameOver = false;
    }

    void Update(float dt) {
        if (gameOver) {
            if (IsKeyPressed(KEY_R)) Reset();
            if (explosion.IsActive()) explosion.Update(dt);
            return;
        }

        if (explosion.IsActive()) {
            explosion.Update(dt);
            if (!explosion.IsActive()) {
                // after explosion finishes, next turn
                SwitchTurn();
            }
            return;
        }

        if (projectile.IsActive()) {
            projectile.Update(dt);
            HandleProjectileCollision();
        } else {
            HandlePlayerInput(dt);
        }
    }

    void Draw() const {
        ClearBackground(Color{ 10, 10, 30, 255 });

        terrain.Draw();
        tank1.Draw();
        tank2.Draw();
        projectile.Draw();
        explosion.Draw();

        DrawUI();
    }

private:
    void HandlePlayerInput(float dt) {
        Tank& t = (currentPlayer == 1 ? tank1 : tank2);

        int leftKey   = (currentPlayer == 1 ? KEY_A      : KEY_LEFT);
        int rightKey  = (currentPlayer == 1 ? KEY_D      : KEY_RIGHT);
        int upKey     = (currentPlayer == 1 ? KEY_W      : KEY_UP);
        int downKey   = (currentPlayer == 1 ? KEY_S      : KEY_DOWN);
        int powerUp   = (currentPlayer == 1 ? KEY_E      : KEY_K);
        int powerDown = (currentPlayer == 1 ? KEY_Q      : KEY_L);
        int fireKey   = (currentPlayer == 1 ? KEY_SPACE  : KEY_ENTER);
        int switchKey = (currentPlayer == 1 ? KEY_F      : KEY_RIGHT_SHIFT);

        if (IsKeyDown(leftKey))  t.Move(-1.0f, dt, terrain);
        if (IsKeyDown(rightKey)) t.Move( 1.0f, dt, terrain);

        if (IsKeyDown(upKey))    t.ChangeAngle( 80.0f * dt);
        if (IsKeyDown(downKey))  t.ChangeAngle(-80.0f * dt);

        if (IsKeyDown(powerUp))   t.ChangePower( 250.0f * dt);
        if (IsKeyDown(powerDown)) t.ChangePower(-250.0f * dt);

        if (IsKeyPressed(switchKey)) {
            int idx = (int)currentWeapon[currentPlayer];
            idx = (idx + 1) % WEAPON_COUNT;
            currentWeapon[currentPlayer] = (WeaponType)idx;
        }

        if (IsKeyPressed(fireKey)) {
            Fire(t, currentWeapon[currentPlayer]);
        }
    }

    void Fire(const Tank& t, WeaponType w) {
        float angle = t.GetAngle();
        float power = t.GetPower() * 0.7f;   // tuning
        projectile.Launch(t.GetMuzzle(), angle, power);
    }

    void GetWeaponStats(WeaponType w, float& radius, int& damage) const {
        switch (w) {
            case WEAPON_NORMAL:
                radius = 45.0f;
                damage = 30;
                break;
            case WEAPON_BIG:
                radius = 75.0f;
                damage = 55;
                break;
            default:
                radius = 45.0f;
                damage = 30;
                break;
        }
    }

    void HandleProjectileCollision() {
        if (!projectile.IsActive()) return;

        Vector2 p = projectile.GetPosition();

        if (p.x < 0 || p.x >= SCREEN_WIDTH) {
            projectile.Deactivate();
            SwitchTurn();
            return;
        }

        if (terrain.Collides(p)) {
            float radius;
            int   damage;
            GetWeaponStats(currentWeapon[currentPlayer], radius, damage);

            explosion.Trigger(p, radius);
            terrain.ApplyExplosion(p, radius);

            if (tank1.IsHit(p, radius)) tank1.ApplyDamage(damage);
            if (tank2.IsHit(p, radius)) tank2.ApplyDamage(damage);

            projectile.Deactivate();
            CheckGameOver();
        }
    }

    void SwitchTurn() {
        if (gameOver) return;
        currentPlayer = (currentPlayer == 1 ? 2 : 1);
    }

    void CheckGameOver() {
        if (tank1.IsDead() && tank2.IsDead()) gameOver = true;
        else if (tank1.IsDead() || tank2.IsDead()) gameOver = true;
    }

    static void DrawTankStatus(const Tank& t, int x, int y, const char* label) {
        DrawText(label, x, y, 20, WHITE);

        int hp = t.GetHealth();
        float barW = 200.0f;
        float barH = 14.0f;

        DrawRectangle(x, y + 26, (int)barW, (int)barH, Color{ 50, 50, 70, 255 });
        DrawRectangle(x, y + 26, (int)(barW * hp / 100.0f), (int)barH,
                      Color{ 0, 220, 90, 255 });

        std::string hpText = std::to_string(hp) + " HP";
        DrawText(hpText.c_str(), x + (int)barW + 10, y + 22, 18, WHITE);
    }

    void DrawUI() const {
        std::string turnText = "Turn: Player " + std::to_string(currentPlayer);
        DrawText(turnText.c_str(), 20, 20, 24, RAYWHITE);

        DrawTankStatus(tank1, 20, 60, "P1");
        DrawTankStatus(tank2, SCREEN_WIDTH - 260, 60, "P2");

        const char* weaponNames[WEAPON_COUNT] = { "Normal", "Big" };
        std::string w1 = "P1 Weapon: " + std::string(weaponNames[currentWeapon[1]]);
        std::string w2 = "P2 Weapon: " + std::string(weaponNames[currentWeapon[2]]);
        DrawText(w1.c_str(), 20, 100, 20, SKYBLUE);
        DrawText(w2.c_str(), SCREEN_WIDTH - 260, 100, 20, PINK);

        if (gameOver) {
            const char* msg;
            if (tank1.IsDead() && tank2.IsDead()) msg = "Draw!";
            else if (tank1.IsDead()) msg = "Player 2 Wins!";
            else msg = "Player 1 Wins!";

            int fs = 48;
            int tw = MeasureText(msg, fs);
            DrawText(msg, SCREEN_WIDTH / 2 - tw / 2, SCREEN_HEIGHT / 2 - 50, fs, YELLOW);

            const char* sub = "Press R to Restart";
            int ts = 24;
            tw = MeasureText(sub, ts);
            DrawText(sub, SCREEN_WIDTH / 2 - tw / 2, SCREEN_HEIGHT / 2 + 10, ts, RAYWHITE);
        }

        const char* help =
            "P1: A/D move, W/S angle, Q/E power, F switch weapon, SPACE fire   |   "
            "P2: <-/-> move, Up/Down angle, L/K power, RightShift switch weapon, ENTER fire   |   R: restart";
        int fs = 16;
        int tw = MeasureText(help, fs);
        DrawText(help, SCREEN_WIDTH / 2 - tw / 2, SCREEN_HEIGHT - 30, fs, GRAY);
    }
};

// --------------------------------------------------
// main
// --------------------------------------------------
int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Tank Stars - C++ + raylib");
    SetTargetFPS(60);

    Game game;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        game.Update(dt);

        BeginDrawing();
        game.Draw();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
