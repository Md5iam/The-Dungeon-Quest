#include <GL/glut.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>

// --- Global Constants & Variables ---
const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 768;

// Texture ID
GLuint dragonTextureID = 0;

// Interaction & Animation States
bool textureEnabled = true;
bool lightingEnabled = true;
bool autoRotate = true;
bool particlesEnabled = true;

float zoom = -5.0f;
float rotX = 25.0f;
float rotY = -45.0f;
float autoRotAngle = 0.0f;

// Mouse Drag State
bool isDragging = false;
int startMouseX = 0;
int startMouseY = 0;

// Light Orbit Settings
float lightAngle = 0.0f;
int lightColorIndex = 4; // Starts as Golden Yellow

// Color definitions for the orbiting light
const GLfloat LIGHT_COLORS[6][4] = {
    { 1.0f, 1.0f, 1.0f, 1.0f }, // 0: White
    { 1.0f, 0.2f, 0.2f, 1.0f }, // 1: Red
    { 0.2f, 1.0f, 0.2f, 1.0f }, // 2: Green
    { 0.2f, 0.4f, 1.0f, 1.0f }, // 3: Blue
    { 1.0f, 0.8f, 0.1f, 1.0f }, // 4: Gold
    { 0.1f, 0.9f, 0.9f, 1.0f }  // 5: Cyan
};
const char* LIGHT_COLOR_NAMES[6] = {
    "White", "Red", "Green", "Blue", "Gold", "Cyan"
};

// --- Particle System ---
struct Particle {
    float x, y, z;
    float vx, vy, vz;
    float r, g, b, alpha;
    float life;
    float size;
};

std::vector<Particle> particles;
const int MAX_PARTICLES = 150;

// Initialize a single particle
void initParticle(Particle &p) {
    // Generate random positions in a sphere around the relic box
    float theta = ((float)rand() / RAND_MAX) * 2.0f * M_PI;
    float phi = acos(((float)rand() / RAND_MAX) * 2.0f - 1.0f);
    float radius = 0.8f + ((float)rand() / RAND_MAX) * 1.5f;

    p.x = radius * sin(phi) * cos(theta);
    p.y = radius * sin(phi) * sin(theta) - 0.2f;
    p.z = radius * cos(phi);

    // Orbital-like velocities
    p.vx = -sin(theta) * 0.01f + (((float)rand() / RAND_MAX) - 0.5f) * 0.005f;
    p.vy = (((float)rand() / RAND_MAX) - 0.5f) * 0.01f + 0.008f; // Drift upwards
    p.vz = cos(theta) * 0.01f + (((float)rand() / RAND_MAX) - 0.5f) * 0.005f;

    // Golden sparks colors
    p.r = 1.0f;
    p.g = 0.5f + ((float)rand() / RAND_MAX) * 0.4f;
    p.b = 0.1f;
    p.alpha = 0.4f + ((float)rand() / RAND_MAX) * 0.6f;

    p.life = 0.5f + ((float)rand() / RAND_MAX) * 1.5f;
    p.size = 2.0f + ((float)rand() / RAND_MAX) * 4.0f;
}

void initParticleSystem() {
    srand(time(NULL));
    particles.resize(MAX_PARTICLES);
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        initParticle(particles[i]);
        // Disperse their initial life so they don't all appear at once
        particles[i].life = ((float)rand() / RAND_MAX);
    }
}

void updateParticles() {
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;
        particles[i].z += particles[i].vz;

        // Reduce life
        particles[i].life -= 0.008f;

        // Fade out
        particles[i].alpha = particles[i].life;

        // Reset if dead
        if (particles[i].life <= 0.0f) {
            initParticle(particles[i]);
        }
    }
}

void drawParticles() {
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE); // Don't write to depth buffer for transparency blending

    glPointSize(3.0f);
    glBegin(GL_POINTS);
    for (const auto &p : particles) {
        glColor4f(p.r, p.g, p.b, p.alpha);
        glVertex3f(p.x, p.y, p.z);
    }
    glEnd();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    if (lightingEnabled) glEnable(GL_LIGHTING);
}

// --- Texture Loading ---
GLuint loadTexture(const char* filename) {
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);
    
    if (!data) {
        std::cerr << "Failed to load texture: " << filename << std::endl;
        return 0;
    }
    
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    GLenum format = GL_RGB;
    if (nrChannels == 4) format = GL_RGBA;
    
    gluBuild2DMipmaps(GL_TEXTURE_2D, format, width, height, format, GL_UNSIGNED_BYTE, data);
    
    stbi_image_free(data);
    std::cout << "Successfully loaded texture: " << filename << " (" << width << "x" << height << ")" << std::endl;
    return textureID;
}

// --- Drawing Helper Functions ---

// Draw a single face of the relic box with texture coordinates mapped
void drawTexturedFace(float size) {
    float h = size / 2.0f;
    glBegin(GL_QUADS);
    // Normal pointing outward
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-h, -h, h);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( h, -h, h);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( h,  h, h);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-h,  h, h);
    glEnd();
}

// Draws a textured cube by rotating a single face to all 6 orientations
void drawTexturedCube(float size) {
    if (textureEnabled && dragonTextureID != 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, dragonTextureID);
    } else {
        glDisable(GL_TEXTURE_2D);
    }

    // Standard material properties for the texture surfaces
    GLfloat matAmbient[] = { 0.4f, 0.4f, 0.4f, 1.0f };
    GLfloat matDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat matSpecular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat matShininess[] = { 32.0f };
    
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);
    glColor3f(1.0f, 1.0f, 1.0f);

    // Front
    glPushMatrix();
    drawTexturedFace(size);
    glPopMatrix();

    // Back
    glPushMatrix();
    glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
    drawTexturedFace(size);
    glPopMatrix();

    // Left
    glPushMatrix();
    glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
    drawTexturedFace(size);
    glPopMatrix();

    // Right
    glPushMatrix();
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    drawTexturedFace(size);
    glPopMatrix();

    // Top
    glPushMatrix();
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    drawTexturedFace(size);
    glPopMatrix();

    // Bottom
    glPushMatrix();
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    drawTexturedFace(size);
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
}

// Draw the decorative metallic borders around the relic box
void drawRelicBorders(float size) {
    glDisable(GL_TEXTURE_2D);
    
    // Set material to a shiny gold/bronze
    GLfloat matAmbient[] = { 0.3f, 0.22f, 0.1f, 1.0f };
    GLfloat matDiffuse[] = { 0.75f, 0.6f, 0.22f, 1.0f };
    GLfloat matSpecular[] = { 0.9f, 0.8f, 0.5f, 1.0f };
    GLfloat matShininess[] = { 80.0f };
    
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);
    
    glColor3f(0.85f, 0.65f, 0.15f); // Gold fallback if lighting is off

    float h = size / 2.0f;
    float borderThick = 0.04f;

    // Draw solid bars along the 12 edges of the cube to make it look like a framed relic
    // 4 vertical pillars
    for (float x : {-h, h}) {
        for (float z : {-h, h}) {
            glPushMatrix();
            glTranslatef(x, 0.0f, z);
            glScalef(borderThick, size + borderThick, borderThick);
            glutSolidCube(1.0f);
            glPopMatrix();
        }
    }
    // 4 horizontal top frame bars
    for (float z : {-h, h}) {
        glPushMatrix();
        glTranslatef(0.0f, h, z);
        glScalef(size + borderThick, borderThick, borderThick);
        glutSolidCube(1.0f);
        glPopMatrix();
    }
    for (float x : {-h, h}) {
        glPushMatrix();
        glTranslatef(x, h, 0.0f);
        glScalef(borderThick, borderThick, size + borderThick);
        glutSolidCube(1.0f);
        glPopMatrix();
    }
    // 4 horizontal bottom frame bars
    for (float z : {-h, h}) {
        glPushMatrix();
        glTranslatef(0.0f, -h, z);
        glScalef(size + borderThick, borderThick, borderThick);
        glutSolidCube(1.0f);
        glPopMatrix();
    }
    for (float x : {-h, h}) {
        glPushMatrix();
        glTranslatef(x, -h, 0.0f);
        glScalef(borderThick, borderThick, size + borderThick);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

    // Add decorative corner spheres
    for (float x : {-h, h}) {
        for (float y : {-h, h}) {
            for (float z : {-h, h}) {
                glPushMatrix();
                glTranslatef(x, y, z);
                glutSolidSphere(0.07f, 16, 16);
                glPopMatrix();
            }
        }
    }
}

// Render a text string on screen using GLUT bitmap characters
void renderBitmapString(float x, float y, void *font, const char *string) {
    const char *c;
    glRasterPos2f(x, y);
    for (c = string; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
}

// Render 2D UI Overlay (HUD)
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

    // 1. Header panel background
    glColor4f(0.05f, 0.05f, 0.1f, 0.75f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);
    glVertex2f(0.0f, WINDOW_HEIGHT - 60.0f);
    glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT - 60.0f);
    glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
    glVertex2f(0.0f, WINDOW_HEIGHT);
    glEnd();

    // 2. Footer panel background
    glBegin(GL_QUADS);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(WINDOW_WIDTH, 0.0f);
    glVertex2f(WINDOW_WIDTH, 80.0f);
    glVertex2f(0.0f, 80.0f);
    glEnd();
    glDisable(GL_BLEND);

    // Title text
    glColor3f(1.0f, 0.8f, 0.2f);
    renderBitmapString(20.0f, WINDOW_HEIGHT - 38.0f, GLUT_BITMAP_HELVETICA_18, "ANCIENT DRAGON RELIC 3D ANIMATION");
    
    glColor3f(0.8f, 0.8f, 0.9f);
    renderBitmapString(WINDOW_WIDTH - 220.0f, WINDOW_HEIGHT - 35.0f, GLUT_BITMAP_HELVETICA_12, "University Computer Graphics Project");

    // Status Panel on Bottom-Left
    glColor3f(0.0f, 0.9f, 0.9f);
    renderBitmapString(20.0f, 50.0f, GLUT_BITMAP_HELVETICA_12, "STATUS INFO:");
    
    glColor3f(1.0f, 1.0f, 1.0f);
    char statusBuf[256];
    sprintf(statusBuf, "Texture (T): %s   |   Lighting (L): %s   |   Auto-Rotate (P): %s",
            textureEnabled ? "ENABLED" : "DISABLED",
            lightingEnabled ? "ENABLED" : "DISABLED",
            autoRotate ? "ENABLED" : "DISABLED");
    renderBitmapString(20.0f, 25.0f, GLUT_BITMAP_HELVETICA_12, statusBuf);

    // Light Details on Bottom-Middle
    sprintf(statusBuf, "Orbiting Light (C): %s", LIGHT_COLOR_NAMES[lightColorIndex]);
    glColor3f(LIGHT_COLORS[lightColorIndex][0], LIGHT_COLORS[lightColorIndex][1], LIGHT_COLORS[lightColorIndex][2]);
    renderBitmapString(WINDOW_WIDTH / 2 - 100.0f, 50.0f, GLUT_BITMAP_HELVETICA_12, statusBuf);

    // Controls on Bottom-Right
    glColor3f(0.8f, 0.8f, 0.8f);
    renderBitmapString(WINDOW_WIDTH - 420.0f, 50.0f, GLUT_BITMAP_HELVETICA_12, "CONTROLS:");
    renderBitmapString(WINDOW_WIDTH - 420.0f, 32.0f, GLUT_BITMAP_HELVETICA_10, "Mouse Drag : Rotate Object manually");
    renderBitmapString(WINDOW_WIDTH - 420.0f, 18.0f, GLUT_BITMAP_HELVETICA_10, "+ / - Keys : Zoom In / Out");
    renderBitmapString(WINDOW_WIDTH - 200.0f, 32.0f, GLUT_BITMAP_HELVETICA_10, "C : Cycle light color");
    renderBitmapString(WINDOW_WIDTH - 200.0f, 18.0f, GLUT_BITMAP_HELVETICA_10, "R : Reset scene  |  Q/Esc : Exit");

    // Restore matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glEnable(GL_DEPTH_TEST);
    if (lightingEnabled) glEnable(GL_LIGHTING);
}

// --- Main Display Callback ---
void display() {
    // Clear color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Position the camera
    glTranslatef(0.0f, 0.0f, zoom);
    
    // Manual rotations from mouse dragging
    glRotatef(rotX, 1.0f, 0.0f, 0.0f);
    glRotatef(rotY, 0.0f, 1.0f, 0.0f);

    // Set up lighting positions and colors
    if (lightingEnabled) {
        glEnable(GL_LIGHTING);
        
        // Define dynamic position of the orbiting light (GL_LIGHT1)
        float lightX = 2.0f * cos(lightAngle);
        float lightZ = 2.0f * sin(lightAngle);
        float lightY = 1.0f * sin(lightAngle * 0.5f);
        GLfloat lightPos1[] = { lightX, lightY, lightZ, 1.0f };
        
        glLightfv(GL_LIGHT1, GL_POSITION, lightPos1);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, LIGHT_COLORS[lightColorIndex]);
        glLightfv(GL_LIGHT1, GL_SPECULAR, LIGHT_COLORS[lightColorIndex]);
        
        // Also draw a tiny sphere at the orbiting light source position (emissive look)
        glDisable(GL_LIGHTING);
        glPushMatrix();
        glTranslatef(lightX, lightY, lightZ);
        glColor3fv(LIGHT_COLORS[lightColorIndex]);
        glutSolidSphere(0.05f, 10, 10);
        glPopMatrix();
        glEnable(GL_LIGHTING);
    } else {
        glDisable(GL_LIGHTING);
    }

    // --- Draw the Relic Box ---
    glPushMatrix();
    // Combine auto-rotation with manual camera rotation
    glRotatef(autoRotAngle, 0.1f, 1.0f, 0.3f);
    
    // Draw the main textured cube representing the relic
    drawTexturedCube(1.2f);
    
    // Draw metallic corner frames & pillars
    drawRelicBorders(1.2f);
    glPopMatrix();

    // --- Draw Particle System ---
    if (particlesEnabled) {
        drawParticles();
    }

    // --- Draw HUD 2D Overlay ---
    drawHUD();

    glutSwapBuffers();
}

// --- Animation Update & Timer ---
void update(int value) {
    // Rotate relic box if enabled
    if (autoRotate) {
        autoRotAngle += 0.4f;
        if (autoRotAngle > 360.0f) autoRotAngle -= 360.0f;
    }

    // Orbit the light source
    lightAngle += 0.02f;
    if (lightAngle > 2.0f * M_PI) lightAngle -= 2.0f * M_PI;

    // Update particle simulation
    if (particlesEnabled) {
        updateParticles();
    }

    // Redraw screen
    glutPostRedisplay();

    // Set timer for ~60 FPS (16ms)
    glutTimerFunc(16, update, 0);
}

// --- Window Reshape Callback ---
void reshape(int w, int h) {
    // Avoid division by zero
    if (h == 0) h = 1;
    
    glViewport(0, 0, w, h);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    // Setup perspective projection
    gluPerspective(45.0f, (GLfloat)w / (GLfloat)h, 0.1f, 100.0f);
    
    glMatrixMode(GL_MODELVIEW);
}

// --- Keyboard Event Handler ---
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'q':
        case 'Q':
        case 27: // ESC key
            exit(0);
            break;
            
        case 't':
        case 'T':
            textureEnabled = !textureEnabled;
            break;
            
        case 'l':
        case 'L':
            lightingEnabled = !lightingEnabled;
            if (lightingEnabled) {
                glEnable(GL_LIGHTING);
            } else {
                glDisable(GL_LIGHTING);
            }
            break;
            
        case 'p':
        case 'P':
            autoRotate = !autoRotate;
            break;
            
        case 'c':
        case 'C':
            // Cycle through available orbit light colors
            lightColorIndex = (lightColorIndex + 1) % 6;
            break;
            
        case 'v':
        case 'V':
            particlesEnabled = !particlesEnabled;
            break;
            
        case '+':
        case '=':
            zoom += 0.2f;
            if (zoom > -2.0f) zoom = -2.0f; // Limit zoom in
            break;
            
        case '-':
        case '_':
            zoom -= 0.2f;
            if (zoom < -15.0f) zoom = -15.0f; // Limit zoom out
            break;
            
        case 'r':
        case 'R':
            // Reset to defaults
            textureEnabled = true;
            lightingEnabled = true;
            autoRotate = true;
            particlesEnabled = true;
            zoom = -5.0f;
            rotX = 25.0f;
            rotY = -45.0f;
            lightColorIndex = 4;
            break;
    }
}

// --- Special Keys Handler (Arrows) ---
void specialKeys(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP:
            rotX -= 5.0f;
            break;
        case GLUT_KEY_DOWN:
            rotX += 5.0f;
            break;
        case GLUT_KEY_LEFT:
            rotY -= 5.0f;
            break;
        case GLUT_KEY_RIGHT:
            rotY += 5.0f;
            break;
    }
    glutPostRedisplay();
}

// --- Mouse Interaction Handlers ---
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
        // Adjust rotX and rotY based on mouse movement
        rotY += (x - startMouseX) * 0.5f;
        rotX += (y - startMouseY) * 0.5f;
        
        startMouseX = x;
        startMouseY = y;
        
        glutPostRedisplay();
    }
}

// --- OpenGL System Initialization ---
void initGL() {
    // Nice premium background color (Very dark navy blue instead of pure black)
    glClearColor(0.06f, 0.06f, 0.12f, 1.0f);
    
    // Enable depth buffering
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    
    // Enable smooth shading
    glShadeModel(GL_SMOOTH);
    
    // Enable lighting setup
    if (lightingEnabled) {
        glEnable(GL_LIGHTING);
        
        // Setup Ambient light (GL_LIGHT0)
        GLfloat lightAmbient0[] = { 0.2f, 0.2f, 0.2f, 1.0f };
        GLfloat lightDiffuse0[] = { 0.3f, 0.3f, 0.3f, 1.0f };
        GLfloat lightSpecular0[] = { 0.2f, 0.2f, 0.2f, 1.0f };
        GLfloat lightPos0[] = { 0.0f, 5.0f, 5.0f, 0.0f }; // Directional light from top-front
        
        glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient0);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse0);
        glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular0);
        glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);
        glEnable(GL_LIGHT0);
        
        // Setup Orbiting Point Light (GL_LIGHT1)
        GLfloat lightAmbient1[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        glLightfv(GL_LIGHT1, GL_AMBIENT, lightAmbient1);
        
        // Enable attenuation so the orbiting light feels local
        glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0f);
        glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.2f);
        glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.05f);
        glEnable(GL_LIGHT1);
    }
    
    // Enable auto normalization of normals
    glEnable(GL_NORMALIZE);

    // Set texture environment mode
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

// --- Main Program Entry Point ---
int main(int argc, char** argv) {
    // Initialize GLUT
    glutInit(&argc, argv);
    
    // Configure window display modes: double buffer, color index, depth buffer
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Ancient Dragon Relic 3D Animation");
    
    // Initialize OpenGL state
    initGL();
    
    // Load Texture
    dragonTextureID = loadTexture("image/dragon.jpg");
    if (dragonTextureID == 0) {
        std::cerr << "Warning: image/dragon.jpg could not be loaded. Running with color fallback." << std::endl;
    }
    
    // Initialize Particles
    initParticleSystem();

    // Register Callbacks
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    
    // Register Animation Update Timer
    glutTimerFunc(16, update, 0);
    
    // Start GLUT Event loop
    glutMainLoop();
    
    return 0;
}
