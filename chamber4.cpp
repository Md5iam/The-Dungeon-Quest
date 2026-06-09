#include "chamber4.h"
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <cstdio>
#include <vector>

// --- Chamber 4 State Variables ---
bool ch4Solved = false;
bool hasCh4Key = false;
bool isCh4FallingInTrap = false;
float ch4TrapFallY = 0.0f;
float ch4TrapFade = 0.0f;
float ch4RumbleTimer = 0.0f;
float ch4PedestalOpenProgress = 0.0f;

// Legacy variables declared in header to satisfy linker
float ch4MirrorAngles[2] = {0.0f, 0.0f};
bool ch4Light0On = false;
bool ch4Light1On = false;
float fanRotation = 0.0f;

// Pedestal coordinates (center of the room)
static const float pedestalX = 0.0f;
static const float pedestalZ = 0.0f;

// --- Horror Stealth Game State ---
static float ghostX = 0.0f;
static float ghostZ = 0.0f;
static float ghostY = 1.0f;
static int ghostState = 0; // 0 = Patrol, 1 = Chase
static int currentWaypointIndex = 0;
static float ghostWaypoints[5][2] = {
    { 0.0f,  0.0f}, // Center
    {-3.8f, -3.8f}, // Room 1 (NW)
    { 3.8f, -3.8f}, // Room 2 (NE)
    {-3.8f,  3.8f}, // Room 3 (SW)
    { 3.8f,  3.8f}  // Room 4 (SE)
};

static bool itemCollected[4] = {false, false, false, false};
static float itemPos[4][3] = {
    {-3.8f, 0.35f, -3.8f}, // Gem 1 (Blue)
    { 3.8f, 0.35f, -3.8f}, // Gem 2 (Green)
    {-3.8f, 0.35f,  3.8f}, // Gem 3 (Orange)
    { 3.8f, 0.35f,  3.8f}  // Gem 4 (Purple)
};

static float cabinetPos[4][2] = {
    {-4.5f, -4.5f},
    { 4.5f, -4.5f},
    {-4.5f,  4.5f},
    { 4.5f,  4.5f}
};

static bool isPlayerHidden = false;
static float caughtRedFlash = 0.0f;
static float HUDFeedbackTimer = 0.0f;
static const char* HUDFeedbackMsg = "";

// --- Ghost Particle System ---
struct GhostParticle {
    float x, y, z;
    float vx, vy, vz;
    float r, g, b, alpha;
    float life;
};
static std::vector<GhostParticle> ghostParticles;
static const int GHOST_MAX_PARTICLES = 50;

static void initGhostParticle(GhostParticle &p) {
    float angle = ((float)rand() / RAND_MAX) * 2.0f * PI;
    float radius = 0.05f + ((float)rand() / RAND_MAX) * 0.15f;
    p.x = ghostX + radius * cos(angle);
    p.y = ghostY - 0.2f + ((float)rand() / RAND_MAX) * 0.4f;
    p.z = ghostZ + radius * sin(angle);

    p.vx = (((float)rand() / RAND_MAX) - 0.5f) * 0.005f;
    p.vy = 0.005f + ((float)rand() / RAND_MAX) * 0.008f; // rise up
    p.vz = (((float)rand() / RAND_MAX) - 0.5f) * 0.005f;

    // Dark red and purple trail colors
    if (rand() % 2 == 0) {
        p.r = 0.9f; p.g = 0.0f; p.b = 0.1f;
    } else {
        p.r = 0.5f; p.g = 0.0f; p.b = 0.8f;
    }
    p.alpha = 0.6f + ((float)rand() / RAND_MAX) * 0.4f;
    p.life = 0.4f + ((float)rand() / RAND_MAX) * 0.5f;
}

static void initGhostParticleSystem() {
    ghostParticles.resize(GHOST_MAX_PARTICLES);
    for (int i = 0; i < GHOST_MAX_PARTICLES; ++i) {
        initGhostParticle(ghostParticles[i]);
        ghostParticles[i].life = (float)rand() / RAND_MAX;
    }
}

static void updateGhostParticles() {
    for (int i = 0; i < GHOST_MAX_PARTICLES; ++i) {
        ghostParticles[i].x += ghostParticles[i].vx;
        ghostParticles[i].y += ghostParticles[i].vy;
        ghostParticles[i].z += ghostParticles[i].vz;
        ghostParticles[i].life -= 0.012f;
        ghostParticles[i].alpha = ghostParticles[i].life;

        if (ghostParticles[i].life <= 0.0f) {
            initGhostParticle(ghostParticles[i]);
        }
    }
}

static void drawGhostParticles() {
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);

    glPointSize(4.0f);
    glBegin(GL_POINTS);
    for (const auto &p : ghostParticles) {
        glColor4f(p.r, p.g, p.b, p.alpha);
        glVertex3f(p.x, p.y, p.z);
    }
    glEnd();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

// --- Collision Helper ---
static bool checkCh4Collision(float x, float z) {
    // Octagon boundary
    if (sqrt(x*x + z*z) > 5.8f) return true;

    // Locked exit door collision
    if (!hasCh4Key && z < -4.8f && fabs(x - 0.0f) < 0.8f) return true;

    // Room 1 (NW) partition walls
    // Doorway is at x = -2.0, z in [-3.2, -2.0]
    if (x < -2.0f && fabs(z - (-2.0f)) < 0.25f) return true; // Wall H
    if (z < -3.2f && fabs(x - (-2.0f)) < 0.25f) return true; // Wall V

    // Room 2 (NE) partition walls
    // Doorway is at x = 2.0, z in [-3.2, -2.0]
    if (x > 2.0f && fabs(z - (-2.0f)) < 0.25f) return true;  // Wall H
    if (z < -3.2f && fabs(x - 2.0f) < 0.25f) return true;   // Wall V

    // Room 3 (SW) partition walls
    // Doorway is at x = -2.0, z in [2.0, 3.2]
    if (x < -2.0f && fabs(z - 2.0f) < 0.25f) return true;   // Wall H
    if (z > 3.2f && fabs(x - (-2.0f)) < 0.25f) return true;  // Wall V

    // Room 4 (SE) partition walls
    // Doorway is at x = 2.0, z in [2.0, 3.2]
    if (x > 2.0f && fabs(z - 2.0f) < 0.25f) return true;    // Wall H
    if (z > 3.2f && fabs(x - 2.0f) < 0.25f) return true;    // Wall V

    // Pedestal collision in the SW room corner
    float dx = x - pedestalX;
    float dz = z - pedestalZ;
    if (sqrt(dx*dx + dz*dz) < 0.8f) return true;

    return false;
}

// --- Draw Ghost Model (Detailed Cloak-type Structure) ---
static void drawGhost() {
    glPushMatrix();
    glTranslatef(ghostX, ghostY, ghostZ);

    // Turn to face the player
    float dx = camX - ghostX;
    float dz = camZ - ghostZ;
    float angle = atan2(dx, dz) * 180.0f / PI;
    glRotatef(angle, 0.0f, 1.0f, 0.0f);

    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 1. Head/Shroud Hood
    if (ghostState == 1) {
        glColor4f(1.0f, 0.1f, 0.1f, 0.85f);
    } else {
        glColor4f(0.82f, 0.85f, 0.95f, 0.65f); // ghostly pale blue shroud
    }
    glPushMatrix();
    glTranslatef(0.0f, 0.25f, 0.0f);
    glutSolidSphere(0.18f, 16, 16);
    glPopMatrix();

    // 2. Wavy Tattered Cloak (using triangle strip and fanRotation sine waves)
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= 12; ++i) {
        float theta = i * 30.0f * PI / 180.0f;
        float wave = 0.07f * sin(fanRotation * 0.1f + i);
        float r_top = 0.15f;
        float r_bot = 0.26f + 0.04f * wave;
        
        float x_top = r_top * cos(theta);
        float z_top = r_top * sin(theta);
        float y_top = 0.20f;

        float x_bot = r_bot * cos(theta);
        float z_bot = r_bot * sin(theta);
        float y_bot = -0.45f + wave;

        if (ghostState == 1) {
            glColor4f(1.0f, 0.1f, 0.1f, 0.8f);
            glVertex3f(x_top, y_top, z_top);
            glColor4f(0.7f, 0.0f, 0.0f, 0.1f);
            glVertex3f(x_bot, y_bot, z_bot);
        } else {
            glColor4f(0.82f, 0.85f, 0.95f, 0.7f);
            glVertex3f(x_top, y_top, z_top);
            glColor4f(0.45f, 0.55f, 0.8f, 0.0f);
            glVertex3f(x_bot, y_bot, z_bot);
        }
    }
    glEnd();

    // 3. Glowing red eyes
    glColor3f(1.0f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(-0.06f, 0.28f, 0.13f);
    glutSolidSphere(0.035f, 8, 8);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.06f, 0.28f, 0.13f);
    glutSolidSphere(0.035f, 8, 8);
    glPopMatrix();

    // 4. Wavy Floating Arms
    if (ghostState == 1) {
        glColor4f(1.0f, 0.1f, 0.1f, 0.55f);
    } else {
        glColor4f(0.82f, 0.85f, 0.95f, 0.45f);
    }
    // Left Arm
    glPushMatrix();
    glTranslatef(-0.2f, 0.1f, 0.05f);
    glRotatef(25.0f + 12.0f * sin(fanRotation * 0.12f), 1.0f, 0.0f, 1.0f);
    drawPrism(0.12f, 0.04f, 0.25f, 0, false, 0.82f, 0.85f, 0.95f);
    glPopMatrix();
    // Right Arm
    glPushMatrix();
    glTranslatef(0.2f, 0.1f, 0.05f);
    glRotatef(-25.0f - 12.0f * sin(fanRotation * 0.12f), 1.0f, 0.0f, -1.0f);
    drawPrism(0.12f, 0.04f, 0.25f, 0, false, 0.82f, 0.85f, 0.95f);
    glPopMatrix();

    // Floating text label
    glColor3f(1.0f, 0.1f, 0.1f);
    glRasterPos3f(-0.25f, 0.52f, 0.0f);
    const char* label = (ghostState == 1) ? "RUN!" : "GHOST";
    for (const char* c = label; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, *c);
    }

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glPopMatrix();
}

// --- Draw Cabinet ---
static void drawCabinet(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 0.9f, z);

    // Outer wardrobe shell
    drawPrism(0.6f, 1.8f, 0.6f, texWood, hasWoodTex, 0.25f, 0.15f, 0.08f, 1.0f, 1.5f);

    // Metal details/handles
    glDisable(GL_LIGHTING);
    glColor3f(0.8f, 0.7f, 0.3f);
    glPushMatrix();
    glTranslatef(0.08f, 0.0f, 0.31f);
    glutSolidSphere(0.03f, 6, 6);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-0.08f, 0.0f, 0.31f);
    glutSolidSphere(0.03f, 6, 6);
    glPopMatrix();
    glEnable(GL_LIGHTING);

    glPopMatrix();
}

// --- Draw Flickering Wall Torch ---
static void drawWallTorch(float x, float y, float z) {
    // Torch bracket
    glPushMatrix();
    glTranslatef(x, y, z);
    drawPrism(0.06f, 0.3f, 0.06f, texStone, hasStoneTex, 0.15f, 0.15f, 0.15f);
    
    // Waving flame particles
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    float flameSize = 0.06f + 0.02f * sin(fanRotation * 0.2f + x + z);
    glColor4f(1.0f, 0.45f, 0.0f, 0.8f);
    glPushMatrix();
    glTranslatef(0.0f, 0.15f, 0.0f);
    glutSolidSphere(flameSize, 8, 8);
    
    // inner brighter core
    glColor4f(1.0f, 0.9f, 0.2f, 0.95f);
    glutSolidSphere(flameSize * 0.6f, 6, 6);
    glPopMatrix();

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glPopMatrix();
}

// --- Draw Chamber 4 ---
void drawChamber4() {
    float r = 6.0f;
    float h = 5.0f;

    // 1. Draw Octagonal Room Floors & Ceilings (Darkened for haunted look)
    glEnable(GL_TEXTURE_2D);
    if (hasGroundTex) {
        glBindTexture(GL_TEXTURE_2D, texGround);
        glColor3f(1.0f, 1.0f, 1.0f);
    } else {
        glBindTexture(GL_TEXTURE_2D, texStone);
        glColor3f(1.0f, 1.0f, 1.0f);
    }

    GLfloat floorAmb[]  = { 0.25f, 0.25f, 0.25f, 1.0f }; // Slightly darker ambient for horror look
    GLfloat floorDiff[] = { 0.5f,  0.5f,  0.5f,  1.0f };
    GLfloat floorSpec[] = { 0.0f,  0.0f,  0.0f,  1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT,  floorAmb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,  floorDiff);
    glMaterialfv(GL_FRONT, GL_SPECULAR, floorSpec);

    glBegin(GL_POLYGON);
    glNormal3f(0.0f, 1.0f, 0.0f);
    for (int i = 0; i < 8; ++i) {
        float angle = i * 45.0f * PI / 180.0f;
        float x = r * cos(angle);
        float z = r * sin(angle);
        glTexCoord2f(x * 0.5f, z * 0.5f);
        glVertex3f(x, 0.0f, z);
    }
    glEnd();

    // Ceiling (keep stone)
    glBindTexture(GL_TEXTURE_2D, texStone);

    GLfloat ceilAmb[]  = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat ceilDiff[] = { 0.4f, 0.4f, 0.4f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT,  ceilAmb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,  ceilDiff);

    glBegin(GL_POLYGON);
    glNormal3f(0.0f, -1.0f, 0.0f);
    for (int i = 0; i < 8; ++i) {
        float angle = i * 45.0f * PI / 180.0f;
        float x = r * cos(angle);
        float z = r * sin(angle);
        glTexCoord2f(x * 0.2f, z * 0.2f);
        glVertex3f(x, h, z);
    }
    glEnd();

    // Mossy dark green wall tinting for haunted look
    glColor3f(0.18f, 0.24f, 0.18f);
    for (int i = 0; i < 8; ++i) {
        float angle1 = i * 45.0f * PI / 180.0f;
        float angle2 = (i + 1) * 45.0f * PI / 180.0f;
        float x1 = r * cos(angle1), z1 = r * sin(angle1);
        float x2 = r * cos(angle2), z2 = r * sin(angle2);

        float mx = (x1 + x2) / 2.0f, mz = (z1 + z2) / 2.0f;
        float len = sqrt(mx*mx + mz*mz);
        float nx = -mx / len, nz = -mz / len;

        glBegin(GL_QUADS);
        glNormal3f(nx, 0.0f, nz);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(x1, 0.0f, z1);
        glTexCoord2f(2.0f, 0.0f); glVertex3f(x2, 0.0f, z2);
        glTexCoord2f(2.0f, 2.0f); glVertex3f(x2, h,    z2);
        glTexCoord2f(0.0f, 2.0f); glVertex3f(x1, h,    z1);
        glEnd();
    }
    glDisable(GL_TEXTURE_2D);

    // 2. Draw Partition Walls (Moldy green-grey tint)
    // Northwest Room 1
    glPushMatrix();
    glTranslatef(-3.75f, 1.25f, -2.0f);
    drawPrism(3.5f, 2.5f, 0.2f, texStone, hasStoneTex, 0.2f, 0.25f, 0.2f, 1.5f, 1.0f); // Wall H
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-2.0f, 1.25f, -4.35f);
    drawPrism(0.2f, 2.5f, 2.3f, texStone, hasStoneTex, 0.2f, 0.25f, 0.2f, 1.0f, 1.0f); // Wall V
    glPopMatrix();

    // Northeast Room 2
    glPushMatrix();
    glTranslatef(3.75f, 1.25f, -2.0f);
    drawPrism(3.5f, 2.5f, 0.2f, texStone, hasStoneTex, 0.2f, 0.25f, 0.2f, 1.5f, 1.0f); // Wall H
    glPopMatrix();
    glPushMatrix();
    glTranslatef(2.0f, 1.25f, -4.35f);
    drawPrism(0.2f, 2.5f, 2.3f, texStone, hasStoneTex, 0.2f, 0.25f, 0.2f, 1.0f, 1.0f); // Wall V
    glPopMatrix();

    // Southwest Room 3
    glPushMatrix();
    glTranslatef(-3.75f, 1.25f, 2.0f);
    drawPrism(3.5f, 2.5f, 0.2f, texStone, hasStoneTex, 0.2f, 0.25f, 0.2f, 1.5f, 1.0f); // Wall H
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-2.0f, 1.25f, 4.35f);
    drawPrism(0.2f, 2.5f, 2.3f, texStone, hasStoneTex, 0.2f, 0.25f, 0.2f, 1.0f, 1.0f); // Wall V
    glPopMatrix();

    // Southeast Room 4
    glPushMatrix();
    glTranslatef(3.75f, 1.25f, 2.0f);
    drawPrism(3.5f, 2.5f, 0.2f, texStone, hasStoneTex, 0.2f, 0.25f, 0.2f, 1.5f, 1.0f); // Wall H
    glPopMatrix();
    glPushMatrix();
    glTranslatef(2.0f, 1.25f, 4.35f);
    drawPrism(0.2f, 2.5f, 2.3f, texStone, hasStoneTex, 0.2f, 0.25f, 0.2f, 1.0f, 1.0f); // Wall V
    glPopMatrix();

    // 3. Draw Hiding Cabinets
    for (int i = 0; i < 4; ++i) {
        drawCabinet(cabinetPos[i][0], cabinetPos[i][1]);
    }

    // 4. Draw Collectible Gems
    float gemColor[4][3] = {
        {0.1f, 0.5f, 1.0f}, // NW: Blue
        {0.1f, 1.0f, 0.3f}, // NE: Green
        {1.0f, 0.5f, 0.1f}, // SW: Orange
        {0.8f, 0.1f, 1.0f}  // SE: Purple
    };
    for (int i = 0; i < 4; ++i) {
        if (!itemCollected[i]) {
            glPushMatrix();
            glTranslatef(itemPos[i][0], itemPos[i][1] + 0.05f * sin(fanRotation * 3.0f * PI / 180.0f), itemPos[i][2]);
            glRotatef(fanRotation * 2.0f, 0.0f, 1.0f, 0.0f);
            glDisable(GL_LIGHTING);
            glColor3f(gemColor[i][0], gemColor[i][1], gemColor[i][2]);
            glutSolidSphere(0.12f, 8, 8);
            glEnable(GL_LIGHTING);
            glPopMatrix();
        }
    }

    // 5. Pedestal (Key Box) - Moved to SW Room 3 Corner
    glPushMatrix();
    glTranslatef(pedestalX, 0.25f, pedestalZ);
    drawPrism(0.8f, 0.5f, 0.8f, texBox, hasBoxTex, 0.45f, 0.45f, 0.45f, 1.0f, 1.5f);
    glPopMatrix();

    // Pedestal Lid (slides open when solved)
    glPushMatrix();
    glTranslatef(pedestalX, 0.52f, pedestalZ - 0.4f - ch4PedestalOpenProgress * 0.5f);
    drawPrism(0.82f, 0.06f, 0.82f, texBox, hasBoxTex, 0.55f, 0.55f, 0.55f, 1.0f, 1.0f);
    glPopMatrix();

    // Final key inside key box
    if (ch4Solved && !hasCh4Key) {
        glPushMatrix();
        glTranslatef(pedestalX, 0.75f, pedestalZ);
        glRotatef(fanRotation, 0.0f, 1.0f, 0.0f);
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 0.85f, 0.1f);
        glPushMatrix(); glTranslatef(0.0f, 0.15f, 0.0f); glutSolidTorus(0.03f, 0.08f, 6, 8); glPopMatrix();
        glPushMatrix(); glTranslatef(0.0f, -0.05f, 0.0f); drawPrism(0.04f, 0.3f, 0.04f, 0, false, 1.0f, 0.85f, 0.1f); glPopMatrix();
        glEnable(GL_LIGHTING);
        glPopMatrix();
    }

    // 6. Patrolling Ghost & Particles
    drawGhost();
    drawGhostParticles();

    // 7. Wall Torches (Flickering light sources)
    drawWallTorch(0.0f, 1.8f, -5.8f);
    drawWallTorch(0.0f, 1.8f, 5.75f);
    drawWallTorch(-5.8f, 1.8f, 0.0f);
    drawWallTorch(5.75f, 1.8f, 0.0f);

    // 8. Glowing Floor Runes (center of main corridor)
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glColor4f(0.8f, 0.05f, 0.05f, 0.35f + 0.1f * sin(fanRotation * 0.06f));
    glPushMatrix();
    glTranslatef(0.0f, 0.015f, 0.0f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glutWireTorus(0.02f, 1.4f, 4, 18);
    glutWireTorus(0.02f, 0.9f, 4, 12);
    glPopMatrix();
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);

    // 9. Single Exit Door (Golden Door at center)
    glPushMatrix();
    glTranslatef(0.0f, 1.05f, -5.2f);
    drawPrism(1.1f, 2.2f, 0.08f, texWood, hasWoodTex, 0.9f, 0.75f, 0.1f, 1.0f, 3.0f);
    glTranslatef(0.0f, 0.0f, 0.041f);
    drawDoorText("EXIT");
    glPopMatrix();
}

// --- Chamber 4 Lighting ---
void setupChamber4Lighting() {
    // Creepy purple-dark atmosphere
    glClearColor(0.02f, 0.01f, 0.04f, 1.0f);
    GLfloat fogC[4] = { 0.02f, 0.01f, 0.04f, 1.0f };
    glFogfv(GL_FOG_COLOR, fogC);
    glFogf(GL_FOG_DENSITY, 0.07f);

    GLfloat ambientLevel[] = { 0.06f, 0.04f, 0.09f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLevel);

    // FLASHLIGHT (GL_LIGHT0): Follows the camera (player)
    GLfloat lightPos0[] = { camX, camY + 0.2f, camZ, 1.0f };
    GLfloat lightDir0[] = { (float)(-sin(rotY * PI / 180.0f)), -0.2f, (float)(-cos(rotY * PI / 180.0f)) };
    
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);
    glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, lightDir0);
    glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 35.0f); // flashlight beam
    glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 15.0f);

    GLfloat fColor[] = { 1.0f, 0.95f, 0.8f, 1.0f }; // soft white-yellow light
    glLightfv(GL_LIGHT0, GL_DIFFUSE, fColor);
    glLightfv(GL_LIGHT0, GL_SPECULAR, fColor);
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.2f);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.15f);

    // GHOST LIGHT (GL_LIGHT1): Red glowing light following the ghost
    GLfloat lightPos1[] = { ghostX, ghostY, ghostZ, 1.0f };
    glEnable(GL_LIGHT1);
    glLightfv(GL_LIGHT1, GL_POSITION, lightPos1);
    
    GLfloat gColor[] = { 1.0f, 0.1f, 0.1f, 1.0f }; // red light
    if (ghostState == 0) {
        gColor[0] = 0.7f; gColor[2] = 0.9f; // purple-magenta light when patrolling
    }
    glLightfv(GL_LIGHT1, GL_DIFFUSE, gColor);
    glLightfv(GL_LIGHT1, GL_SPECULAR, gColor);
    glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 0.5f);
    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.4f);
    glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.15f);

    glDisable(GL_LIGHT2);
    glDisable(GL_LIGHT3);
}

// --- Chamber 4 Update Logic ---
void updateChamber4() {
    if (fadeAlpha > 0.0f) { fadeAlpha -= 0.04f; if (fadeAlpha < 0.0f) fadeAlpha = 0.0f; }

    fanRotation += 1.2f;

    // Update timers
    if (HUDFeedbackTimer > 0.0f) HUDFeedbackTimer -= 0.016f;
    if (caughtRedFlash > 0.0f) caughtRedFlash -= 0.02f;

    // 1. Ghost particles update
    updateGhostParticles();

    // 2. Proximity checks for Cabinets (Hiding Status)
    bool nearCab = false;
    for (int i = 0; i < 4; ++i) {
        float dx = camX - cabinetPos[i][0];
        float dz = camZ - cabinetPos[i][1];
        if (sqrt(dx*dx + dz*dz) < 0.7f) {
            nearCab = true;
            break;
        }
    }
    isPlayerHidden = nearCab;

    // 3. Proximity check for Gems (Collection)
    int collectedCount = 0;
    for (int i = 0; i < 4; ++i) {
        if (!itemCollected[i]) {
            float dx = camX - itemPos[i][0];
            float dz = camZ - itemPos[i][2];
            if (sqrt(dx*dx + dz*dz) < 0.7f) {
                itemCollected[i] = true;
                ch4RumbleTimer = 0.2f;
                HUDFeedbackTimer = 2.0f;
                HUDFeedbackMsg = "RELIC GEM FRAGMENT COLLECTED!";
            }
        }
        if (itemCollected[i]) collectedCount++;
    }

    if (collectedCount == 4) {
        if (!ch4Solved) {
            ch4Solved = true;
            HUDFeedbackTimer = 3.5f;
            HUDFeedbackMsg = "ALL GEM FRAGMENTS COLLECTED! PEDESTAL LID IS OPENING!";
        }
    }

    if (ch4Solved && ch4PedestalOpenProgress < 1.0f) {
        ch4PedestalOpenProgress += 0.02f;
        if (ch4PedestalOpenProgress > 1.0f) ch4PedestalOpenProgress = 1.0f;
    }

    // 4. Ghost AI State Machine
    float dx = camX - ghostX;
    float dz = camZ - ghostZ;
    float distToPlayer = sqrt(dx*dx + dz*dz);

    if (isPlayerHidden) {
        // Lost chase if player hides
        ghostState = 0;
    } else {
        if (ghostState == 0) {
            // Patrol: check if player enters detection range
            if (distToPlayer < 2.0f) {
                ghostState = 1; // Chase Mode!
                ch4RumbleTimer = 0.35f;
                HUDFeedbackTimer = 2.0f;
                HUDFeedbackMsg = "THE GHOST HAS DETECTED YOU! RUN!";
            }
        } else {
            // Chase: check if player escaped or got caught
            if (distToPlayer > 4.5f) {
                ghostState = 0; // return to patrol
            } else if (distToPlayer < 0.6f) {
                // Caught! Jumpscare trigger
                caughtRedFlash = 1.0f;
                ch4RumbleTimer = 1.0f;
                camX = 0.0f; camY = 1.0f; camZ = 4.8f;
                rotX = 10.0f; rotY = 0.0f;
                resetChamber4();
                HUDFeedbackTimer = 3.0f;
                HUDFeedbackMsg = "YOU WERE CAUGHT! RESETTING CHAMBER...";
                return;
            }
        }
    }

    // 5. Ghost Movement
    if (ghostState == 0) {
        // Patrol movement between waypoints
        float tx = ghostWaypoints[currentWaypointIndex][0];
        float tz = ghostWaypoints[currentWaypointIndex][1];
        float gdx = tx - ghostX;
        float gdz = tz - ghostZ;
        float glen = sqrt(gdx*gdx + gdz*gdz);
        if (glen < 0.15f) {
            // Advance to next waypoint index
            static const int sequence[] = {0, 1, 0, 2, 0, 3, 0, 4};
            static int seqIdx = 0;
            seqIdx = (seqIdx + 1) % 8;
            currentWaypointIndex = sequence[seqIdx];
        } else {
            ghostX += (gdx / glen) * 0.024f;
            ghostZ += (gdz / glen) * 0.024f;
        }
    } else {
        // Chase movement directly towards player
        if (distToPlayer > 0.0f) {
            ghostX += (dx / distToPlayer) * 0.052f; // Fast chase
            ghostZ += (dz / distToPlayer) * 0.052f;
        }
    }

    // 6. Player Movement & Keyboard Checks
    const float PLAYER_SPEED = 0.055f;
    const float TURN_SPEED = 1.8f;

    if (keyStates['a'] || keyStates['A'] || specialKeyStates[GLUT_KEY_LEFT]) rotY += TURN_SPEED;
    if (keyStates['d'] || keyStates['D'] || specialKeyStates[GLUT_KEY_RIGHT]) rotY -= TURN_SPEED;

    float moveX = 0.0f, moveZ = 0.0f;
    bool playerMoving = false;
    if (keyStates['w'] || keyStates['W'] || specialKeyStates[GLUT_KEY_UP]) {
        float rad = rotY * PI / 180.0f;
        moveX = -sin(rad) * PLAYER_SPEED;
        moveZ = -cos(rad) * PLAYER_SPEED;
        playerMoving = true;
    }
    if (keyStates['s'] || keyStates['S'] || specialKeyStates[GLUT_KEY_DOWN]) {
        float rad = rotY * PI / 180.0f;
        moveX = sin(rad) * PLAYER_SPEED;
        moveZ = cos(rad) * PLAYER_SPEED;
        playerMoving = true;
    }

    if (playerMoving) {
        float newX = camX + moveX;
        float newZ = camZ + moveZ;
        if (!checkCh4Collision(newX, newZ)) {
            camX = newX;
            camZ = newZ;
        } else if (!checkCh4Collision(camX, newZ)) {
            camZ = newZ;
        } else if (!checkCh4Collision(newX, camZ)) {
            camX = newX;
        }
    }

    // 7. Door transition logic
    if (hasCh4Key && camZ < -4.5f) {
        // Single Exit Door at center (Success / Transition to Outside Scenario)
        if (fabs(camX - 0.0f) < 0.6f) {
            currentGameState = STATE_OUTSIDE_SCENARIO;
            camX = 0.0f; camY = 1.0f; camZ = 5.0f;
            rotX = 5.0f; rotY = 0.0f;
            fadeAlpha = 1.0f;
        }
    }

    if (isCh4FallingInTrap) {
        ch4TrapFallY += 0.12f;
        ch4TrapFade += 0.02f;
        if (ch4TrapFade >= 1.0f) {
            camX = 0.0f; camY = 1.0f; camZ = 4.8f;
            rotX = 10.0f; rotY = 0.0f;
            isCh4FallingInTrap = false;
            ch4TrapFallY = 0.0f;
            ch4TrapFade = 0.0f;
            resetChamber4();
        }
    }
}

// --- Chamber 4 HUD ---
void drawChamber4HUD() {
    char info[128];
    glColor3f(0.85f, 0.15f, 0.15f);
    renderBitmapString(20.0f, WINDOW_HEIGHT - 35.0f, GLUT_BITMAP_HELVETICA_12, "CHAMBER 4: GHOST ESCAPE HALL");

    glColor3f(0.8f, 0.8f, 0.8f);
    renderBitmapString(20.0f, WINDOW_HEIGHT - 55.0f, GLUT_BITMAP_HELVETICA_10, "Goal: Collect 4 gem fragments, locate the corner Key Box, unlock it, and escape.");
    renderBitmapString(20.0f, WINDOW_HEIGHT - 72.0f, GLUT_BITMAP_HELVETICA_10, "Stealth: Standing near tall Cabinets hides you. The ghost cannot see you when hidden.");

    // Count collected gems
    int count = 0;
    for (int i = 0; i < 4; ++i) if (itemCollected[i]) count++;

    sprintf(info, "Gems Collected: %d / 4", count);
    glColor3f(0.1f, 0.9f, 0.9f);
    renderBitmapString(20.0f, WINDOW_HEIGHT - 95.0f, GLUT_BITMAP_HELVETICA_12, info);

    // Hiding State Info
    if (isPlayerHidden) {
        glColor3f(0.1f, 0.9f, 0.1f);
        renderBitmapString(20.0f, WINDOW_HEIGHT - 115.0f, GLUT_BITMAP_HELVETICA_12, "[STATUS: HIDDEN IN CABINET]");
    } else {
        glColor3f(1.0f, 0.6f, 0.1f);
        renderBitmapString(20.0f, WINDOW_HEIGHT - 115.0f, GLUT_BITMAP_HELVETICA_12, "[STATUS: EXPOSED]");
    }

    // Ghost status info
    if (ghostState == 1) {
        glColor3f(1.0f, 0.0f, 0.0f);
        renderBitmapString(20.0f, WINDOW_HEIGHT - 135.0f, GLUT_BITMAP_HELVETICA_12, "[ALERT: GHOST IS CHASING YOU!]");
    } else {
        glColor3f(0.6f, 0.3f, 0.8f);
        renderBitmapString(20.0f, WINDOW_HEIGHT - 135.0f, GLUT_BITMAP_HELVETICA_10, "[Ghost Status: Patrolling]");
    }

    // Dynamic HUD Feedback Message
    if (HUDFeedbackTimer > 0.0f) {
        glColor3f(1.0f, 1.0f, 0.1f);
        renderBitmapString(WINDOW_WIDTH / 2.0f - 150.0f, WINDOW_HEIGHT / 2.0f + 180.0f, GLUT_BITMAP_HELVETICA_12, HUDFeedbackMsg);
    }

    // Lock status message
    if (hasCh4Key) {
        glColor3f(1.0f, 0.85f, 0.1f);
        renderBitmapString(WINDOW_WIDTH / 2.0f - 180.0f, WINDOW_HEIGHT / 2.0f + 140.0f, GLUT_BITMAP_HELVETICA_12, "Pedestal Unlocked! Escape through the Golden Door at the center.");
    }

    // Jumpscare overlay (Caught Red Flash)
    if (caughtRedFlash > 0.0f) {
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 0.0f, 0.0f, caughtRedFlash);
        glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(WINDOW_WIDTH, 0);
        glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
        glVertex2f(0, WINDOW_HEIGHT);
        glEnd();
        glDisable(GL_BLEND);

        glColor3f(1.0f, 1.0f, 1.0f);
        renderBitmapString(WINDOW_WIDTH / 2.0f - 140.0f, WINDOW_HEIGHT / 2.0f, GLUT_BITMAP_HELVETICA_18, "CAUGHT BY THE GHOST!");

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
    }
}

// --- Reset Chamber 4 ---
void resetChamber4() {
    ch4Solved = false;
    hasCh4Key = false;
    isCh4FallingInTrap = false;
    ch4TrapFallY = 0.0f;
    ch4TrapFade = 0.0f;
    ch4RumbleTimer = 0.0f;
    ch4PedestalOpenProgress = 0.0f;

    ghostX = 0.0f;
    ghostZ = 0.0f;
    ghostState = 0;
    currentWaypointIndex = 0;

    for (int i = 0; i < 4; ++i) {
        itemCollected[i] = false;
    }

    initGhostParticleSystem();
}

// --- Interaction (Pedestal Key Box) ---
void handleChamber4Interaction() {
    // Check if close to pedestal in the corner (-4.5, 4.5)
    float dx = camX - pedestalX;
    float dz = camZ - pedestalZ;
    float distToPed = sqrt(dx*dx + dz*dz);
    if (distToPed < 1.0f) {
        int count = 0;
        for (int i = 0; i < 4; ++i) {
            if (itemCollected[i]) count++;
        }

        if (count == 4) {
            if (!hasCh4Key) {
                hasCh4Key = true;
                ch4RumbleTimer = 0.5f;
                HUDFeedbackTimer = 3.5f;
                HUDFeedbackMsg = "GOLDEN KEY ACQUIRED! ESCAPE THROUGH THE CENTER DOOR!";
                std::cout << "Chamber 4 Key Acquired! Exit door unlocked." << std::endl;
            } else {
                HUDFeedbackTimer = 2.0f;
                HUDFeedbackMsg = "YOU ALREADY HAVE THE GOLDEN KEY!";
            }
        } else {
            HUDFeedbackTimer = 2.0f;
            HUDFeedbackMsg = "THE KEY BOX IS LOCKED! FIND ALL 4 GEM FRAGMENTS FIRST!";
        }
    }
}
