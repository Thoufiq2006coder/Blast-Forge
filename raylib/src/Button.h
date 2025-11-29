#pragma once
#include "raylib.h"
#include <string>

class Button {
private:
    Rectangle   bounds;
    std::string text;
    char        hotChar;
    KeyboardKey hotKey;
    bool        active;

public:
    Button();
    Button(Rectangle r, const std::string& t, char h, KeyboardKey k);

    bool WasClicked() const;
    void Draw() const;

    void SetActive(bool v);
    bool IsActive() const;
};