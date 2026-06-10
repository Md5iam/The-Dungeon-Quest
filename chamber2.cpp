#include "chamber2.h"
#include "chamber3.h"
#include <cstdlib>
#include <iostream>

// --- Chamber 2 State Variables ---
float ringAngle1 = 0.0f;
float ringAngle2 = 0.0f;
float ringAngle3 = 0.0f;
float targetRingAngle1 = 0.0f;
float targetRingAngle2 = 0.0f;
float targetRingAngle3 = 0.0f;

bool ch2Solved = false;
bool hasCh2Key = false;
float ch2PedestalOpenProgress = 0.0f;
bool isCh2FallingInTrap = false;
float ch2TrapFallY = 0.0f;
float ch2TrapFade = 0.0f;
float ch2BlackoutTimer = 0.0f;
float ch2RumbleTimer = 0.0f;

// --- Positions ---
static const float WHEEL_POS[3][2] = {
    { -3.0f, -1.5f }, // Wheel A
    {  0.0f,  3.0f }, // Wheel B
    {  3.0f, -1.5f }  // Wheel C
};

// --- Reset Chamber 2 ---
void resetChamber2() {
    ch2Solved = false;
    hasCh2Key = false;
    ch2PedestalOpenProgress = 0.0f;
    isCh2FallingInTrap = false;
    ch2TrapFallY = 0.0f;
    ch2TrapFade = 0.0f;
    ch2BlackoutTimer = 0.0f;
    ch2RumbleTimer = 0.0f;

    // Randomize initial target angles (multiples of 90, but not aligned)
    targetRingAngle1 = (rand() % 3 + 1) * 90.0f;
    targetRingAngle2 = (rand() % 3 + 1) * 90.0f;
    targetRingAngle3 = (rand() % 3 + 1) * 90.0f;

    ringAngle1 = targetRingAngle1;
    ringAngle2 = targetRingAngle2;
    ringAngle3 = targetRingAngle3;
}

// --- Draw Symbols ---
void drawSunSymbol3D() {
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 0.8f, 0.0f);
    glutSolidSphere(0.12f, 10, 10);
    glEnable(GL_LIGHTING);
}

void drawMoonSymbol3D() {
    glDisable(GL_LIGHTING);
    glColor3f(0.85f, 0.85f, 0.9f);
    glutSolidTorus(0.04f, 0.08f, 8, 16);
    glEnable(GL_LIGHTING);
}

void drawStarSymbol3D() {
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 0.5f);
    glPushMatrix();
    glRotatef(90, 1, 0, 0);
    glutSolidCone(0.1f, 0.15f, 6, 2);
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

void drawCloudSymbol3D() {
    glDisable(GL_LIGHTING);
    glColor3f(0.95f, 0.95f, 0.95f);
    glPushMatrix();
    glutSolidSphere(0.07f, 8, 8);
    glTranslatef(0.08f, 0.0f, 0.0f);
    glutSolidSphere(0.06f, 8, 8);
    glTranslatef(-0.16f, 0.0f, 0.0f);
    glutSolidSphere(0.05f, 8, 8);
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

void drawEagleSymbol3D() {
    glDisable(GL_LIGHTING);
    glColor3f(0.1f, 0.4f, 1.0f);
    glPushMatrix();
    glRotatef(90, 1, 0, 0);
    glutSolidCone(0.09f, 0.18f, 8, 8);
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

void drawWolfSymbol3D() {
    glDisable(GL_LIGHTING);
    glColor3f(0.6f, 0.6f, 0.65f);
    glutSolidSphere(0.1f, 8, 8);
    glEnable(GL_LIGHTING);
}

void drawLionSymbol3D() {
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 0.75f, 0.0f);
    glPushMatrix();
    glRotatef(90, 1, 0, 0);
    glutSolidTorus(0.04f, 0.07f, 6, 12);
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

void drawSnakeSymbol3D() {
    glDisable(GL_LIGHTING);
    glColor3f(0.0f, 0.8f, 0.1f);
    glPushMatrix();
    glScalef(0.08f, 0.08f, 0.08f);
    glutSolidOctahedron();
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

void drawFireSymbol3D() {
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 0.3f, 0.0f);
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    glutSolidCone(0.08f, 0.16f, 8, 8);
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

void drawWaterSymbol3D() {
    glDisable(GL_LIGHTING);
    glColor3f(0.0f, 0.6f, 0.9f);
    glPushMatrix();
    glutSolidTorus(0.03f, 0.07f, 6, 12);
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

void drawEarthSymbol3D() {
    glDisable(GL_LIGHTING);
    glColor3f(0.55f, 0.35f, 0.15f);
    glutSolidCube(0.12f);
    glEnable(GL_LIGHTING);
}

void drawWindSymbol3D() {
    glDisable(GL_LIGHTING);
    glColor3f(0.8f, 0.95f, 1.0f);
    glPushMatrix();
    glutSolidTorus(0.02f, 0.06f, 6, 12);
    glPopMatrix();
    glEnable(GL_LIGHTING);
}



// --- Render Chamber 2 ---
void drawOctagonRoom2() {
    float r = 6.0f;
    float h = 5.0f;
    int segments = 8;

    glEnable(GL_LIGHTING);
    if (hasGroundTex) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texGround);
        glColor3f(1.0f, 1.0f, 1.0f);
    } else if (hasStoneTex) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texStone);
        glColor3f(1.0f, 1.0f, 1.0f);
    } else {
        glDisable(GL_TEXTURE_2D);
        glColor3f(0.2f, 0.22f, 0.2f);
    }

    GLfloat floorAmb[]  = { 0.55f, 0.55f, 0.55f, 1.0f };
    GLfloat floorDiff[] = { 1.0f,  1.0f,  1.0f,  1.0f };
    GLfloat floorSpec[] = { 0.0f,  0.0f,  0.0f,  1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT,  floorAmb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,  floorDiff);
    glMaterialfv(GL_FRONT, GL_SPECULAR, floorSpec);

    // Floor
    glBegin(GL_TRIANGLE_FAN);
    glTexCoord2f(0.0f, 0.0f); glNormal3f(0.0f, 1.0f, 0.0f); glVertex3f(0.0f, 0.0f, 0.0f);
    for (int i = 0; i <= segments; ++i) {
        float angle = i * 2.0f * PI / segments;
        float cx = r * cos(angle), cz = r * sin(angle);
        glTexCoord2f(cx * 0.5f, cz * 0.5f);
        glVertex3f(cx, 0.0f, cz);
    }
    glEnd();

    // Ceiling (keep stone)
    if (hasStoneTex) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texStone);
        glColor3f(1.0f, 1.0f, 1.0f);
    } else {
        glDisable(GL_TEXTURE_2D);
        glColor3f(0.2f, 0.22f, 0.2f);
    }

    GLfloat ceilAmb[]  = { 0.4f, 0.4f, 0.4f, 1.0f };
    GLfloat ceilDiff[] = { 0.6f, 0.6f, 0.6f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT,  ceilAmb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,  ceilDiff);

    glBegin(GL_TRIANGLE_FAN);
    glTexCoord2f(0.5f, 0.5f); glNormal3f(0.0f, -1.0f, 0.0f); glVertex3f(0.0f, h, 0.0f);
    for (int i = 0; i <= segments; ++i) {
        float angle = -i * 2.0f * PI / segments;
        glTexCoord2f(0.5f + 0.5f * cos(angle), 0.5f + 0.5f * sin(angle));
        glVertex3f(r * cos(angle), h, r * sin(angle));
    }
    glEnd();

    // Walls
    GLfloat matWallAmb[]  = { 0.15f, 0.2f, 0.15f, 1.0f };
    GLfloat matWallDiff[] = { 0.4f, 0.45f, 0.4f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, matWallAmb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matWallDiff);

    if (hasStoneTex) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texStone);
    }

    for (int i = 0; i < segments; ++i) {
        float angle1 = i * 2.0f * PI / segments;
        float angle2 = (i + 1) * 2.0f * PI / segments;

        float x1 = r * cos(angle1), z1 = r * sin(angle1);
        float x2 = r * cos(angle2), z2 = r * sin(angle2);

        float nx = -(cos(angle1) + cos(angle2)) / 2.0f;
        float nz = -(sin(angle1) + sin(angle2)) / 2.0f;
        float len = sqrt(nx*nx + nz*nz); nx /= len; nz /= len;

        glBegin(GL_QUADS);
        glNormal3f(nx, 0.0f, nz);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(x1, 0.0f, z1);
        glTexCoord2f(2.0f, 0.0f); glVertex3f(x2, 0.0f, z2);
        glTexCoord2f(2.0f, 2.0f); glVertex3f(x2, h,    z2);
        glTexCoord2f(0.0f, 2.0f); glVertex3f(x1, h,    z1);
        glEnd();
    }
    glDisable(GL_TEXTURE_2D);
}

void drawChamber2() {
    // 1. Draw octagonal room structure
    drawOctagonRoom2();

    // 2. Chandelier
    glDisable(GL_LIGHTING);
    glColor3f(0.15f, 0.15f, 0.15f);
    glBegin(GL_LINES); glVertex3f(0, 5, 0); glVertex3f(0, 3.2f, 0); glEnd();
    glEnable(GL_LIGHTING);

    glPushMatrix();
    glTranslatef(0.0f, 3.2f, 0.0f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    GLfloat matChandelierAmb[]  = { 0.2f, 0.15f, 0.25f, 1.0f };
    GLfloat matChandelierDiff[] = { 0.5f, 0.4f, 0.6f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, matChandelierAmb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matChandelierDiff);
    glutSolidTorus(0.05f, 0.3f, 8, 12);
    glPopMatrix();

    if (ch2BlackoutTimer == 0.0f) {
        glDisable(GL_LIGHTING); glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glColor4f(0.6f, 0.3f, 0.9f, 0.8f); // Purple chandelier light
        glPushMatrix(); glTranslatef(0.0f, 3.1f, 0.0f); glutSolidSphere(0.12f, 8, 8); glPopMatrix();
        glDisable(GL_BLEND); glEnable(GL_LIGHTING);
    }

    // 3. Rotating Rings (Concentric on far wall at z = -5.1f)
    glPushMatrix();
    glTranslatef(0.0f, 2.0f, -5.05f);

    GLfloat matRingAmb[]  = { 0.15f, 0.15f, 0.15f, 1.0f };
    GLfloat matRingDiff[] = { 0.35f, 0.35f, 0.35f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, matRingAmb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matRingDiff);

    // --- Outer Ring ---
    glPushMatrix();
    glRotatef(ringAngle1, 0.0f, 0.0f, 1.0f);
    glutSolidTorus(0.04f, 1.3f, 8, 48);
    // Sun at 0 deg (top)
    glPushMatrix(); glTranslatef(0.0f, 1.3f, 0.0f); drawSunSymbol3D(); glPopMatrix();
    // Moon at 90 deg (right)
    glPushMatrix(); glRotatef(90, 0,0,1); glTranslatef(0.0f, 1.3f, 0.0f); drawMoonSymbol3D(); glPopMatrix();
    // Star at 180 deg (bottom)
    glPushMatrix(); glRotatef(180, 0,0,1); glTranslatef(0.0f, 1.3f, 0.0f); drawStarSymbol3D(); glPopMatrix();
    // Cloud at 270 deg (left)
    glPushMatrix(); glRotatef(270, 0,0,1); glTranslatef(0.0f, 1.3f, 0.0f); drawCloudSymbol3D(); glPopMatrix();
    glPopMatrix();

    // --- Middle Ring ---
    glPushMatrix();
    glRotatef(ringAngle2, 0.0f, 0.0f, 1.0f);
    glutSolidTorus(0.04f, 0.9f, 8, 36);
    // Eagle at 0 deg (top)
    glPushMatrix(); glTranslatef(0.0f, 0.9f, 0.0f); drawEagleSymbol3D(); glPopMatrix();
    // Wolf at 90 deg (right)
    glPushMatrix(); glRotatef(90, 0,0,1); glTranslatef(0.0f, 0.9f, 0.0f); drawWolfSymbol3D(); glPopMatrix();
    // Lion at 180 deg (bottom)
    glPushMatrix(); glRotatef(180, 0,0,1); glTranslatef(0.0f, 0.9f, 0.0f); drawLionSymbol3D(); glPopMatrix();
    // Snake at 270 deg (left)
    glPushMatrix(); glRotatef(270, 0,0,1); glTranslatef(0.0f, 0.9f, 0.0f); drawSnakeSymbol3D(); glPopMatrix();
    glPopMatrix();

    // --- Inner Ring ---
    glPushMatrix();
    glRotatef(ringAngle3, 0.0f, 0.0f, 1.0f);
    glutSolidTorus(0.04f, 0.5f, 8, 24);
    // Fire at 0 deg (top)
    glPushMatrix(); glTranslatef(0.0f, 0.5f, 0.0f); drawFireSymbol3D(); glPopMatrix();
    // Water at 90 deg (right)
    glPushMatrix(); glRotatef(90, 0,0,1); glTranslatef(0.0f, 0.5f, 0.0f); drawWaterSymbol3D(); glPopMatrix();
    // Earth at 180 deg (bottom)
    glPushMatrix(); glRotatef(180, 0,0,1); glTranslatef(0.0f, 0.5f, 0.0f); drawEarthSymbol3D(); glPopMatrix();
    // Wind at 270 deg (left)
    glPushMatrix(); glRotatef(270, 0,0,1); glTranslatef(0.0f, 0.5f, 0.0f); drawWindSymbol3D(); glPopMatrix();
    glPopMatrix();

    glPopMatrix(); // concentric rings pop

    // 4. Aligning light beam & Glowing Lock
    if (ch2Solved) {
        // Magical alignment beam
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        // Render light beam cylinder from pedestal/back wall through rings
        glColor4f(0.5f, 0.2f, 1.0f, 0.7f); // Purple magical light beam
        glPushMatrix();
        glTranslatef(0.0f, 2.0f, -5.0f);
        // Draw cylinder
        GLUquadric* quad = gluNewQuadric();
        gluCylinder(quad, 0.12f, 0.12f, 5.0f, 16, 1);
        gluDeleteQuadric(quad);
        glPopMatrix();

        // Lock glow on exit wall
        glColor4f(0.7f, 0.4f, 1.0f, 0.9f);
        glPushMatrix();
        glTranslatef(0.0f, 2.0f, -5.18f);
        glutSolidSphere(0.18f, 12, 12);
        glPopMatrix();

        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
    } else {
        // Standard unlit circular stone lock
        glPushMatrix();
        glTranslatef(0.0f, 2.0f, -5.19f);
        GLfloat matLock[] = { 0.25f, 0.25f, 0.25f, 1.0f };
        glMaterialfv(GL_FRONT, GL_DIFFUSE, matLock);
        glutSolidSphere(0.15f, 10, 10);
        glPopMatrix();
    }

    // 5. Interaction Wheels (A, B, C)
    for (int i = 0; i < 3; ++i) {
        glPushMatrix();
        glTranslatef(WHEEL_POS[i][0], 0.35f, WHEEL_POS[i][1]);
        
        // Pedestal base
        drawPrism(0.4f, 0.7f, 0.4f, texStone, hasStoneTex, 0.35f, 0.35f, 0.35f, 1, 1);
        
        // Wheel top (horizontal torus)
        glTranslatef(0.0f, 0.37f, 0.0f);
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        GLfloat matWheel[] = { 0.45f, 0.4f, 0.3f, 1.0f };
        glMaterialfv(GL_FRONT, GL_DIFFUSE, matWheel);
        
        // Rotate the wheel rendering slightly based on what ring it controls
        if (i == 0) glRotatef(ringAngle1, 0.0f, 0.0f, 1.0f);
        else if (i == 1) glRotatef(ringAngle2, 0.0f, 0.0f, 1.0f);
        else glRotatef(ringAngle3, 0.0f, 0.0f, 1.0f);
        
        glutSolidTorus(0.03f, 0.15f, 6, 12);
        
        // Spoke lines
        glDisable(GL_LIGHTING);
        glColor3f(0.2f, 0.15f, 0.1f);
        glBegin(GL_LINES);
        glVertex3f(-0.15f, 0.0f, 0.0f); glVertex3f(0.15f, 0.0f, 0.0f);
        glVertex3f(0.0f, -0.15f, 0.0f); glVertex3f(0.0f, 0.15f, 0.0f);
        glEnd();
        glEnable(GL_LIGHTING);
        
        glPopMatrix();
    }

    // 6. Central Pedestal
    glPushMatrix();
    glTranslatef(0.0f, 0.25f, 0.0f);
    drawPrism(0.7f, 0.5f, 0.7f, texBox, hasBoxTex, 0.40f, 0.40f, 0.40f, 1.0f, 1.5f);
    glPopMatrix();

    // Sliding Lid (slides open when solved)
    glPushMatrix();
    glTranslatef(0.0f, 0.52f, -0.35f - ch2PedestalOpenProgress * 0.5f);
    drawPrism(0.72f, 0.06f, 0.72f, texBox, hasBoxTex, 0.50f, 0.50f, 0.50f, 1.0f, 1.0f);
    glPopMatrix();

    // The Golden Key for Chamber 2 (visible only when solved)
    if (ch2Solved && !hasCh2Key) {
        static float keyRot = 0.0f;
        keyRot += 1.5f;

        glPushMatrix();
        glTranslatef(0.0f, 0.75f + 0.05f * sin(keyRot * PI / 180.0f), 0.0f);
        glRotatef(keyRot, 0.0f, 1.0f, 0.0f);

        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 0.85f, 0.1f);

        // Ring
        glPushMatrix(); glTranslatef(0.0f, 0.15f, 0.0f); glutSolidTorus(0.03f, 0.08f, 6, 8); glPopMatrix();
        // Shaft
        glPushMatrix(); glTranslatef(0.0f, -0.05f, 0.0f); drawPrism(0.04f, 0.3f, 0.04f, 0, false, 1.0f, 0.85f, 0.1f, 1.0f, 1.0f); glPopMatrix();
        // Teeth
        glPushMatrix(); glTranslatef(0.06f, -0.15f, 0.0f); drawPrism(0.08f, 0.06f, 0.03f, 0, false, 1.0f, 0.85f, 0.1f, 1.0f, 1.0f); glPopMatrix();

        glEnable(GL_LIGHTING);
        glPopMatrix();
    }

    // 7. Exit Doors (Clock and Hourglass)
    // Left Door (Clock) at x = -1.2, z = -5.2
    glPushMatrix();
    glTranslatef(-1.2f, 1.05f, -5.2f);
    drawPrism(0.9f, 2.1f, 0.08f, texWood, hasWoodTex, 0.45f, 0.28f, 0.12f, 1.0f, 3.0f);
    glTranslatef(0.0f, 0.0f, 0.041f);
    drawDoorText("CLOCK");
    glPopMatrix();

    // Right Door (Hourglass) at x = 1.2, z = -5.2
    glPushMatrix();
    glTranslatef(1.2f, 1.05f, -5.2f);
    drawPrism(0.9f, 2.1f, 0.08f, texWood, hasWoodTex, 0.45f, 0.28f, 0.12f, 1.0f, 3.0f);
    glTranslatef(0.0f, 0.0f, 0.041f);
    drawDoorText("HOURGLASS");
    glPopMatrix();
}

// --- Chamber 2 Lighting ---
void setupChamber2Lighting() {
    glClearColor(0.02f, 0.01f, 0.06f, 1.0f); // Dark violet chamber clear
    GLfloat fogColor[4] = { 0.02f, 0.01f, 0.06f, 1.0f };
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogf(GL_FOG_DENSITY, 0.035f);

    if (ch2BlackoutTimer > 0.0f) {
        GLfloat globalAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
        glDisable(GL_LIGHT0); glDisable(GL_LIGHT1); glDisable(GL_LIGHT2); glDisable(GL_LIGHT3);
    } else {
        GLfloat globalAmbient[] = { 0.45f, 0.4f, 0.55f, 1.0f };
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

        glDisable(GL_LIGHT0); glDisable(GL_LIGHT1); glDisable(GL_LIGHT2);

        // Chandelier light - purple tint
        GLfloat lightAmbient3[]  = { 0.2f, 0.15f, 0.3f, 1.0f };
        GLfloat lightDiffuse3[]  = { 0.8f, 0.6f, 0.95f, 1.0f };
        GLfloat lightSpecular3[] = { 0.3f, 0.3f, 0.3f, 1.0f };
        GLfloat lightPos3[]      = { 0.0f, 3.2f, 0.0f, 1.0f };
        glLightfv(GL_LIGHT3, GL_AMBIENT,  lightAmbient3);
        glLightfv(GL_LIGHT3, GL_DIFFUSE,  lightDiffuse3);
        glLightfv(GL_LIGHT3, GL_SPECULAR, lightSpecular3);
        glLightfv(GL_LIGHT3, GL_POSITION, lightPos3);

        glLightf(GL_LIGHT3, GL_CONSTANT_ATTENUATION,  0.5f);
        glLightf(GL_LIGHT3, GL_LINEAR_ATTENUATION,    0.05f);
        glLightf(GL_LIGHT3, GL_QUADRATIC_ATTENUATION, 0.01f);
        glEnable(GL_LIGHT3);
    }
}

// --- Interaction Handler ---
void handleChamber2Interaction() {
    if (currentGameState != STATE_CHAMBER_2 || ch2Solved || isCh2FallingInTrap) return;

    // Check proximity to the three rotation controllers (Wheels A, B, C)
    for (int i = 0; i < 3; ++i) {
        float dx = camX - WHEEL_POS[i][0];
        float dz = camZ - WHEEL_POS[i][1];
        if (sqrt(dx * dx + dz * dz) < 0.8f) {
            ch2RumbleTimer = 0.15f; // Short screen rumble to simulate pull force

            if (i == 0) {
                // Wheel A rotates only the Outer Ring
                targetRingAngle1 += 90.0f;
                std::cout << "Wheel A pulled -> Rotating Outer Ring to " << targetRingAngle1 << std::endl;
            } else if (i == 1) {
                // Wheel B rotates only the Middle Ring
                targetRingAngle2 += 90.0f;
                std::cout << "Wheel B pulled -> Rotating Middle Ring to " << targetRingAngle2 << std::endl;
            } else if (i == 2) {
                // Wheel C rotates BOTH Middle & Inner Rings (linked puzzle difficulty)
                targetRingAngle2 += 90.0f;
                targetRingAngle3 += 90.0f;
                std::cout << "Wheel C pulled -> Rotating Middle & Inner Rings" << std::endl;
            }
            break;
        }
    }
}

// --- Chamber 2 Update Logic ---
void updateChamber2() {
    if (fadeAlpha > 0.0f) {
        fadeAlpha -= 0.04f;
        if (fadeAlpha < 0.0f) fadeAlpha = 0.0f;
    }

    // 1. Smooth Ring Rotation Interpolation (lerps current angle towards target angle)
    ringAngle1 += (targetRingAngle1 - ringAngle1) * 0.1f;
    ringAngle2 += (targetRingAngle2 - ringAngle2) * 0.1f;
    ringAngle3 += (targetRingAngle3 - ringAngle3) * 0.1f;

    // Pedestal lid animation
    if (ch2Solved && ch2PedestalOpenProgress < 1.0f) {
        ch2PedestalOpenProgress += 0.02f;
        if (ch2PedestalOpenProgress > 1.0f) ch2PedestalOpenProgress = 1.0f;
    }

    // Timers
    if (ch2BlackoutTimer > 0.0f) {
        ch2BlackoutTimer -= 0.016f;
        if (ch2BlackoutTimer < 0.0f) ch2BlackoutTimer = 0.0f;
    }
    if (ch2RumbleTimer > 0.0f) {
        ch2RumbleTimer -= 0.016f;
        if (ch2RumbleTimer < 0.0f) ch2RumbleTimer = 0.0f;
    }

    // 2. Check Trap Falling Animation (resets chamber when finished)
    if (isCh2FallingInTrap) {
        ch2TrapFallY += 0.12f;
        ch2TrapFade += 0.02f;
        if (ch2TrapFade >= 1.0f) {
            // Restart Chamber 2
            camX = 0.0f; camY = 1.0f; camZ = 4.8f;
            rotX = 10.0f; rotY = 0.0f;
            isCh2FallingInTrap = false;
            ch2TrapFallY = 0.0f;
            ch2TrapFade = 0.0f;
            resetChamber2();
        }
    } else {
        // Player Movement
        const float PLAYER_SPEED = 0.06f;
        const float TURN_SPEED = 1.8f;

        if (keyStates['a'] || keyStates['A'] || specialKeyStates[GLUT_KEY_LEFT]) {
            rotY += TURN_SPEED;
        }
        if (keyStates['d'] || keyStates['D'] || specialKeyStates[GLUT_KEY_RIGHT]) {
            rotY -= TURN_SPEED;
        }

        float moveX = 0.0f;
        float moveZ = 0.0f;
        bool isMoving = false;

        if (keyStates['w'] || keyStates['W'] || specialKeyStates[GLUT_KEY_UP]) {
            float rad = rotY * PI / 180.0f;
            moveX = -sin(rad) * PLAYER_SPEED;
            moveZ = -cos(rad) * PLAYER_SPEED;
            isMoving = true;
        }
        if (keyStates['s'] || keyStates['S'] || specialKeyStates[GLUT_KEY_DOWN]) {
            float rad = rotY * PI / 180.0f;
            moveX = sin(rad) * PLAYER_SPEED;
            moveZ = cos(rad) * PLAYER_SPEED;
            isMoving = true;
        }

        if (isMoving) {
            float nX = camX + moveX;
            float nZ = camZ + moveZ;

            // Collision checks
            float dist = sqrt(nX * nX + nZ * nZ);
            bool col = false;

            // Room outer wall (radius 5.8)
            if (dist > 5.8f) col = true;
            // Center Pedestal (radius 0.6)
            if (dist < 0.6f) col = true;

            // Collision with the 3 Wheels (radius 0.5)
            for (int i = 0; i < 3; ++i) {
                float dx = nX - WHEEL_POS[i][0];
                float dz = nZ - WHEEL_POS[i][1];
                if (sqrt(dx * dx + dz * dz) < 0.5f) col = true;
            }

            if (!hasCh2Key && nZ < -4.8f && fabs(nX) < 1.8f) col = true;

            if (!col) {
                camX = nX;
                camZ = nZ;
            }
        }

        // 3. Check Puzzle Alignment
        // Sun, Eagle, Fire aligned at top 12 o'clock => angles are multiples of 360 deg (0 mod 360)
        if (!ch2Solved) {
            int a1 = ((int)targetRingAngle1) % 360;
            int a2 = ((int)targetRingAngle2) % 360;
            int a3 = ((int)targetRingAngle3) % 360;

            if (a1 == 0 && a2 == 0 && a3 == 0) {
                ch2Solved = true;
                std::cout << "SUCCESS! Rings fully aligned!" << std::endl;
            }
        }

        // Key collection from central pedestal
        if (ch2Solved && !hasCh2Key) {
            float dx = camX - 0.0f;
            float dz = camZ - 0.0f;
            if (sqrt(dx * dx + dz * dz) < 0.9f) {
                hasCh2Key = true;
                std::cout << "Chamber 2 Key Acquired!" << std::endl;
            }
        }

        // 4. Exit Doors Riddle Verification
        // Riddle: "What has hands but cannot clap?" -> Answer: CLOCK (Left Door)
        if (hasCh2Key && camZ < -4.5f) {
            // Left Door (CLOCK) [CORRECT] - Transition to Chamber 3
            float dxL = camX - (-1.2f);
            float dzL = camZ - (-5.2f);
            if (sqrt(dxL * dxL + dzL * dzL) < 0.6f) {
                currentGameState = STATE_CHAMBER_3;
                camX = 0.0f; camY = 1.0f; camZ = 4.8f;
                rotX = 10.0f; rotY = 0.0f;
                fadeAlpha = 1.0f;
                resetChamber3();
            }

            // Right Door (HOURGLASS) [TRAP] - Trigger trap animation
            float dxR = camX - 1.2f;
            float dzR = camZ - (-5.2f);
            if (sqrt(dxR * dxR + dzR * dzR) < 0.6f) {
                isCh2FallingInTrap = true;
                ch2TrapFallY = 0.0f;
                ch2TrapFade = 0.0f;
                ch2RumbleTimer = 0.8f;
            }
        }
    }
}

// --- Chamber 2 HUD ---
void drawChamber2HUD() {
    if (currentGameState == STATE_CHAMBER_2) {
        // Top and Bottom HUD boxes
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.02f, 0.02f, 0.08f, 0.85f);
        glBegin(GL_QUADS);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(WINDOW_WIDTH, 0.0f);
        glVertex2f(WINDOW_WIDTH, 120.0f);
        glVertex2f(0.0f, 120.0f);
        glEnd();

        glBegin(GL_QUADS);
        glVertex2f(0.0f, WINDOW_HEIGHT - 65.0f);
        glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT - 65.0f);
        glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
        glVertex2f(0.0f, WINDOW_HEIGHT);
        glEnd();
        glDisable(GL_BLEND);

        glColor3f(0.6f, 0.4f, 1.0f);
        renderBitmapString(25.0f, WINDOW_HEIGHT - 25.0f, GLUT_BITMAP_HELVETICA_18, "CHAMBER 2: THE HALL OF TURNING RINGS");

        glColor3f(0.9f, 0.8f, 0.6f);
        renderBitmapString(25.0f, WINDOW_HEIGHT - 50.0f, GLUT_BITMAP_HELVETICA_12,
            "Wall Clues: \"The king fears the flames. The sky follows the sun. The flames stand beneath the sun.\"");

        if (!ch2Solved) {
            glColor3f(1.0f, 1.0f, 1.0f);
            renderBitmapString(30.0f, 90.0f, GLUT_BITMAP_HELVETICA_12, "Objective: Align the symbols (Sun, Eagle, Fire) at the top of the rings.");
            
            // Interaction help text if near a wheel
            bool nearWheel = false;
            for (int i = 0; i < 3; ++i) {
                float dx = camX - WHEEL_POS[i][0];
                float dz = camZ - WHEEL_POS[i][1];
                if (sqrt(dx * dx + dz * dz) < 0.8f) {
                    nearWheel = true;
                    glColor3f(0.0f, 0.9f, 1.0f);
                    if (i == 0) renderBitmapString(30.0f, 65.0f, GLUT_BITMAP_HELVETICA_12, ">> PRESS [ENTER] TO TURN WHEEL A (Controls Outer Ring)");
                    else if (i == 1) renderBitmapString(30.0f, 65.0f, GLUT_BITMAP_HELVETICA_12, ">> PRESS [ENTER] TO TURN WHEEL B (Controls Middle Ring)");
                    else renderBitmapString(30.0f, 65.0f, GLUT_BITMAP_HELVETICA_12, ">> PRESS [ENTER] TO TURN WHEEL C (Controls Middle & Inner Rings)");
                    break;
                }
            }
            if (!nearWheel) {
                glColor3f(0.5f, 0.8f, 0.5f);
                renderBitmapString(30.0f, 65.0f, GLUT_BITMAP_HELVETICA_12, "Find and approach the 3 rotating wheels around the room to turn the rings.");
            }
        } else if (!hasCh2Key) {
            glColor3f(1.0f, 0.85f, 0.1f);
            renderBitmapString(30.0f, 90.0f, GLUT_BITMAP_HELVETICA_12, "SUCCESS! The rings are aligned. The pedestal lid has opened.");
            glColor3f(1.0f, 1.0f, 1.0f);
            renderBitmapString(30.0f, 65.0f, GLUT_BITMAP_HELVETICA_12, "Objective: Approach the central pedestal and collect the Golden Key.");
        } else {
            glColor3f(0.2f, 0.9f, 0.3f);
            renderBitmapString(30.0f, 95.0f, GLUT_BITMAP_HELVETICA_12, "KEY ACQUIRED! The exit doors are now active.");
            glColor3f(0.9f, 0.85f, 0.5f);
            renderBitmapString(30.0f, 75.0f, GLUT_BITMAP_HELVETICA_12,
                "Door Riddle: \"What has hands but cannot clap?\"");
            glColor3f(1.0f, 1.0f, 1.0f);
            renderBitmapString(30.0f, 50.0f, GLUT_BITMAP_HELVETICA_12,
                "Objective: Walk through the correct door - Left Door (Clock) or Right Door (Hourglass)");
        }

        glColor3f(0.7f, 0.7f, 0.7f);
        renderBitmapString(30.0f, 20.0f, GLUT_BITMAP_HELVETICA_10, "Controls: WASD / Arrow Keys to Move | ENTER to interact | R to reset");

        // --- FLOATING SYMBOL LEGEND PANEL ---
        float boxLeft = WINDOW_WIDTH - 270.0f;
        float boxRight = WINDOW_WIDTH - 15.0f;
        float boxBottom = 135.0f;
        float boxTop = WINDOW_HEIGHT - 80.0f;

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.01f, 0.01f, 0.04f, 0.85f);
        glBegin(GL_QUADS);
        glVertex2f(boxLeft, boxBottom);
        glVertex2f(boxRight, boxBottom);
        glVertex2f(boxRight, boxTop);
        glVertex2f(boxLeft, boxTop);
        glEnd();

        // Border
        glColor4f(0.6f, 0.4f, 1.0f, 0.6f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(boxLeft, boxBottom);
        glVertex2f(boxRight, boxBottom);
        glVertex2f(boxRight, boxTop);
        glVertex2f(boxLeft, boxTop);
        glEnd();
        glLineWidth(1.0f);
        glDisable(GL_BLEND);

        float lx = boxLeft + 15.0f;
        float ly = boxTop - 25.0f;

        glColor3f(0.6f, 0.4f, 1.0f);
        renderBitmapString(lx, ly, GLUT_BITMAP_HELVETICA_12, "SYMBOL LEGEND");
        ly -= 20.0f;

        // Outer Ring
        glColor3f(0.9f, 0.9f, 0.9f);
        renderBitmapString(lx, ly, GLUT_BITMAP_HELVETICA_10, "Outer Ring:");
        ly -= 15.0f;
        glColor3f(1.0f, 0.8f, 0.0f);
        renderBitmapString(lx + 10.0f, ly, GLUT_BITMAP_HELVETICA_10, "- Sun : Yellow Sphere");
        ly -= 15.0f;
        glColor3f(0.85f, 0.85f, 0.9f);
        renderBitmapString(lx + 10.0f, ly, GLUT_BITMAP_HELVETICA_10, "- Moon : Silver Ring");
        ly -= 15.0f;
        glColor3f(1.0f, 1.0f, 0.5f);
        renderBitmapString(lx + 10.0f, ly, GLUT_BITMAP_HELVETICA_10, "- Star : Light Yellow Cone");
        ly -= 15.0f;
        glColor3f(0.95f, 0.95f, 0.95f);
        renderBitmapString(lx + 10.0f, ly, GLUT_BITMAP_HELVETICA_10, "- Cloud : White Spheres");
        ly -= 25.0f;

        // Middle Ring
        glColor3f(0.9f, 0.9f, 0.9f);
        renderBitmapString(lx, ly, GLUT_BITMAP_HELVETICA_10, "Middle Ring:");
        ly -= 15.0f;
        glColor3f(0.1f, 0.4f, 1.0f);
        renderBitmapString(lx + 10.0f, ly, GLUT_BITMAP_HELVETICA_10, "- Eagle : Blue Cone");
        ly -= 15.0f;
        glColor3f(0.6f, 0.6f, 0.65f);
        renderBitmapString(lx + 10.0f, ly, GLUT_BITMAP_HELVETICA_10, "- Wolf : Gray Sphere");
        ly -= 15.0f;
        glColor3f(1.0f, 0.75f, 0.0f);
        renderBitmapString(lx + 10.0f, ly, GLUT_BITMAP_HELVETICA_10, "- Lion : Gold Ring");
        ly -= 15.0f;
        glColor3f(0.0f, 0.8f, 0.1f);
        renderBitmapString(lx + 10.0f, ly, GLUT_BITMAP_HELVETICA_10, "- Snake : Green Diamond");
        ly -= 25.0f;

        // Inner Ring
        glColor3f(0.9f, 0.9f, 0.9f);
        renderBitmapString(lx, ly, GLUT_BITMAP_HELVETICA_10, "Inner Ring:");
        ly -= 15.0f;
        glColor3f(1.0f, 0.3f, 0.0f);
        renderBitmapString(lx + 10.0f, ly, GLUT_BITMAP_HELVETICA_10, "- Fire : Red Cone");
        ly -= 15.0f;
        glColor3f(0.0f, 0.6f, 0.9f);
        renderBitmapString(lx + 10.0f, ly, GLUT_BITMAP_HELVETICA_10, "- Water : Teal Ring");
        ly -= 15.0f;
        glColor3f(0.55f, 0.35f, 0.15f);
        renderBitmapString(lx + 10.0f, ly, GLUT_BITMAP_HELVETICA_10, "- Earth : Brown Cube");
        ly -= 15.0f;
        glColor3f(0.8f, 0.95f, 1.0f);
        renderBitmapString(lx + 10.0f, ly, GLUT_BITMAP_HELVETICA_10, "- Wind : Cyan Ring");

        if (isCh2FallingInTrap) {
            glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(0.3f, 0.0f, 0.0f, ch2TrapFade);
            glBegin(GL_QUADS); glVertex2f(0, 0); glVertex2f(WINDOW_WIDTH, 0); glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT); glVertex2f(0, WINDOW_HEIGHT); glEnd();
            glColor3f(1.0f, 0.1f, 0.1f); renderBitmapString(WINDOW_WIDTH/2 - 160.0f, WINDOW_HEIGHT/2, GLUT_BITMAP_HELVETICA_18, "YOU CHOSE WRONG! TRAP DETONATED!");
            glColor3f(0.8f, 0.8f, 0.8f); renderBitmapString(WINDOW_WIDTH/2 - 100.0f, WINDOW_HEIGHT/2 - 30.0f, GLUT_BITMAP_HELVETICA_12, "Resetting chamber...");
            glDisable(GL_BLEND);
        }
    } else if (currentGameState == STATE_VICTORY) {
        glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.08f, 0.07f, 0.02f, 0.9f);
        glBegin(GL_QUADS); glVertex2f(0, 0); glVertex2f(WINDOW_WIDTH, 0); glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT); glVertex2f(0, WINDOW_HEIGHT); glEnd();
        glDisable(GL_BLEND);
        glColor3f(1.0f, 0.85f, 0.1f); renderBitmapString(WINDOW_WIDTH/2 - 150.0f, WINDOW_HEIGHT/2 + 60.0f, GLUT_BITMAP_HELVETICA_18, "CHAMBER 2 COMPLETE!");
        glColor3f(0.9f, 0.9f, 0.9f); renderBitmapString(WINDOW_WIDTH/2 - 250.0f, WINDOW_HEIGHT/2 + 10.0f, GLUT_BITMAP_HELVETICA_12, "You correctly identified the Clock as the answer to the riddle and safely escaped!");
        renderBitmapString(WINDOW_WIDTH/2 - 180.0f, WINDOW_HEIGHT/2 - 20.0f, GLUT_BITMAP_HELVETICA_12, "Congratulations! You have completed the Dungeon Quest!");
        glColor3f(0.0f, 0.9f, 1.0f); renderBitmapString(WINDOW_WIDTH/2 - 140.0f, WINDOW_HEIGHT/2 - 80.0f, GLUT_BITMAP_HELVETICA_12, "PRESS [R] TO PLAY AGAIN FROM START");
    }
}
