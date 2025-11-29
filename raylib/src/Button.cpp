#include "Button.h"
#include "Theme.h"

using namespace std;

// Constructor 1: Default
Button::Button() 
    : text(""), hotChar(' '), hotKey(KEY_NULL), active(false) 
{
    bounds = {0, 0, 0, 0};
}

// Constructor 2: Parameterized
Button::Button(Rectangle r, const string& t, char h, KeyboardKey k)
    : text(t), hotChar(h), hotKey(k), active(false) 
{
    bounds = r;
}

bool Button::WasClicked() const {
    Vector2 m = GetMousePosition();
    bool clicked = false;

    if (CheckCollisionPointRec(m, bounds) && IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        clicked = true;

    if (hotKey != KEY_NULL && IsKeyPressed(hotKey))
        clicked = true;

    return clicked;
}

void Button::Draw() const {
    Vector2 m = GetMousePosition();
    bool isHover = CheckCollisionPointRec(m, bounds);

    Color bg = Theme::ButtonNormal;
    Color border = Theme::ButtonBorder;
    
    // Logic for hover/active colors
    if (active) {
        bg = Theme::ButtonActive;
    } else if (isHover) {
        bg = Theme::ButtonHover;
        border = Theme::Accent; 
    }

    // Shadow
    DrawRectangleRounded({bounds.x + 4, bounds.y + 4, bounds.width, bounds.height}, 0.3f, 10, Theme::Shadow);

    // Button Base
    DrawRectangleRounded(bounds, 0.3f, 10, bg);
    
    // FIX: Removed the thickness parameter here to fix "Float to Color" error
    DrawRectangleRoundedLines(bounds, 0.3f, 10, border);

    // Text
    int font = 22;
    int tw   = MeasureText(text.c_str(), font);
    float tx = bounds.x + bounds.width / 2.0f - tw / 2.0f;
    float ty = bounds.y + bounds.height / 2.0f - font / 2.0f;

    DrawText(text.c_str(), (int)tx, (int)ty, font, Theme::Text);

    // Hotkey Hint
    if (hotChar != ' ') {
        char c[2] = { hotChar, '\0' };
        DrawText(c, (int)(bounds.x + 10), (int)(bounds.y + 5), 10, Theme::Accent);
    }
}

void Button::SetActive(bool v) { active = v; }
bool Button::IsActive() const { return active; }