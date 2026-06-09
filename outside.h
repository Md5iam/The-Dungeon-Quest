#ifndef OUTSIDE_H
#define OUTSIDE_H

#include "chamber1.h"

extern bool isOutsideNight;
extern float cameraOrbitAngle;
extern float sunAngle;
extern bool cinematicMode;
extern float fanRotation;

void drawOutsideScenario();
void updateOutsideScenario();
void drawOutsideScenarioHUD();
void handleOutsideScenarioInteraction();
void setupOutsideScenarioLighting();

#endif
