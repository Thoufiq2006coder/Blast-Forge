#include "Tank.h"
#include <cmath>
#include "Theme.h"

using namespace std;

Tank::Tank() 
    : barrelAngle(45.0f), health(100.0f), destroyed(false) 
{
    pos = {0, 0};
}

void Tank::Init(Vector2 p) {
    pos = p;
    barrelAngle = 45.0f;
    health = 100.0f;
    destroyed = false;
}

void Tank::Update(bool active, int left, int right, int up, int down, float dt) {
    if (!active || destroyed) return;

    float speed = 200.0f;
    float rot = 60.0f;

    if (IsKeyDown(left))  pos.x -= speed * dt;
    if (IsKeyDown(right)) pos.x += speed * dt;
    if (IsKeyDown(up))    barrelAngle += rot * dt;
    if (IsKeyDown(down))  barrelAngle -= rot * dt;

    if (barrelAngle < 5.0f) barrelAngle = 5.0f;
    if (barrelAngle > 175.0f) barrelAngle = 175.0f;
}

Rectangle Tank::GetBody() const {
    return { pos.x - 40.0f, pos.y - 30.0f, 80.0f, 30.0f };
}

void Tank::Draw(Color mainColor) const {
    Rectangle b = GetBody();

    if (destroyed) {
        DrawRectangleRec(b, DARKGRAY);
        DrawText("X", (int)(b.x + 30), (int)(b.y - 20), 30, RED);
        return;
    }

    // Body
    DrawRectangleRounded(b, 0.4f, 10, mainColor);
    
    // FIX: Removed the thickness (2) parameter here to fix "Int to Color"
    DrawRectangleRoundedLines(b, 0.4f, 10, BLACK);

    // Tracks
    DrawRectangle((int)b.x + 5, (int)(b.y + b.height - 5), (int)b.width - 10, 10, BLACK);

    // Barrel
    Vector2 base = { b.x + b.width/2, b.y };
    float   ang  = GetBarrelAngleRad();
    Vector2 tip  = { base.x + cosf(ang)*50.0f, base.y - sinf(ang)*50.0f };

    DrawLineEx(base, tip, 8, BLACK);
    DrawLineEx(base, tip, 4, mainColor);
}

Vector2 Tank::GetBarrelTip() const {
    Rectangle b = GetBody();
    float ang = GetBarrelAngleRad();
    return { b.x + b.width/2 + cosf(ang)*50.0f,
             b.y - sinf(ang)*50.0f };
}

float Tank::GetBarrelAngleRad() const {
    return barrelAngle * 3.14159265f / 180.0f;
}

void Tank::TakeDamage(float dmg) {
    if (destroyed) return;
    health -= dmg;
    if (health <= 0) { health = 0; destroyed = true; }
}

bool Tank::IsDead() const { return destroyed; }
float Tank::GetHealth() const { return health; }