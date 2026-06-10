#include "outside.h"
#include <cstdlib>
#include <iostream>
#include <cmath>

// --- Exit Scenario Lighting Configuration ---
void setupOutsideScenarioLighting() {
    // Disable all lighting and clear to a pitch-black background
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHT1);
    glDisable(GL_LIGHT2);
    glDisable(GL_LIGHT3);
}

// --- Exit Scenario 3D Rendering ---
void drawOutsideScenario() {
    // Pitch-black scene representing the dark reality after escaping the dungeon
}

// --- Exit Scenario Update ---
void updateOutsideScenario() {
    if (fadeAlpha > 0.0f) {
        fadeAlpha -= 0.04f;
        if (fadeAlpha < 0.0f) fadeAlpha = 0.0f;
    }
}

// --- Exit Scenario HUD / Ending Screen ---
void drawOutsideScenarioHUD() {
    // 1. Semi-transparent dark overlay to ensure clean rendering
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.95f);
    glBegin(GL_QUADS);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(WINDOW_WIDTH, 0.0f);
    glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
    glVertex2f(0.0f, WINDOW_HEIGHT);
    glEnd();
    glDisable(GL_BLEND);

    // 2. The main warning message in bright red (using Times Roman 24 for maximum size)
    glColor3f(1.0f, 0.2f, 0.2f);
    renderBitmapString(WINDOW_WIDTH / 2.0f - 340.0f, WINDOW_HEIGHT / 2.0f + 20.0f, GLUT_BITMAP_TIMES_ROMAN_24, "AI is replacing software engineers, go back to the dungeon!!!");

    // 3. Return prompt in gray (resets the entire quest from Chamber 1)
    glColor3f(0.6f, 0.6f, 0.6f);
    renderBitmapString(WINDOW_WIDTH / 2.0f - 110.0f, WINDOW_HEIGHT / 2.0f - 40.0f, GLUT_BITMAP_HELVETICA_12, "Press [R] to return to the dungeon...");
}

// --- Exit Scenario Interaction ---
void handleOutsideScenarioInteraction() {
    // No interactions
}
