#ifndef CHAMBER1_H
#define CHAMBER1_H

#include <GL/glut.h>
#include <vector>
#include <string>
#include <cmath>

// --- Shared Constants ---
const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 768;
const float PI = 3.14159265f;

// --- GameState enum ---
enum GameState {
    STATE_OUTSIDE,
    STATE_ENTERING,
    STATE_CHAMBER_1,
    STATE_CHAMBER_2,
    STATE_CHAMBER_3,
    STATE_CHAMBER_4,
    STATE_VICTORY,
    STATE_OUTSIDE_SCENARIO
};

// --- Shared variables from dungeon_game.cpp ---
extern GameState currentGameState;
extern GLuint texStone, texWood, texWolf, texLion, texSnake, texEagle, texBox, texDragon, texGround;
extern bool hasStoneTex, hasWoodTex, hasWolfTex, hasLionTex, hasSnakeTex, hasEagleTex, hasBoxTex, hasDragonTex, hasGroundTex;
extern float camX, camY, camZ, rotX, rotY;
extern float fadeAlpha;
extern float zoomFov;
extern bool keyStates[256];
extern bool specialKeyStates[256];

// --- Shared functions from dungeon_game.cpp ---
void drawPrism(float dx, float dy, float dz, GLuint texID, bool hasTex,
               float r, float g, float b, float texScaleS = 1.0f, float texScaleT = 1.0f);
void renderBitmapString(float x, float y, void *font, const char *string);
void drawDoorText(const char* text);

// --- Chamber 1 variables (defined in chamber1.cpp) ---
extern bool platePressed[4];
extern std::vector<int> plateOrder;
extern bool ch1Solved;
extern bool hasKey;
extern float pedestalOpenProgress;
extern float ch1BlackoutTimer;
extern float ch1RumbleTimer;
extern bool isFallingInTrap;
extern float trapFallY;
extern float trapFade;
extern float statueBoxRot;

// --- Chamber 1 functions ---
void drawChamber1();
void updateChamber1();
void drawChamber1HUD();
void setupChamber1Lighting();
void resetChamber1();
void handleChamber1Interaction();

#endif
