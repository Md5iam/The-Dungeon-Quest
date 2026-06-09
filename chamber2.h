#ifndef CHAMBER2_H
#define CHAMBER2_H

#include "chamber1.h"

// --- Chamber 2 State Variables ---
extern float ringAngle1;        // Outer
extern float ringAngle2;        // Middle
extern float ringAngle3;        // Inner
extern float targetRingAngle1;  // Target Outer
extern float targetRingAngle2;  // Target Middle
extern float targetRingAngle3;  // Target Inner

extern bool ch2Solved;
extern bool hasCh2Key;
extern float ch2PedestalOpenProgress;
extern bool isCh2FallingInTrap;
extern float ch2TrapFallY;
extern float ch2TrapFade;
extern float ch2BlackoutTimer;
extern float ch2RumbleTimer;

// --- Chamber 2 Functions ---
void drawChamber2();
void updateChamber2();
void drawChamber2HUD();
void setupChamber2Lighting();
void resetChamber2();
void handleChamber2Interaction();

#endif
