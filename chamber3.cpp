#include "chamber3.h"
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <cstdio>

// --- Chamber 3 State Variables ---
bool roomLight = true;
bool ch3Solved = false;
bool hasCh3Key = false;
float ch3PedestalOpenProgress = 0.0f; // starts closed
bool isCh3FallingInTrap = false;
float ch3TrapFallY = 0.0f;
float ch3TrapFade = 0.0f;
float ch3BlackoutTimer = 0.0f;
float ch3RumbleTimer = 0.0f;
bool showCh3SolutionGuide = false;

// Target viewpoint coordinates
const float targetCx = 1.8f;
const float targetCy = 2.2f;
const float targetCz = 3.5f;
const float targetRx = 20.0f;
const float targetRy = 25.0f;

// --- Hologram Particle System ---
struct Ch3Particle {
    float x, y, z;
    float vx, vy, vz;
    float r, g, b, alpha;
    float life;
};
static std::vector<Ch3Particle> ch3Particles;
static const int CH3_MAX_PARTICLES = 80;

static void initCh3Particle(Ch3Particle &p) {
    float angle = ((float)rand() / RAND_MAX) * 2.0f * PI;
    float radius = 0.1f + ((float)rand() / RAND_MAX) * 0.3f;
    p.x = radius * cos(angle);
    p.y = 0.9f + ((float)rand() / RAND_MAX) * 0.2f;
    p.z = radius * sin(angle);

    p.vx = -sin(angle) * 0.005f + (((float)rand() / RAND_MAX) - 0.5f) * 0.002f;
    p.vy = 0.006f + ((float)rand() / RAND_MAX) * 0.006f; // Drift upwards
    p.vz = cos(angle) * 0.005f + (((float)rand() / RAND_MAX) - 0.5f) * 0.002f;

    p.r = 0.0f; 
    p.g = 0.75f + ((float)rand() / RAND_MAX) * 0.25f; 
    p.b = 1.0f; // Cyan-blue neon glow
    p.alpha = 0.4f + ((float)rand() / RAND_MAX) * 0.6f;
    p.life = 0.5f + ((float)rand() / RAND_MAX) * 1.0f;
}

static void initCh3ParticleSystem() {
    ch3Particles.resize(CH3_MAX_PARTICLES);
    for (int i = 0; i < CH3_MAX_PARTICLES; ++i) {
        initCh3Particle(ch3Particles[i]);
        ch3Particles[i].life = ((float)rand() / RAND_MAX); // disperse initial life
    }
}

static void updateCh3Particles() {
    for (int i = 0; i < CH3_MAX_PARTICLES; ++i) {
        ch3Particles[i].x += ch3Particles[i].vx;
        ch3Particles[i].y += ch3Particles[i].vy;
        ch3Particles[i].z += ch3Particles[i].vz;
        ch3Particles[i].life -= 0.008f;
        ch3Particles[i].alpha = ch3Particles[i].life;

        if (ch3Particles[i].life <= 0.0f) {
            initCh3Particle(ch3Particles[i]);
        }
    }
}

static void drawCh3Particles() {
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);

    glPointSize(3.5f);
    glBegin(GL_POINTS);
    for (const auto &p : ch3Particles) {
        if (ch3Solved) {
            // Gold particles when solved
            glColor4f(1.0f, 0.8f, 0.1f, p.alpha);
        } else if (!roomLight) {
            glColor4f(p.r, p.g, p.b, p.alpha);
        } else {
            glColor4f(p.r, p.g, p.b, p.alpha * 0.2f); // very faint in light mode
        }
        glVertex3f(p.x, p.y, p.z);
    }
    glEnd();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

// --- Helper: Draw Octagonal Room 3 ---
void drawOctagonRoom3() {
    float r = 6.0f;
    float h = 5.0f;

    // Floor
    glEnable(GL_TEXTURE_2D);
    if (hasGroundTex) {
        glBindTexture(GL_TEXTURE_2D, texGround);
    } else {
        glBindTexture(GL_TEXTURE_2D, texStone);
    }
    glColor3f(1.0f, 1.0f, 1.0f);

    GLfloat floorAmb[]  = { 0.55f, 0.55f, 0.55f, 1.0f };
    GLfloat floorDiff[] = { 1.0f,  1.0f,  1.0f,  1.0f };
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

    GLfloat ceilAmb[]  = { 0.4f, 0.4f, 0.4f, 1.0f };
    GLfloat ceilDiff[] = { 0.6f, 0.6f, 0.6f, 1.0f };
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

    // Walls
    for (int i = 0; i < 8; ++i) {
        float angle1 = i * 45.0f * PI / 180.0f;
        float angle2 = (i + 1) * 45.0f * PI / 180.0f;
        float x1 = r * cos(angle1), z1 = r * sin(angle1);
        float x2 = r * cos(angle2), z2 = r * sin(angle2);

        // Normal pointing inward
        float mx = (x1 + x2) / 2.0f;
        float mz = (z1 + z2) / 2.0f;
        float len = sqrt(mx*mx + mz*mz);
        float nx = -mx / len;
        float nz = -mz / len;

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

// --- Draw Dragon Hologram Relic ---
static void drawDragonHologram() {
    static float holoRot = 0.0f;
    holoRot += 0.8f;

    glPushMatrix();
    glTranslatef(0.0f, 1.3f, 0.0f);
    glRotatef(holoRot, 0.0f, 1.0f, 0.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    glDisable(GL_LIGHTING);

    // Hologram Color
    if (ch3Solved) {
        glColor4f(1.0f, 0.85f, 0.1f, 0.85f); // Glowing Gold
    } else if (!roomLight) {
        glColor4f(0.0f, 0.7f, 1.0f, 0.7f);   // Mystic Blue
    } else {
        glColor4f(0.0f, 0.7f, 1.0f, 0.12f);  // Faded in Light Mode
    }

    // 1. Draw Textured Cube (Relic)
    if (hasDragonTex && texDragon != 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texDragon);
        
        float h = 0.35f;
        glBegin(GL_QUADS);
        // Front
        glNormal3f(0.0f, 0.0f, 1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-h, -h, h);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( h, -h, h);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( h,  h, h);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-h,  h, h);
        // Back
        glNormal3f(0.0f, 0.0f, -1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f( h, -h, -h);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-h, -h, -h);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-h,  h, -h);
        glTexCoord2f(0.0f, 1.0f); glVertex3f( h,  h, -h);
        // Left
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-h, -h, -h);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-h, -h,  h);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-h,  h,  h);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-h,  h, -h);
        // Right
        glNormal3f(1.0f, 0.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f( h, -h,  h);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( h, -h, -h);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( h,  h, -h);
        glTexCoord2f(0.0f, 1.0f); glVertex3f( h,  h,  h);
        // Top
        glNormal3f(0.0f, 1.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-h,  h,  h);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( h,  h,  h);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( h,  h, -h);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-h,  h, -h);
        // Bottom
        glNormal3f(0.0f, -1.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-h, -h, -h);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( h, -h, -h);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( h, -h,  h);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-h, -h,  h);
        glEnd();
        
        glDisable(GL_TEXTURE_2D);
    } else {
        glutWireCube(0.7f);
    }

    // 2. Wireframe Gold/Cyan Relic Borders
    float h = 0.35f;
    float thick = 0.02f;
    for (float x : {-h, h}) {
        for (float z : {-h, h}) {
            glPushMatrix();
            glTranslatef(x, 0.0f, z);
            glScalef(thick, 0.7f + thick, thick);
            glutSolidCube(1.0f);
            glPopMatrix();
        }
    }
    for (float z : {-h, h}) {
        glPushMatrix();
        glTranslatef(0.0f, h, z);
        glScalef(0.7f + thick, thick, thick);
        glutSolidCube(1.0f);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(0.0f, -h, z);
        glScalef(0.7f + thick, thick, thick);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

    // 3. Orbiting Hologram Rings
    glPushMatrix();
    glRotatef(holoRot * 1.5f, 1.0f, 0.5f, 0.0f);
    glutWireSphere(0.55f, 12, 12);
    glPopMatrix();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glPopMatrix();
}

// --- Draw Chamber 3 ---
void drawChamber3() {
    drawOctagonRoom3();

    // 2. Central Pedestal holding the Hologram
    glPushMatrix();
    glTranslatef(0.0f, 0.25f, 0.0f);
    drawPrism(0.7f, 0.5f, 0.7f, texBox, hasBoxTex, 0.40f, 0.40f, 0.40f, 1.0f, 1.5f);
    glPopMatrix();

    // Pedestal Lid (slides open when solved)
    glPushMatrix();
    glTranslatef(0.0f, 0.52f, -0.35f - ch3PedestalOpenProgress * 0.5f);
    drawPrism(0.72f, 0.06f, 0.72f, texBox, hasBoxTex, 0.50f, 0.50f, 0.50f, 1.0f, 1.0f);
    glPopMatrix();

    // 3. Light Switch / Lever at front-left (-2.0, 3.8)
    glPushMatrix();
    glTranslatef(-2.0f, 0.45f, 3.8f);
    drawPrism(0.3f, 0.9f, 0.3f, texStone, hasStoneTex, 0.4f, 0.4f, 0.4f, 1.0f, 1.0f);
    // Lever Base
    glTranslatef(0.0f, 0.45f, 0.0f);
    drawPrism(0.12f, 0.05f, 0.12f, texBox, hasBoxTex, 0.2f, 0.2f, 0.2f, 1.0f, 1.0f);
    // Lever Handle
    glPushMatrix();
    if (roomLight) {
        glRotatef(-30.0f, 1.0f, 0.0f, 0.0f); // Up / ON
    } else {
        glRotatef(30.0f, 1.0f, 0.0f, 0.0f);  // Down / OFF
    }
    glTranslatef(0.0f, 0.15f, 0.0f);
    drawPrism(0.04f, 0.3f, 0.04f, texWood, hasWoodTex, 0.8f, 0.2f, 0.1f, 1.0f, 1.0f);
    // Knob
    glTranslatef(0.0f, 0.15f, 0.0f);
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 0.1f, 0.1f);
    glutSolidSphere(0.06f, 8, 8);
    glEnable(GL_LIGHTING);
    glPopMatrix();
    glPopMatrix();

    // 4. Target Frame on Back Wall at (-2.66, 1.30, -5.18)
    if (!roomLight) {
        glPushMatrix();
        glTranslatef(-2.66f, 1.30f, -5.18f);
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        if (ch3Solved) {
            glColor4f(1.0f, 0.85f, 0.1f, 0.85f); // Glowing Gold
        } else {
            glColor4f(0.0f, 0.9f, 0.6f, 0.65f); // Glowing Neon Cyan
        }

        // Draw double square frame
        glBegin(GL_LINE_LOOP);
        glVertex3f(-0.35f, -0.35f, 0.0f);
        glVertex3f( 0.35f, -0.35f, 0.0f);
        glVertex3f( 0.35f,  0.35f, 0.0f);
        glVertex3f(-0.35f,  0.35f, 0.0f);
        glEnd();

        glBegin(GL_LINE_LOOP);
        glVertex3f(-0.38f, -0.38f, 0.0f);
        glVertex3f( 0.38f, -0.38f, 0.0f);
        glVertex3f( 0.38f,  0.38f, 0.0f);
        glVertex3f(-0.38f,  0.38f, 0.0f);
        glEnd();

        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
        glPopMatrix();
    }

    // Solution Guide Beacon (toggled with 'U' key)
    if (showCh3SolutionGuide) {
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        // Draw vertical gold beam at target position
        glLineWidth(5.0f);
        glBegin(GL_LINES);
        glColor4f(1.0f, 0.8f, 0.1f, 0.4f);
        glVertex3f(targetCx, 0.0f, targetCz);
        glVertex3f(targetCx, 5.0f, targetCz);
        glEnd();

        glLineWidth(2.0f);
        glBegin(GL_LINES);
        glColor4f(1.0f, 0.95f, 0.7f, 0.9f);
        glVertex3f(targetCx, 0.0f, targetCz);
        glVertex3f(targetCx, 5.0f, targetCz);
        glEnd();

        // Glowing sphere beacon at target height
        glPushMatrix();
        glTranslatef(targetCx, targetCy, targetCz);
        glColor4f(1.0f, 0.85f, 0.1f, 0.85f);
        glutSolidSphere(0.12f, 16, 16);
        glPopMatrix();

        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
    }

    // 5. The Golden Key (visible only when solved)
    if (ch3Solved && !hasCh3Key) {
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

    // 6. Exit Doors
    // Left Door (Eye) at x = -1.2, z = -5.2
    glPushMatrix();
    glTranslatef(-1.2f, 1.05f, -5.2f);
    drawPrism(0.9f, 2.1f, 0.08f, texWood, hasWoodTex, 0.45f, 0.28f, 0.12f, 1.0f, 3.0f);
    glTranslatef(0.0f, 0.0f, 0.041f);
    drawDoorText("EYE");
    glPopMatrix();

    // Right Door (Mirror) at x = 1.2, z = -5.2
    glPushMatrix();
    glTranslatef(1.2f, 1.05f, -5.2f);
    drawPrism(0.9f, 2.1f, 0.08f, texWood, hasWoodTex, 0.45f, 0.28f, 0.12f, 1.0f, 3.0f);
    glTranslatef(0.0f, 0.0f, 0.041f);
    drawDoorText("MIRROR");
    glPopMatrix();

    // 1. Render the Dragon Hologram & Rising Sparks (DRAW LAST!)
    drawDragonHologram();
    drawCh3Particles();
}

// --- Chamber 3 Lighting ---
void setupChamber3Lighting() {
    if (roomLight) {
        // Bright Mode
        glClearColor(0.2f, 0.2f, 0.22f, 1.0f);
        GLfloat fogC[4] = { 0.2f, 0.2f, 0.22f, 1.0f };
        glFogfv(GL_FOG_COLOR, fogC);
        glFogf(GL_FOG_DENSITY, 0.02f);

        GLfloat a[] = { 0.55f, 0.55f, 0.55f, 1.0f };
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, a);
    } else {
        // Dark Hologram Mode
        glClearColor(0.01f, 0.01f, 0.03f, 1.0f);
        GLfloat fogC[4] = { 0.01f, 0.01f, 0.03f, 1.0f };
        glFogfv(GL_FOG_COLOR, fogC);
        glFogf(GL_FOG_DENSITY, 0.05f);

        GLfloat a[] = { 0.06f, 0.06f, 0.12f, 1.0f };
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, a);
    }
    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHT1);
    glDisable(GL_LIGHT2);
    glDisable(GL_LIGHT3);
}

// --- Chamber 3 Update Logic ---
void updateChamber3() {
    if (fadeAlpha > 0.0f) { fadeAlpha -= 0.04f; if (fadeAlpha < 0.0f) fadeAlpha = 0.0f; }

    // Update hologram particle system
    updateCh3Particles();

    // Pedestal lid animation
    if (ch3Solved && ch3PedestalOpenProgress < 1.0f) {
        ch3PedestalOpenProgress += 0.02f;
        if (ch3PedestalOpenProgress > 1.0f) ch3PedestalOpenProgress = 1.0f;
    }

    // Movement & flying controls
    const float PLAYER_SPEED = 0.06f, TURN_SPEED = 1.8f;

    if (keyStates['a'] || keyStates['A'] || specialKeyStates[GLUT_KEY_LEFT]) rotY += TURN_SPEED;
    if (keyStates['d'] || keyStates['D'] || specialKeyStates[GLUT_KEY_RIGHT]) rotY -= TURN_SPEED;

    // Flight controls
    if (keyStates[' ']) { // Space to fly UP
        camY += 0.05f;
        if (camY > 4.5f) camY = 4.5f;
    }
    if (keyStates['c'] || keyStates['C']) { // C to fly DOWN
        camY -= 0.05f;
        if (camY < 0.5f) camY = 0.5f;
    }

    float moveX = 0, moveZ = 0;
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
        float nX = camX + moveX, nZ = camZ + moveZ;
        float dist = sqrt(nX * nX + nZ * nZ);
        bool col = false;
        if (dist > 5.8f) col = true;
        // Pedestal collision
        if (sqrt(nX*nX + nZ*nZ) < 0.8f) col = true;
        // Lever collision
        if (sqrt((nX - (-2.0f))*(nX - (-2.0f)) + (nZ - 3.8f)*(nZ - 3.8f)) < 0.6f) col = true;

        if (!hasCh3Key && nZ < -4.8f && fabs(nX) < 1.8f) col = true;

        if (!col) {
            camX = nX;
            camZ = nZ;
        }
    }

    // Key collection
    if (ch3Solved && !hasCh3Key) {
        float dx = camX;
        float dz = camZ;
        if (sqrt(dx * dx + dz * dz) < 0.9f) {
            hasCh3Key = true;
            std::cout << "Chamber 3 Key Acquired!" << std::endl;
        }
    }

    // Check perspective alignment
    float posDiff = sqrt((camX - targetCx) * (camX - targetCx) +
                         (camY - targetCy) * (camY - targetCy) +
                         (camZ - targetCz) * (camZ - targetCz));
    // Wrap rotY diff to [0, 360)
    float diffRy = fmod(fabs(rotY - targetRy), 360.0f);
    if (diffRy > 180.0f) diffRy = 360.0f - diffRy;

    float diffRx = fabs(rotX - targetRx);
    float rotDiff = sqrt(diffRx * diffRx + diffRy * diffRy);

    // Solve if position, rotation, zoom, AND room is in Dark Mode
    if (!roomLight && posDiff < 0.35f && rotDiff < 6.0f && fabs(zoomFov - 45.0f) < 4.0f) {
        ch3Solved = true;
    } else {
        ch3Solved = false;
    }

    // Exit doors verification
    if (hasCh3Key && camZ < -4.5f) {
        // Left Door (Eye) at x = -1.2, z = -5.2
        float dxL = camX - (-1.2f);
        float dzL = camZ - (-5.2f);
        if (sqrt(dxL * dxL + dzL * dzL) < 0.6f) {
            // Incorrect Door (activates trap)
            isCh3FallingInTrap = true;
            ch3TrapFallY = 0.0f;
            ch3TrapFade = 0.0f;
            ch3RumbleTimer = 0.8f;
        }

        // Right Door (Mirror) at x = 1.2, z = -5.2
        float dxR = camX - 1.2f;
        float dzR = camZ - (-5.2f);
        if (sqrt(dxR * dxR + dzR * dzR) < 0.6f) {
            // Success: Transition to Chamber 4 (STATE_CHAMBER_4)
            currentGameState = STATE_CHAMBER_4;
            camX = 0.0f; camY = 1.0f; camZ = 4.8f;
            rotX = 10.0f; rotY = 0.0f;
            fadeAlpha = 1.0f;
        }
    }

    if (isCh3FallingInTrap) {
        ch3TrapFallY += 0.12f;
        ch3TrapFade += 0.02f;
        if (ch3TrapFade >= 1.0f) {
            camX = 0.0f; camY = 1.0f; camZ = 4.8f;
            rotX = 10.0f; rotY = 0.0f;
            isCh3FallingInTrap = false;
            ch3TrapFallY = 0.0f;
            ch3TrapFade = 0.0f;
            resetChamber3();
        }
    }
}

// --- Chamber 3 HUD ---
void drawChamber3HUD() {
    char msg[128];
    glColor3f(0.0f, 0.8f, 1.0f);
    renderBitmapString(20.0f, WINDOW_HEIGHT - 35.0f, GLUT_BITMAP_HELVETICA_12, "CHAMBER 3: THE CHAMBER OF DRAGON ILLUSION");
    
    glColor3f(0.8f, 0.8f, 0.8f);
    renderBitmapString(20.0f, WINDOW_HEIGHT - 55.0f, GLUT_BITMAP_HELVETICA_10, "Goal: Align the floating Dragon Hologram inside the glowing wall frame.");
    
    if (roomLight) {
        glColor3f(1.0f, 0.3f, 0.3f);
        renderBitmapString(20.0f, WINDOW_HEIGHT - 75.0f, GLUT_BITMAP_HELVETICA_10, "Room Light is ON. Hologram is faded. Interact with the switch lever near the entrance (or press [L]) to darken the room!");
    } else {
        glColor3f(0.3f, 1.0f, 0.6f);
        renderBitmapString(20.0f, WINDOW_HEIGHT - 75.0f, GLUT_BITMAP_HELVETICA_10, "Dark Mode Active! Align the Hologram inside the target frame on the back wall.");
    }
    
    // Guide toggle details
    sprintf(msg, "Press [U] to Toggle Solution Beacon: %s", showCh3SolutionGuide ? "ACTIVE" : "INACTIVE");
    glColor3f(1.0f, 0.85f, 0.1f);
    renderBitmapString(20.0f, WINDOW_HEIGHT - 90.0f, GLUT_BITMAP_HELVETICA_10, msg);

    // Display current values for developer/tester
    sprintf(msg, "Camera: Pos(%.2f, %.2f, %.2f) | Rot(Pitch: %.1f, Yaw: %.1f) | Zoom FOV: %.1f", camX, camY, camZ, rotX, rotY, zoomFov);
    renderBitmapString(20.0f, WINDOW_HEIGHT - 105.0f, GLUT_BITMAP_HELVETICA_10, msg);

    sprintf(msg, "Target: Pos(%.2f, %.2f, %.2f) | Rot(Pitch: %.1f, Yaw: %.1f) | Zoom FOV: 45.0", targetCx, targetCy, targetCz, targetRx, targetRy);
    renderBitmapString(20.0f, WINDOW_HEIGHT - 125.0f, GLUT_BITMAP_HELVETICA_10, msg);

    if (ch3Solved) {
        glColor3f(1.0f, 0.85f, 0.1f);
        renderBitmapString(WINDOW_WIDTH / 2.0f - 180.0f, WINDOW_HEIGHT / 2.0f + 120.0f, GLUT_BITMAP_HELVETICA_18, "PERSPECTIVE ALIGNED!");
        renderBitmapString(WINDOW_WIDTH / 2.0f - 160.0f, WINDOW_HEIGHT / 2.0f + 95.0f, GLUT_BITMAP_HELVETICA_12, "Proceed through the Right Door to Chamber 4.");
    }
}

// --- Reset Chamber 3 ---
void resetChamber3() {
    roomLight = true;
    ch3Solved = false;
    hasCh3Key = false;
    ch3PedestalOpenProgress = 0.0f;
    isCh3FallingInTrap = false;
    ch3TrapFallY = 0.0f;
    ch3TrapFade = 0.0f;
    ch3BlackoutTimer = 0.0f;
    ch3RumbleTimer = 0.0f;
    showCh3SolutionGuide = false;

    initCh3ParticleSystem();
}

// --- Interaction ---
void handleChamber3Interaction() {
    // Check leverage switch interaction
    float dx = camX - (-2.0f);
    float dz = camZ - 3.8f;
    if (sqrt(dx * dx + dz * dz) < 1.0f) {
        roomLight = !roomLight;
        ch3RumbleTimer = 0.12f;
        std::cout << "Chamber 3 Light lever toggled! Room light: " << (roomLight ? "ON" : "OFF") << std::endl;
    }
}
