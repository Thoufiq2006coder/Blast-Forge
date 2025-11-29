#include "Projectile.h"
#include <cmath>
#include "Theme.h"

using namespace std;

Projectile::Projectile()
    : active(false), owner(0), type(WeaponType::Rocket) 
{
    pos = {0,0};
    vel = {0,0};
}

void Projectile::Fire(Vector2 start, Vector2 velocity, WeaponType t, int player) {
    pos    = start;
    vel    = velocity;
    type   = t;
    owner  = player;
    active = true;
}

void Projectile::Update(float dt) {
    if (!active) return;

    const float GRAVITY = 300.0f;

    vel.y += GRAVITY * dt;
    pos.x += vel.x * dt;
    pos.y += vel.y * dt;

    if (pos.x < -50 || pos.x > 2000 || pos.y > 2000)
        active = false;
}

void Projectile::Draw() const {
    if (!active) return;

    float angle = atan2f(-vel.y, vel.x);

    Vector2 tip  = pos;
    Vector2 rear = { pos.x - cosf(angle)*25.0f,
                     pos.y + sinf(angle)*25.0f };

    DrawLineEx(rear, tip, 4, GRAY); // Trail

    // Warhead
    Vector2 p1 = tip;
    Vector2 p2 = { tip.x - cosf(angle)*10 + sinf(angle)*5,
                   tip.y + sinf(angle)*10 + cosf(angle)*5 };
    Vector2 p3 = { tip.x - cosf(angle)*10 - sinf(angle)*5,
                   tip.y + sinf(angle)*10 - cosf(angle)*5 };

    DrawTriangle(p1, p2, p3, Theme::Projectile);
}

bool Projectile::Active() const { return active; }
void Projectile::Deactivate()   { active = false; }
int  Projectile::GetOwner() const { return owner; }

Rectangle Projectile::GetRect() const {
    return { pos.x - 8, pos.y - 8, 16, 16 };
}