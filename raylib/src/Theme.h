#pragma once
#include "raylib.h"

namespace Theme {
    // --- PALETTE ---
    
    // Backgrounds
    static const Color MenuBG      = { 30, 35, 50, 255 };       // Slate Navy
    static const Color DesertSky   = { 255, 230, 180, 255 };    // Peach
    static const Color MoonSky     = { 20, 25, 40, 255 };       // Deep Dark Blue
    static const Color Ground      = { 100, 70, 50, 255 };      // Brown (Game)
    
    // UI Colors
    static const Color Text        = WHITE;
    static const Color Shadow      = { 0, 0, 0, 150 };          // Transparent Black
    static const Color Accent      = { 255, 197, 15, 255 };      // Gold
    
    // Buttons
    static const Color ButtonNormal = { 60, 70, 90, 255 };      // Blue-Grey
    static const Color ButtonHover  = { 80, 90, 110, 255 };     // Light Blue-Grey
    static const Color ButtonActive = { 0, 180, 120, 255 };     // Teal
    static const Color ButtonBorder = { 20, 20, 30, 255 };      // Dark

    // --- PLAYER COLORS (UPDATED) ---
    
    // DESERT THEME 
    // P1: Deep Blue #1A3D64
    static const Color P1_Color_Desert = { 76, 118, 59, 255 };    
    // P2: Red #CF4B00
    static const Color P2_Color_Desert = { 207, 75, 0, 255 };       

    // MOON THEME
    // P1: Orange/Amber #E2852E
    static const Color P1_Color_Moon   = { 226, 133, 46, 255 };   
    // P2: Bright Green #4CB648
    static const Color P2_Color_Moon   = { 76, 182, 72, 255 };    

    static const Color Projectile  = { 255, 140, 0, 255 };      // Orange

    // HUD
    static const Color BarBG       = { 0, 0, 0, 150 };
    static const Color BarBorder   = WHITE;
}