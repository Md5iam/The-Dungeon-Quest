#ifndef CHAMBER3_H
#define CHAMBER3_H

#include "chamber1.h"

// --- Chamber 3 State Variables ---
extern bool roomLight;
extern bool ch3Solved;
extern bool hasCh3Key;
extern float ch3PedestalOpenProgress;
extern bool isCh3FallingInTrap;
extern float ch3TrapFallY;
extern float ch3TrapFade;
extern float ch3BlackoutTimer;
extern float ch3RumbleTimer;
extern bool showCh3SolutionGuide;

// Target viewpoint constants for HUD/checks
extern const float targetCx;
extern const float targetCy;
extern const float targetCz;
extern const float targetRx;
extern const float targetRy;

// --- Chamber 3 Functions ---
void drawChamber3();
void updateChamber3();
void drawChamber3HUD();
void setupChamber3Lighting();
void resetChamber3();
void handleChamber3Interaction();

#endif
