#include <GL/glut.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include "chamber1.h"
#include "chamber2.h"
#include "chamber3.h"
#include "chamber4.h"
#include "outside.h"

/*
 * =========================================================================
 *                   THE DUNGEON QUEST - MAIN GAME ENGINE
 * =========================================================================
 * This file acts as the primary game controller, coordinating the game states
 * (GameState enum) and routing logic (render, update, and inputs) to the
 * respective chamber modules:
 *
 *   1. STATE_OUTSIDE & STATE_ENTERING (dungeon_game.cpp):
 *      - Initial entrance gates scenario. Uses local sparks particles & torch flicker.
 *
 *   2. STATE_CHAMBER_1 (chamber1.cpp):
 *      - Chamber of Sequence: Players must walk up to statues and select them 
 *        in the correct sequence order (Wolf -> Lion -> Eagle -> Snake).
 *
 *   3. STATE_CHAMBER_2 (chamber2.cpp):
 *      - Chamber of Rings: Alignment puzzle where concentric rings are rotated 
 *        to match specific orientations.
 *
 *   4. STATE_CHAMBER_3 (chamber3.cpp):
 *      - Chamber of Dragon Illusion: Light/Dark mode alignment puzzle. Turn off 
 *        lights, stand on solution coordinates, and align a dragon relic hologram.
 *
 *   5. STATE_CHAMBER_4 (chamber4.cpp):
 *      - Chamber 4: Ghost Escape Hall (Horror Stealth): Sneak around stone partition 
 *        walls to collect 4 colored gem fragments, hide in corner cabinets, 
 *        retrieve the key from the center pedestal, and escape the patrolling ghost.
 *
 *   6. STATE_OUTSIDE_SCENARIO (outside.cpp):
 *      - Red text message stating AI is replacing engineers, prompt to press R to return.
 *
 *   7. STATE_VICTORY (dungeon_game.cpp):
 *      - Final escape screen showing completion details.
 * =========================================================================
 */
GameState currentGameState = STATE_OUTSIDE;

// --- Textures ---
GLuint texStone = 0;
GLuint texWood = 0;
GLuint texGround = 0;
GLuint texSky = 0;
GLuint texWolf = 0;
GLuint texLion = 0;
GLuint texSnake = 0;
GLuint texEagle = 0;
GLuint texBox = 0;
GLuint texDragon = 0;

bool hasStoneTex = false;
bool hasWoodTex = false;
bool hasGroundTex = false;
bool hasSkyTex = false;
bool hasWolfTex = false;
bool hasLionTex = false;
bool hasSnakeTex = false;
bool hasEagleTex = false;
bool hasBoxTex = false;
bool hasDragonTex = false;

// --- Camera & Interaction States ---
float camZ = 5.0f;
float camY = 0.8f;
float camX = 0.0f;

float rotX = 12.0f;
float rotY = 0.0f;

// Transition Animations
float doorOpenAngle = 0.0f;
float fadeAlpha = 0.0f;
float zoomFov = 45.0f;
float transitionTimer = 0.0f;

// Mouse drag state
bool isDragging = false;
int startMouseX = 0;
int startMouseY = 0;

// --- Keyboard state array for smooth WASD movement ---
bool keyStates[256] = {false};
bool specialKeyStates[256] = {false};

// Chamber 1 variables are in chamber1.cpp

// --- Dynamic Lighting & Torches ---
float torchFlickerLeft = 1.0f;
float torchFlickerRight = 1.0f;

// --- Particle System for Torch Sparks ---
struct SparkParticle {
    float x, y, z;
    float vx, vy, vz;
    float r, g, b, alpha;
    float life;
    float size;
};

std::vector<SparkParticle> sparks;
const int MAX_SPARKS = 80;

void initSpark(SparkParticle &p, bool isLeftTorch) {
    float baseX = isLeftTorch ? -1.05f : 1.05f;
    float baseY = 0.95f;
    float baseZ = -0.7f;

    p.x = baseX + (((float)rand() / RAND_MAX) - 0.5f) * 0.05f;
    p.y = baseY + ((float)rand() / RAND_MAX) * 0.05f;
    p.z = baseZ + (((float)rand() / RAND_MAX) - 0.5f) * 0.05f;

    p.vx = (((float)rand() / RAND_MAX) - 0.5f) * 0.005f;
    p.vy = 0.005f + ((float)rand() / RAND_MAX) * 0.01f;
    p.vz = (((float)rand() / RAND_MAX) - 0.5f) * 0.005f;

    p.r = 1.0f;
    p.g = 0.3f + ((float)rand() / RAND_MAX) * 0.5f;
    p.b = 0.0f;
    p.alpha = 0.6f + ((float)rand() / RAND_MAX) * 0.4f;

    p.life = 0.3f + ((float)rand() / RAND_MAX) * 0.7f;
    p.size = 2.0f + ((float)rand() / RAND_MAX) * 3.0f;
}

void initSparksSystem() {
    sparks.resize(MAX_SPARKS);
    for (int i = 0; i < MAX_SPARKS; ++i) {
        initSpark(sparks[i], (i % 2 == 0));
        sparks[i].life = ((float)rand() / RAND_MAX);
    }
}

void updateSparks() {
    for (int i = 0; i < MAX_SPARKS; ++i) {
        sparks[i].x += sparks[i].vx;
        sparks[i].y += sparks[i].vy;
        sparks[i].z += sparks[i].vz;

        sparks[i].life -= 0.012f;
        sparks[i].alpha = sparks[i].life;

        if (sparks[i].life <= 0.0f) {
            initSpark(sparks[i], (i % 2 == 0));
        }
    }
}

void drawSparks() {
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    glBegin(GL_POINTS);
    for (const auto &p : sparks) {
        glPointSize(p.size);
        glColor4f(p.r, p.g, p.b, p.alpha);
        glVertex3f(p.x, p.y, p.z);
    }
    glEnd();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

// --- Texture Loader ---
GLuint loadTexture(const char* filename, bool &success) {
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);

    if (!data) {
        success = false;
        std::cout << "Texture missing: " << filename << " (Using color fallback)" << std::endl;
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
    gluBuild2DMipmaps(GL_TEXTURE_2D, format, width, height, format, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);
    success = true;
    std::cout << "Loaded texture: " << filename << std::endl;
    return textureID;
}

// --- Scene Rendering Functions ---

void drawDoorText(const char* text) {
    glDisable(GL_LIGHTING);
    glPushMatrix();
    float totalWidth = 0.0f;
    for (const char* c = text; *c != '\0'; c++) {
        totalWidth += glutStrokeWidth(GLUT_STROKE_MONO_ROMAN, *c);
    }
    float scale = 0.0018f;
    glTranslatef(-totalWidth * 0.5f * scale, -50.0f * scale, 0.0f);
    glScalef(scale, scale, scale);
    glLineWidth(3.0f);
    glColor3f(0.9f, 0.9f, 0.9f);
    for (const char* c = text; *c != '\0'; c++) {
        glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, *c);
    }
    glLineWidth(1.0f);
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

void drawPrism(float dx, float dy, float dz, GLuint texID, bool hasTex, float r, float g, float b, float texScaleS, float texScaleT) {
    if (hasTex) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texID);
        glColor3f(1.0f, 1.0f, 1.0f);
    } else {
        glDisable(GL_TEXTURE_2D);
        glColor3f(r, g, b);
    }

    // --- BRIGHTENED: raised ambient and diffuse so surfaces catch more light ---
    GLfloat matAmbient[]   = { r * 0.6f, g * 0.6f, b * 0.6f, 1.0f };
    GLfloat matDiffuse[]   = { r,        g,        b,        1.0f };
    if (hasTex) {
        matAmbient[0] = 0.7f; matAmbient[1] = 0.7f; matAmbient[2] = 0.7f;
        matDiffuse[0] = 1.0f; matDiffuse[1] = 1.0f; matDiffuse[2] = 1.0f;
    }
    GLfloat matSpecular[]  = { 0.15f, 0.15f, 0.15f, 1.0f };
    GLfloat matShininess[] = { 10.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT,   matAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   matDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  matSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);

    float hx = dx / 2.0f;
    float hy = dy / 2.0f;
    float hz = dz / 2.0f;

    glBegin(GL_QUADS);
    // Front Face
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0.0f,       0.0f);       glVertex3f(-hx, -hy,  hz);
    glTexCoord2f(texScaleS,  0.0f);       glVertex3f( hx, -hy,  hz);
    glTexCoord2f(texScaleS,  texScaleT);  glVertex3f( hx,  hy,  hz);
    glTexCoord2f(0.0f,       texScaleT);  glVertex3f(-hx,  hy,  hz);

    // Back Face
    glNormal3f(0.0f, 0.0f, -1.0f);
    glTexCoord2f(0.0f,       0.0f);       glVertex3f( hx, -hy, -hz);
    glTexCoord2f(texScaleS,  0.0f);       glVertex3f(-hx, -hy, -hz);
    glTexCoord2f(texScaleS,  texScaleT);  glVertex3f(-hx,  hy, -hz);
    glTexCoord2f(0.0f,       texScaleT);  glVertex3f( hx,  hy, -hz);

    // Left Face
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f,       0.0f);       glVertex3f(-hx, -hy, -hz);
    glTexCoord2f(texScaleS,  0.0f);       glVertex3f(-hx, -hy,  hz);
    glTexCoord2f(texScaleS,  texScaleT);  glVertex3f(-hx,  hy,  hz);
    glTexCoord2f(0.0f,       texScaleT);  glVertex3f(-hx,  hy, -hz);

    // Right Face
    glNormal3f(1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f,       0.0f);       glVertex3f( hx, -hy,  hz);
    glTexCoord2f(texScaleS,  0.0f);       glVertex3f( hx, -hy, -hz);
    glTexCoord2f(texScaleS,  texScaleT);  glVertex3f( hx,  hy, -hz);
    glTexCoord2f(0.0f,       texScaleT);  glVertex3f( hx,  hy,  hz);

    // Top Face
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f,       0.0f);       glVertex3f(-hx,  hy,  hz);
    glTexCoord2f(texScaleS,  0.0f);       glVertex3f( hx,  hy,  hz);
    glTexCoord2f(texScaleS,  texScaleT);  glVertex3f( hx,  hy, -hz);
    glTexCoord2f(0.0f,       texScaleT);  glVertex3f(-hx,  hy, -hz);

    // Bottom Face
    glNormal3f(0.0f, -1.0f, 0.0f);
    glTexCoord2f(0.0f,       0.0f);       glVertex3f(-hx, -hy, -hz);
    glTexCoord2f(texScaleS,  0.0f);       glVertex3f( hx, -hy, -hz);
    glTexCoord2f(texScaleS,  texScaleT);  glVertex3f( hx, -hy,  hz);
    glTexCoord2f(0.0f,       texScaleT);  glVertex3f(-hx, -hy,  hz);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

void drawGround() {
    float size   = 40.0f;
    float repeat = 20.0f;
    if (hasGroundTex) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texGround);
        glColor3f(1.0f, 1.0f, 1.0f);
    } else {
        glDisable(GL_TEXTURE_2D);
        glColor3f(0.25f, 0.28f, 0.20f); // Brighter mossy green fallback
    }

    // --- BRIGHTENED: raised ground material ---
    GLfloat matAmbient[]  = { 0.15f, 0.18f, 0.12f, 1.0f };
    GLfloat matDiffuse[]  = { 0.30f, 0.35f, 0.25f, 1.0f };
    if (hasGroundTex) {
        matAmbient[0] = 0.55f; matAmbient[1] = 0.55f; matAmbient[2] = 0.55f;
        matDiffuse[0] = 1.0f;  matDiffuse[1] = 1.0f;  matDiffuse[2] = 1.0f;
    }
    GLfloat matSpecular[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT,  matAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,  matDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f,   0.0f);   glVertex3f(-size, 0.0f,  size);
    glTexCoord2f(repeat, 0.0f);   glVertex3f( size, 0.0f,  size);
    glTexCoord2f(repeat, repeat); glVertex3f( size, 0.0f, -size);
    glTexCoord2f(0.0f,   repeat); glVertex3f(-size, 0.0f, -size);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

void drawSky() {
    glDisable(GL_LIGHTING);
    float size = 45.0f;
    if (hasSkyTex) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texSky);
        glColor3f(0.8f, 0.8f, 0.9f); // Slightly brighter sky tint
    } else {
        glDisable(GL_TEXTURE_2D);
    }

    // 1. Back Sky Quad
    glBegin(GL_QUADS);
    if (!hasSkyTex) glColor3f(0.65f, 0.85f, 1.0f); // Horizon daytime blue
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-size, -5.0f, -size);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( size, -5.0f, -size);
    if (!hasSkyTex) glColor3f(0.35f, 0.60f, 0.95f); // Upper daytime blue
    glTexCoord2f(1.0f, 1.0f); glVertex3f( size,  size, -size);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-size,  size, -size);
    glEnd();

    // 2. Left Sky Quad
    glBegin(GL_QUADS);
    if (!hasSkyTex) glColor3f(0.65f, 0.85f, 1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-size, -5.0f,  size);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-size, -5.0f, -size);
    if (!hasSkyTex) glColor3f(0.35f, 0.60f, 0.95f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-size,  size, -size);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-size,  size,  size);
    glEnd();

    // 3. Right Sky Quad
    glBegin(GL_QUADS);
    if (!hasSkyTex) glColor3f(0.65f, 0.85f, 1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( size, -5.0f, -size);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( size, -5.0f,  size);
    if (!hasSkyTex) glColor3f(0.35f, 0.60f, 0.95f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( size,  size,  size);
    glTexCoord2f(0.0f, 1.0f); glVertex3f( size,  size, -size);
    glEnd();

    glEnable(GL_LIGHTING);
}

void drawTorch(float x, float y, float z, float flickerIntensity) {
    glDisable(GL_TEXTURE_2D);

    GLfloat metalAmbient[]   = { 0.08f, 0.08f, 0.08f, 1.0f };
    GLfloat metalDiffuse[]   = { 0.20f, 0.20f, 0.20f, 1.0f };
    GLfloat metalSpecular[]  = { 0.8f,  0.8f,  0.8f,  1.0f };
    GLfloat metalShininess[] = { 90.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT,   metalAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   metalDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  metalSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, metalShininess);

    glColor3f(0.2f, 0.2f, 0.2f);
    glPushMatrix();
    glTranslatef(x, y - 0.15f, z);
    glRotatef(20.0f, 0.0f, 0.0f, x < 0 ? -1.0f : 1.0f);
    glScalef(0.03f, 0.3f, 0.03f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Flame core
    glColor4f(1.0f, 0.85f, 0.3f, 0.95f);
    glPushMatrix();
    glTranslatef(x, y, z);
    glutSolidSphere(0.05f * flickerIntensity, 8, 8);
    glPopMatrix();

    // Flame outer glow
    glColor4f(1.0f, 0.45f, 0.05f, 0.45f);
    glPushMatrix();
    glTranslatef(x, y + 0.02f, z);
    glutSolidSphere(0.12f * flickerIntensity, 10, 10);
    glPopMatrix();

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}
void drawDungeonEntrance() {

    // Right wall section
    glPushMatrix();
    glTranslatef(2.5f, 2.0f, -1.0f);
    drawPrism(3.0f, 4.0f, 0.8f, texStone, hasStoneTex, 0.45f, 0.45f, 0.45f, 3.0f, 4.0f);
    glPopMatrix();

    // Left wall section
    glPushMatrix();
    glTranslatef(-2.5f, 2.0f, -1.0f);
    drawPrism(3.0f, 4.0f, 0.8f, texStone, hasStoneTex, 0.45f, 0.45f, 0.45f, 3.0f, 4.0f);
    glPopMatrix();

    // Left Side Wall (going back)
    glPushMatrix();
    glTranslatef(-4.0f, 2.0f, -3.0f);
    drawPrism(0.8f, 4.0f, 4.0f, texStone, hasStoneTex, 0.45f, 0.45f, 0.45f, 4.0f, 4.0f);
    glPopMatrix();

    // Right Side Wall (going back)
    glPushMatrix();
    glTranslatef(4.0f, 2.0f, -3.0f);
    drawPrism(0.8f, 4.0f, 4.0f, texStone, hasStoneTex, 0.45f, 0.45f, 0.45f, 4.0f, 4.0f);
    glPopMatrix();

    // Back Wall
    glPushMatrix();
    glTranslatef(0.0f, 2.0f, -5.0f);
    drawPrism(8.8f, 4.0f, 0.8f, texStone, hasStoneTex, 0.45f, 0.45f, 0.45f, 8.0f, 4.0f);
    glPopMatrix();

    // Ceiling (Top Wall)
    glPushMatrix();
    glTranslatef(0.0f, 4.0f, -3.0f);
    drawPrism(8.8f, 0.8f, 4.8f, texStone, hasStoneTex, 0.4f, 0.4f, 0.4f, 8.0f, 4.0f);
    glPopMatrix();



    // Left Column
    glPushMatrix();
    glTranslatef(-0.9f, 1.15f, -0.7f);
    drawPrism(0.3f, 2.3f, 0.3f, texStone, hasStoneTex, 0.50f, 0.50f, 0.50f, 0.5f, 3.0f);
    glPopMatrix();

    // Right Column
    glPushMatrix();
    glTranslatef(0.9f, 1.15f, -0.7f);
    drawPrism(0.3f, 2.3f, 0.3f, texStone, hasStoneTex, 0.50f, 0.50f, 0.50f, 0.5f, 3.0f);
    glPopMatrix();

    // Arch Top Header
    glPushMatrix();
    glTranslatef(0.0f, 2.45f, -0.7f);
    drawPrism(2.1f, 0.35f, 0.4f, texStone, hasStoneTex, 0.45f, 0.45f, 0.45f, 3.0f, 0.5f);
    glPopMatrix();

    // Left Door
    glPushMatrix();
    glTranslatef(-0.75f, 1.15f, -0.75f);
    glRotatef(doorOpenAngle, 0.0f, -1.0f, 0.0f);
    glTranslatef(0.375f, 0.0f, 0.0f);
    drawPrism(0.75f, 2.1f, 0.08f, texWood, hasWoodTex, 0.45f, 0.28f, 0.12f, 1.0f, 3.0f);
    glPopMatrix();

    // Right Door
    glPushMatrix();
    glTranslatef(0.75f, 1.15f, -0.75f);
    glRotatef(doorOpenAngle, 0.0f, 1.0f, 0.0f);
    glTranslatef(-0.375f, 0.0f, 0.0f);
    drawPrism(0.75f, 2.1f, 0.08f, texWood, hasWoodTex, 0.45f, 0.28f, 0.12f, 1.0f, 3.0f);
    glPopMatrix();

    // Left Torch Pillar
    glPushMatrix();
    glTranslatef(-1.25f, 0.8f, -0.6f);
    drawPrism(0.25f, 1.6f, 0.25f, texStone, hasStoneTex, 0.42f, 0.42f, 0.42f, 0.4f, 2.0f);
    glPopMatrix();

    // Right Torch Pillar
    glPushMatrix();
    glTranslatef(1.25f, 0.8f, -0.6f);
    drawPrism(0.25f, 1.6f, 0.25f, texStone, hasStoneTex, 0.42f, 0.42f, 0.42f, 0.4f, 2.0f);
    glPopMatrix();

    drawTorch(-1.25f, 1.7f, -0.6f, torchFlickerLeft);
    drawTorch( 1.25f, 1.7f, -0.6f, torchFlickerRight);
}

// Chamber 1 drawing functions are in chamber1.cpp

void renderBitmapString(float x, float y, void *font, const char *string) {
    const char *c;
    glRasterPos2f(x, y);
    for (c = string; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
}

void drawHUD() {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Fade overlay
    if (fadeAlpha > 0.01f) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.0f, 0.0f, 0.0f, fadeAlpha);
        glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(WINDOW_WIDTH, 0);
        glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
        glVertex2f(0, WINDOW_HEIGHT);
        glEnd();
        glDisable(GL_BLEND);
    }

    if (currentGameState == STATE_OUTSIDE) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.03f, 0.03f, 0.06f, 0.8f);
        glBegin(GL_QUADS);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(WINDOW_WIDTH, 0.0f);
        glVertex2f(WINDOW_WIDTH, 80.0f);
        glVertex2f(0.0f, 80.0f);
        glEnd();
        glDisable(GL_BLEND);

        glColor3f(1.0f, 0.84f, 0.0f);
        renderBitmapString(30.0f, WINDOW_HEIGHT - 40.0f, GLUT_BITMAP_HELVETICA_18, "THE DUNGEON QUEST");

        glColor3f(0.9f, 0.9f, 0.9f);
        renderBitmapString(30.0f, 45.0f, GLUT_BITMAP_HELVETICA_12, "You stand outside the ominous iron-locked Dungeon Gates.");
        glColor3f(0.0f, 0.9f, 1.0f);
        renderBitmapString(30.0f, 20.0f, GLUT_BITMAP_HELVETICA_12, ">> PRESS [E] or [ENTER] TO APPROACH AND ENTER THE DUNGEON");

        glColor3f(0.7f, 0.7f, 0.7f);
        renderBitmapString(WINDOW_WIDTH - 380.0f, 45.0f, GLUT_BITMAP_HELVETICA_10, "CONTROLS:");
        renderBitmapString(WINDOW_WIDTH - 380.0f, 30.0f, GLUT_BITMAP_HELVETICA_10, "Mouse Drag / Arrows : Look around");
        renderBitmapString(WINDOW_WIDTH - 380.0f, 15.0f, GLUT_BITMAP_HELVETICA_10, "Q / ESC : Exit Game");

    } else if (currentGameState == STATE_ENTERING) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.0f, 0.0f, 0.0f, 0.4f);
        glBegin(GL_QUADS);
        glVertex2f(0, WINDOW_HEIGHT/2 - 40);
        glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT/2 - 40);
        glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT/2 + 40);
        glVertex2f(0, WINDOW_HEIGHT/2 + 40);
        glEnd();
        glDisable(GL_BLEND);

        glColor3f(1.0f, 0.2f, 0.2f);
        renderBitmapString(WINDOW_WIDTH/2 - 120.0f, WINDOW_HEIGHT/2 - 8.0f, GLUT_BITMAP_HELVETICA_18, "THE GATES ARE OPENING...");

    } else if (currentGameState == STATE_CHAMBER_1) {
        drawChamber1HUD();
    } else if (currentGameState == STATE_CHAMBER_2) {
        drawChamber2HUD();
    } else if (currentGameState == STATE_CHAMBER_3) {
        drawChamber3HUD();
    } else if (currentGameState == STATE_CHAMBER_4) {
        drawChamber4HUD();
    } else if (currentGameState == STATE_OUTSIDE_SCENARIO) {
        drawOutsideScenarioHUD();
    } else if (currentGameState == STATE_VICTORY) {
        // Dedicated victory screen
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.08f, 0.07f, 0.02f, 0.9f);
        glBegin(GL_QUADS);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(WINDOW_WIDTH, 0.0f);
        glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
        glVertex2f(0.0f, WINDOW_HEIGHT);
        glEnd();
        glDisable(GL_BLEND);

        glColor3f(1.0f, 0.85f, 0.1f);
        renderBitmapString(WINDOW_WIDTH / 2.0f - 180.0f, WINDOW_HEIGHT / 2.0f + 60.0f, GLUT_BITMAP_HELVETICA_18, "THE DUNGEON ESCAPED!");
        glColor3f(0.9f, 0.9f, 0.9f);
        renderBitmapString(WINDOW_WIDTH / 2.0f - 250.0f, WINDOW_HEIGHT / 2.0f + 10.0f, GLUT_BITMAP_HELVETICA_12, "You solved all the ancient puzzles and escaped the Chamber of Shadows.");
        renderBitmapString(WINDOW_WIDTH / 2.0f - 210.0f, WINDOW_HEIGHT / 2.0f - 20.0f, GLUT_BITMAP_HELVETICA_12, "Congratulations, Brave Adventurer! You completed the Quest!");
        glColor3f(0.5f, 0.8f, 0.5f);
        renderBitmapString(WINDOW_WIDTH / 2.0f - 120.0f, WINDOW_HEIGHT / 2.0f - 60.0f, GLUT_BITMAP_HELVETICA_10, "Press [R] to restart your journey.");
    }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

// --- Main Display Callback ---
void display() {
    // 1. Setup dynamic daylight (outside) vs dark aesthetic (inside)
    if (currentGameState == STATE_OUTSIDE || currentGameState == STATE_ENTERING) {
        glClearColor(0.55f, 0.75f, 0.95f, 1.0f); // Bright blue sky background
        
        GLfloat fogColor[4] = { 0.55f, 0.75f, 0.95f, 1.0f };
        glFogfv(GL_FOG_COLOR, fogColor);
        glFogf(GL_FOG_DENSITY, 0.01f); // Thin ambient daytime mist
        
        GLfloat globalAmbient[] = { 0.65f, 0.65f, 0.65f, 1.0f };
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
        
        // Sun directional light
        GLfloat lightAmbient0[]  = { 0.55f, 0.55f, 0.50f, 1.0f };
        GLfloat lightDiffuse0[]  = { 0.95f, 0.95f, 0.85f, 1.0f };
        GLfloat lightSpecular0[] = { 0.40f, 0.40f, 0.35f, 1.0f };
        GLfloat lightPos0[]      = { 3.0f, 12.0f, 5.0f,  0.0f };
        glLightfv(GL_LIGHT0, GL_AMBIENT,  lightAmbient0);
        glLightfv(GL_LIGHT0, GL_DIFFUSE,  lightDiffuse0);
        glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular0);
        glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);
        glEnable(GL_LIGHT0);
        
        // Torch point lights
        GLfloat lightPosLeft[]    = { -1.25f, 1.7f, -0.6f, 1.0f };
        GLfloat lightColorLeft[]  = { 1.0f * torchFlickerLeft,  0.55f * torchFlickerLeft,  0.15f * torchFlickerLeft,  1.0f };
        glLightfv(GL_LIGHT1, GL_POSITION, lightPosLeft);
        glLightfv(GL_LIGHT1, GL_DIFFUSE,  lightColorLeft);
        glLightfv(GL_LIGHT1, GL_SPECULAR, lightColorLeft);
        glEnable(GL_LIGHT1);

        GLfloat lightPosRight[]   = { 1.25f, 1.7f, -0.6f, 1.0f };
        GLfloat lightColorRight[] = { 1.0f * torchFlickerRight, 0.55f * torchFlickerRight, 0.15f * torchFlickerRight, 1.0f };
        glLightfv(GL_LIGHT2, GL_POSITION, lightPosRight);
        glLightfv(GL_LIGHT2, GL_DIFFUSE,  lightColorRight);
        glLightfv(GL_LIGHT2, GL_SPECULAR, lightColorRight);
        glEnable(GL_LIGHT2);

        for (int light : {GL_LIGHT1, GL_LIGHT2}) {
            glLightf(light, GL_CONSTANT_ATTENUATION,  0.5f);
            glLightf(light, GL_LINEAR_ATTENUATION,    0.15f);
            glLightf(light, GL_QUADRATIC_ATTENUATION, 0.05f);
        }
    } else if (currentGameState == STATE_CHAMBER_1) {
        setupChamber1Lighting();
    } else if (currentGameState == STATE_CHAMBER_2) {
        setupChamber2Lighting();
    } else if (currentGameState == STATE_CHAMBER_3) {
        setupChamber3Lighting();
    } else if (currentGameState == STATE_CHAMBER_4) {
        setupChamber4Lighting();
    } else if (currentGameState == STATE_OUTSIDE_SCENARIO) {
        setupOutsideScenarioLighting();
    } else if (currentGameState == STATE_VICTORY) {
        GLfloat globalAmbient[] = { 0.15f, 0.15f, 0.15f, 1.0f };
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
        glDisable(GL_LIGHT0); glDisable(GL_LIGHT1); glDisable(GL_LIGHT2); glDisable(GL_LIGHT3);
    }

    // Dynamic projection setup (for zoom and glFrustum in Chamber 3)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (currentGameState == STATE_CHAMBER_3) {
        float aspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
        float fH = tan(zoomFov / 360.0f * PI) * 0.1f;
        float fW = fH * aspect;
        glFrustum(-fW, fW, -fH, fH, 0.1f, 100.0f);
    } else {
        gluPerspective(45.0f, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);
    }
    glMatrixMode(GL_MODELVIEW);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // 2. Camera View Setup
    if (currentGameState == STATE_CHAMBER_1 || currentGameState == STATE_CHAMBER_2 || 
        currentGameState == STATE_CHAMBER_3 || currentGameState == STATE_CHAMBER_4 || 
        currentGameState == STATE_VICTORY || currentGameState == STATE_OUTSIDE_SCENARIO) {
        float rumbleX = 0.0f;
        float rumbleY = 0.0f;
        float fallY = 0.0f;

        if (currentGameState == STATE_CHAMBER_1) {
            if (ch1RumbleTimer > 0.0f) {
                rumbleX = ((rand() % 100 / 100.0f) - 0.5f) * ch1RumbleTimer * 0.15f;
                rumbleY = ((rand() % 100 / 100.0f) - 0.5f) * ch1RumbleTimer * 0.15f;
            }
            fallY = trapFallY;
        } else if (currentGameState == STATE_CHAMBER_2) {
            if (ch2RumbleTimer > 0.0f) {
                rumbleX = ((rand() % 100 / 100.0f) - 0.5f) * ch2RumbleTimer * 0.15f;
                rumbleY = ((rand() % 100 / 100.0f) - 0.5f) * ch2RumbleTimer * 0.15f;
            }
            fallY = ch2TrapFallY;
        } else if (currentGameState == STATE_CHAMBER_3) {
            if (ch3RumbleTimer > 0.0f) {
                rumbleX = ((rand() % 100 / 100.0f) - 0.5f) * ch3RumbleTimer * 0.04f;
                rumbleY = ((rand() % 100 / 100.0f) - 0.5f) * ch3RumbleTimer * 0.04f;
            }
            fallY = ch3TrapFallY;
        } else if (currentGameState == STATE_CHAMBER_4) {
            if (ch4RumbleTimer > 0.0f) {
                rumbleX = ((rand() % 100 / 100.0f) - 0.5f) * ch4RumbleTimer * 0.04f;
                rumbleY = ((rand() % 100 / 100.0f) - 0.5f) * ch4RumbleTimer * 0.04f;
            }
            fallY = ch4TrapFallY;
        }

        float radY = rotY * PI / 180.0f;
        float radX = rotX * PI / 180.0f;

        // Calculate direction vector from Euler angles
        float dirX = -sin(radY) * cos(radX);
        float dirY = -sin(radX);
        float dirZ = -cos(radY) * cos(radX);

        gluLookAt(camX + rumbleX, camY + rumbleY - fallY, camZ,
                  camX + rumbleX + dirX, camY + rumbleY - fallY + dirY, camZ + dirZ,
                  0.0f, 1.0f, 0.0f);
    } else {
        // Outside scene camera
        gluLookAt(camX, camY, camZ,
                  0.0f, 0.6f, -1.0f,
                  0.0f, 1.0f, 0.0f);

        if (currentGameState == STATE_OUTSIDE) {
            glRotatef(rotX, 1.0f, 0.0f, 0.0f);
            glRotatef(rotY, 0.0f, 1.0f, 0.0f);
        }
    }

    // 3. Render Scene
    if (currentGameState == STATE_OUTSIDE || currentGameState == STATE_ENTERING) {
        // [SCENARIO: DUNGEON ENTRANCE]
        // Renders the initial outdoor gates, sky, torches, and spark particles defined here in dungeon_game.cpp.
        drawSky();
        drawGround();
        drawDungeonEntrance();
        drawSparks();
    } else if (currentGameState == STATE_CHAMBER_1) {
        // [SCENARIO: CHAMBER 1 - SEQUENCE PUZZLE]
        // Renders the statues, glowing sequence pillars, pedestal, and SUN/MOON exit doors from chamber1.cpp.
        drawChamber1();
    } else if (currentGameState == STATE_CHAMBER_2) {
        // [SCENARIO: CHAMBER 2 - CONCENTRIC RINGS]
        // Renders the 3 rotating concentric rings and the CLOCK/HOURGLASS exit doors from chamber2.cpp.
        drawChamber2();
    } else if (currentGameState == STATE_CHAMBER_3) {
        // [SCENARIO: CHAMBER 3 - DRAGON ILLUSION]
        // Renders the light switch lever, dragon hologram relic, target frame, and EYE/MIRROR exit doors from chamber3.cpp.
        drawChamber3();
    } else if (currentGameState == STATE_CHAMBER_4 || currentGameState == STATE_VICTORY) {
        // [SCENARIO: CHAMBER 4 - GHOST ESCAPE HALL]
        // Renders the maze walls, cabinets, 4 gems, patrolling/chasing tattered ghost, and exit door from chamber4.cpp.
        drawChamber4();
    } else if (currentGameState == STATE_OUTSIDE_SCENARIO) {
        // [SCENARIO: OUTSIDE GAME EXIT]
        // Renders the pitch-black exit scene defined in outside.cpp.
        drawOutsideScenario();
    }

    drawHUD();
    glutSwapBuffers();
}

// --- Animation Update & Timer ---
void update(int value) {
    torchFlickerLeft  = 0.85f + 0.15f * (rand() % 100 / 100.0f);
    torchFlickerRight = 0.85f + 0.15f * (rand() % 100 / 100.0f);

    updateSparks();

    if (currentGameState == STATE_ENTERING) {
        transitionTimer += 0.015f;

        if (transitionTimer < 1.0f) {
            doorOpenAngle = transitionTimer * 85.0f;
        }

        if (transitionTimer > 0.5f && transitionTimer < 2.0f) {
            float t = (transitionTimer - 0.5f) / 1.5f;
            camZ = 5.0f - (t * 5.5f);
            camY = 0.8f - (t * 0.2f);
        }

        if (transitionTimer > 1.3f && transitionTimer < 2.3f) {
            fadeAlpha = (transitionTimer - 1.3f);
            if (fadeAlpha > 1.0f) fadeAlpha = 1.0f;
        }

        if (transitionTimer >= 2.3f) {
            currentGameState = STATE_CHAMBER_1;
            camZ = 4.8f;
            camY = 1.0f;
            camX = 0.0f;
            rotX = 10.0f;
            rotY = 0.0f;
            transitionTimer = 0.0f;
        }
    }

    if (currentGameState == STATE_CHAMBER_1) {
        // [UPDATE: CHAMBER 1] Statue rotation, blackout timers, sequence checking, and trap animations.
        updateChamber1();
    } else if (currentGameState == STATE_CHAMBER_2) {
        // [UPDATE: CHAMBER 2] Decelerates ring rotation speed, checks key alignment, and runs trap animation.
        updateChamber2();
    } else if (currentGameState == STATE_CHAMBER_3) {
        // [UPDATE: CHAMBER 3] Rotates hologram relic, updates floating sparks particles, slides pedestal lid.
        updateChamber3();
    } else if (currentGameState == STATE_CHAMBER_4) {
        // [UPDATE: CHAMBER 4] Updates ghost AI pathfinding waypoints, check player exposure/hiding cabinets.
        updateChamber4();
    } else if (currentGameState == STATE_OUTSIDE_SCENARIO) {
        // [UPDATE: OUTSIDE EXIT] Updates camera movement fade transitions.
        updateOutsideScenario();
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

// --- Window Reshape ---
void reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (GLfloat)w / (GLfloat)h, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
}

// --- Keyboard ---
void keyboard(unsigned char key, int x, int y) {
    keyStates[key] = true;
    if (key >= 'A' && key <= 'Z') {
        keyStates[key - 'A' + 'a'] = true;
    } else if (key >= 'a' && key <= 'z') {
        keyStates[key - 'a' + 'A'] = true;
    }

    switch (key) {
        case 'q': case 'Q': case 27:
            exit(0);
            break;
        case 'e': case 'E': case 13:
            if (currentGameState == STATE_OUTSIDE) {
                currentGameState = STATE_ENTERING;
                transitionTimer = 0.0f;
            } else if (currentGameState == STATE_CHAMBER_1) {
                handleChamber1Interaction();
            } else if (currentGameState == STATE_CHAMBER_2) {
                handleChamber2Interaction();
            } else if (currentGameState == STATE_CHAMBER_3) {
                handleChamber3Interaction();
            } else if (currentGameState == STATE_CHAMBER_4) {
                handleChamber4Interaction();
            } else if (currentGameState == STATE_OUTSIDE_SCENARIO) {
                handleOutsideScenarioInteraction();
            }
            break;
        case 'l': case 'L':
            if (currentGameState == STATE_CHAMBER_3 && !ch3Solved && !isCh3FallingInTrap) {
                roomLight = !roomLight;
                ch3RumbleTimer = 0.1f;
                std::cout << "L key pressed! Room light toggle: " << (roomLight ? "ON" : "OFF") << std::endl;
            }
            break;
        case 'u': case 'U':
            if (currentGameState == STATE_CHAMBER_3) {
                showCh3SolutionGuide = !showCh3SolutionGuide;
                std::cout << "U key pressed! Solution guide toggle: " << (showCh3SolutionGuide ? "ACTIVE" : "INACTIVE") << std::endl;
            }
            break;
        case '[':
            if (currentGameState == STATE_CHAMBER_3) {
                zoomFov -= 2.0f;
                if (zoomFov < 15.0f) zoomFov = 15.0f;
            }
            break;
        case ']':
            if (currentGameState == STATE_CHAMBER_3) {
                zoomFov += 2.0f;
                if (zoomFov > 60.0f) zoomFov = 60.0f;
            }

            break;
        case 'k': case 'K':
            if (currentGameState == STATE_OUTSIDE) {
                currentGameState = STATE_ENTERING;
                transitionTimer = 0.0f;
                std::cout << "Admin Skip: Entering Dungeon" << std::endl;
            } else if (currentGameState == STATE_CHAMBER_1) {
                currentGameState = STATE_CHAMBER_2;
                camX = 0.0f; camY = 1.0f; camZ = 4.8f;
                rotX = 10.0f; rotY = 0.0f;
                fadeAlpha = 1.0f;
                std::cout << "Admin Skip: Chamber 1 -> Chamber 2" << std::endl;
            } else if (currentGameState == STATE_CHAMBER_2) {
                currentGameState = STATE_CHAMBER_3;
                camX = 0.0f; camY = 1.0f; camZ = 4.8f;
                rotX = 10.0f; rotY = 0.0f;
                fadeAlpha = 1.0f;
                std::cout << "Admin Skip: Chamber 2 -> Chamber 3" << std::endl;
            } else if (currentGameState == STATE_CHAMBER_3) {
                currentGameState = STATE_CHAMBER_4;
                camX = 0.0f; camY = 1.0f; camZ = 4.8f;
                rotX = 10.0f; rotY = 0.0f;
                fadeAlpha = 1.0f;
                std::cout << "Admin Skip: Chamber 3 -> Chamber 4" << std::endl;
            } else if (currentGameState == STATE_CHAMBER_4) {
                currentGameState = STATE_OUTSIDE_SCENARIO;
                camX = 0.0f; camY = 1.0f; camZ = 5.0f;
                rotX = 5.0f; rotY = 0.0f;
                fadeAlpha = 1.0f;
                std::cout << "Admin Skip: Chamber 4 -> Outside Valley Scenario" << std::endl;
            }
            break;
        case 'r': case 'R':
            if (currentGameState == STATE_CHAMBER_1 || currentGameState == STATE_CHAMBER_2 || 
                currentGameState == STATE_CHAMBER_3 || currentGameState == STATE_CHAMBER_4 || 
                currentGameState == STATE_VICTORY || currentGameState == STATE_OUTSIDE_SCENARIO) {
                currentGameState = STATE_OUTSIDE;
                camZ = 5.0f; camY = 0.8f; camX = 0.0f;
                rotX = 12.0f; rotY = 0.0f;
                doorOpenAngle = 0.0f;
                fadeAlpha = 0.0f;
                zoomFov = 45.0f;
                resetChamber1();
                resetChamber2();
                resetChamber3();
                resetChamber4();
            }
            break;
    }
}

void keyboardUp(unsigned char key, int x, int y) {
    keyStates[key] = false;
    if (key >= 'A' && key <= 'Z') {
        keyStates[key - 'A' + 'a'] = false;
    } else if (key >= 'a' && key <= 'z') {
        keyStates[key - 'a' + 'A'] = false;
    }
}

// --- Special Keys ---
void specialKeys(int key, int x, int y) {
    if (key >= 0 && key < 256) {
        specialKeyStates[key] = true;
    }
    if (currentGameState == STATE_OUTSIDE) {
        switch (key) {
            case GLUT_KEY_UP:
                rotX -= 2.0f;
                if (rotX < -5.0f) rotX = -5.0f;
                break;
            case GLUT_KEY_DOWN:
                rotX += 2.0f;
                if (rotX > 30.0f) rotX = 30.0f;
                break;
            case GLUT_KEY_LEFT:
                rotY -= 3.0f;
                if (rotY < -30.0f) rotY = -30.0f;
                break;
            case GLUT_KEY_RIGHT:
                rotY += 3.0f;
                if (rotY > 30.0f) rotY = 30.0f;
                break;
        }
    } else if (currentGameState == STATE_CHAMBER_1 || currentGameState == STATE_CHAMBER_2 || 
               currentGameState == STATE_CHAMBER_3 || currentGameState == STATE_CHAMBER_4 ||
               currentGameState == STATE_OUTSIDE_SCENARIO) {
        switch (key) {
            case GLUT_KEY_PAGE_UP:
                rotX -= 2.0f;
                if (rotX < -50.0f) rotX = -50.0f;
                break;
            case GLUT_KEY_PAGE_DOWN:
                rotX += 2.0f;
                if (rotX > 50.0f) rotX = 50.0f;
                break;
        }
    }
    glutPostRedisplay();
}

void specialKeysUp(int key, int x, int y) {
    if (key >= 0 && key < 256) {
        specialKeyStates[key] = false;
    }
}

// --- Mouse ---
void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            isDragging = true;
            startMouseX = x;
            startMouseY = y;
        } else if (state == GLUT_UP) {
            isDragging = false;
        }
    }
}

void motion(int x, int y) {
    if (isDragging) {
        if (currentGameState == STATE_OUTSIDE) {
            rotY += (x - startMouseX) * 0.4f;
            rotX += (y - startMouseY) * 0.4f;
            if (rotX < -5.0f)  rotX = -5.0f;
            if (rotX > 30.0f)  rotX = 30.0f;
            if (rotY < -30.0f) rotY = -30.0f; // Clamp rotation left
            if (rotY > 30.0f)  rotY = 30.0f;  // Clamp rotation right
        } else if (currentGameState == STATE_CHAMBER_1 || currentGameState == STATE_CHAMBER_2 || 
                   currentGameState == STATE_CHAMBER_3 || currentGameState == STATE_CHAMBER_4 ||
                   currentGameState == STATE_OUTSIDE_SCENARIO) {
            rotY += (x - startMouseX) * 0.4f;
            rotX += (y - startMouseY) * 0.4f;
            if (rotX < -50.0f) rotX = -50.0f; // Clamp look up
            if (rotX > 50.0f)  rotX = 50.0f;  // Clamp look down
        }
        startMouseX = x;
        startMouseY = y;
        glutPostRedisplay();
    }
}

// --- OpenGL Initialization ---
void initGL() {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_EXP);
    glHint(GL_FOG_HINT, GL_NICEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    // Cull back faces so the reverse side of walls/doors are invisible
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
}

// --- Main ---
int main(int argc, char** argv) {
    srand((unsigned int)time(NULL));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("The Dungeon Quest - Chapter Entrance");

    initGL();

    texStone  = loadTexture("image/stone.jpg",  hasStoneTex);
    texWood   = loadTexture("image/wood.jpg",   hasWoodTex);
    texGround = loadTexture("image/ground.jpg", hasGroundTex);
    texSky    = loadTexture("image/sky.jpg",    hasSkyTex);
    texWolf   = loadTexture("image/wolf.jpg",   hasWolfTex);
    texLion   = loadTexture("image/lion.jpg",   hasLionTex);
    texSnake  = loadTexture("image/snake.jpg",  hasSnakeTex);
    texEagle  = loadTexture("image/eagle.jpg",  hasEagleTex);
    texBox    = loadTexture("image/box.jpg",    hasBoxTex);
    texDragon = loadTexture("image/dragon.jpg", hasDragonTex);

    initSparksSystem();
    resetChamber1();
    resetChamber2();
    resetChamber3();
    resetChamber4();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialKeys);
    glutSpecialUpFunc(specialKeysUp);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutTimerFunc(16, update, 0);

    glutMainLoop();
    return 0;
}