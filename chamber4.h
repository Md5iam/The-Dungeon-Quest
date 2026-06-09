#ifndef CHAMBER4_H
#define CHAMBER4_H

#include "chamber1.h"

extern bool ch4Solved;
extern bool hasCh4Key;
extern bool isCh4FallingInTrap;
extern float ch4TrapFallY;
extern float ch4TrapFade;
extern float ch4RumbleTimer;

// Mirror angles for Chamber 4 (Mirror A and Mirror B)
extern float ch4MirrorAngles[2]; 
// Active light state
extern bool ch4Light0On;
extern bool ch4Light1On;

// Continuous fan rotation angle
extern float fanRotation;

void drawChamber4();
void updateChamber4();
void drawChamber4HUD();
void setupChamber4Lighting();
void resetChamber4();
void handleChamber4Interaction();

#endif
