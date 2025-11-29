#pragma once
#include "raylib.h"
#include "Types.h"

class Projectile {
private:
    Vector2 pos;
    Vector2 vel;
    bool    active;
    int     owner;  
    WeaponType type;

public:
    Projectile();

    void Fire(Vector2 start, Vector2 velocity, WeaponType t, int player);
    void Update(float dt);
    void Draw() const;

    bool Active() const;
    void Deactivate();

    int GetOwner() const;
    Rectangle GetRect() const;
};