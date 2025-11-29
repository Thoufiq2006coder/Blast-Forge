#pragma once
#include "raylib.h"

class Tank {
private:
    Vector2 pos;
    float   barrelAngle;
    float   health;
    bool    destroyed;

public:
    Tank();
    void Init(Vector2 p);
    void Update(bool active, int left, int right, int up, int down, float dt);
    
    Rectangle GetBody() const;
    void Draw(Color mainColor) const;
    
    Vector2 GetBarrelTip() const;
    float   GetBarrelAngleRad() const;
    
    void TakeDamage(float dmg);
    
    bool  IsDead() const;
    float GetHealth() const;
};