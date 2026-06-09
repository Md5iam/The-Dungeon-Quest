#include "chamber1.h"
#include "chamber2.h"
#include <cstdlib>
#include <iostream>

// --- Chamber 1 Variable Definitions ---
bool platePressed[4] = {false, false, false, false};
std::vector<int> plateOrder;
bool ch1Solved = false;
bool hasKey = false;
float pedestalOpenProgress = 0.0f;
float ch1BlackoutTimer = 0.0f;
float ch1RumbleTimer = 0.0f;
bool isFallingInTrap = false;
float trapFallY = 0.0f;
float trapFade = 0.0f;
float statueBoxRot = 0.0f;

// --- Reset ---
void resetChamber1() {
    ch1Solved = false;
    hasKey = false;
    pedestalOpenProgress = 0.0f;
    isFallingInTrap = false;
    trapFallY = 0.0f;
    trapFade = 0.0f;
    ch1BlackoutTimer = 0.0f;
    ch1RumbleTimer = 0.0f;
    plateOrder.clear();
    for (int i = 0; i < 4; ++i) platePressed[i] = false;
}



// --- Octagon Room ---
void drawOctagonRoom() {
    float r = 6.0f;
    float h = 5.0f;
    int segments = 8;

    glEnable(GL_LIGHTING);
    if (hasGroundTex) { 
        glEnable(GL_TEXTURE_2D); 
        glBindTexture(GL_TEXTURE_2D, texGround); 
        glColor3f(1.0f,1.0f,1.0f); 
    }
    else if (hasStoneTex) { 
        glEnable(GL_TEXTURE_2D); 
        glBindTexture(GL_TEXTURE_2D, texStone); 
        glColor3f(1.0f,1.0f,1.0f); 
    }
    else { 
        glDisable(GL_TEXTURE_2D); 
        glColor3f(0.25f,0.25f,0.28f); 
    }

    GLfloat floorAmb[]  = { 0.55f, 0.55f, 0.55f, 1.0f };
    GLfloat floorDiff[] = { 1.0f,  1.0f,  1.0f,  1.0f };
    GLfloat floorSpec[] = { 0.0f,  0.0f,  0.0f,  1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT,  floorAmb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,  floorDiff);
    glMaterialfv(GL_FRONT, GL_SPECULAR, floorSpec);

    glBegin(GL_TRIANGLE_FAN);
    glTexCoord2f(0.0f,0.0f); glNormal3f(0,1,0); glVertex3f(0,0,0);
    for (int i = 0; i <= segments; ++i) {
        float a = i*2.0f*PI/segments;
        float cx = r*cos(a), cz = r*sin(a);
        glTexCoord2f(cx*0.5f, cz*0.5f);
        glVertex3f(cx, 0, cz);
    }
    glEnd();

    if (hasStoneTex) { 
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texStone); 
        glColor3f(1.0f,1.0f,1.0f); 
    }
    else {
        glDisable(GL_TEXTURE_2D);
        glColor3f(0.15f,0.15f,0.18f);
    }

    GLfloat ceilAmb[]  = { 0.4f, 0.4f, 0.4f, 1.0f };
    GLfloat ceilDiff[] = { 0.6f, 0.6f, 0.6f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT,  ceilAmb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,  ceilDiff);

    glBegin(GL_TRIANGLE_FAN);
    glTexCoord2f(0.5f,0.5f); glNormal3f(0,-1,0); glVertex3f(0,h,0);
    for (int i = 0; i <= segments; ++i) {
        float a = -i*2.0f*PI/segments;
        glTexCoord2f(0.5f+0.5f*cos(a), 0.5f+0.5f*sin(a));
        glVertex3f(r*cos(a), h, r*sin(a));
    }
    glEnd();

    GLfloat matWallAmb[] = {0.2f,0.2f,0.2f,1}; GLfloat matWallDiff[] = {0.5f,0.5f,0.5f,1};
    glMaterialfv(GL_FRONT, GL_AMBIENT, matWallAmb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matWallDiff);
    if (hasStoneTex) { glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, texStone); }
    else glDisable(GL_TEXTURE_2D);

    for (int i = 0; i < segments; ++i) {
        float a1 = i*2.0f*PI/segments, a2 = (i+1)*2.0f*PI/segments;
        float x1=r*cos(a1), z1=r*sin(a1), x2=r*cos(a2), z2=r*sin(a2);
        float nx = -(cos(a1)+cos(a2))/2.0f, nz = -(sin(a1)+sin(a2))/2.0f;
        float len = sqrt(nx*nx+nz*nz); nx/=len; nz/=len;
        glBegin(GL_QUADS);
        glNormal3f(nx,0,nz);
        glTexCoord2f(0,0); glVertex3f(x1,0,z1);
        glTexCoord2f(2,0); glVertex3f(x2,0,z2);
        glTexCoord2f(2,2); glVertex3f(x2,h,z2);
        glTexCoord2f(0,2); glVertex3f(x1,h,z1);
        glEnd();
    }
    glDisable(GL_TEXTURE_2D);
}

// --- Main Chamber 1 Rendering ---
void drawChamber1() {
    drawOctagonRoom();

    // Chandelier
    glDisable(GL_LIGHTING); glColor3f(0.2f,0.2f,0.2f);
    glBegin(GL_LINES); glVertex3f(0,5.0f,0); glVertex3f(0,3.2f,0); glEnd();
    glEnable(GL_LIGHTING);

    glPushMatrix();
    glTranslatef(0,3.2f,0); glRotatef(90,1,0,0);
    GLfloat gA[]={0.25f,0.20f,0.07f,1}, gD[]={0.75f,0.60f,0.23f,1};
    glMaterialfv(GL_FRONT,GL_AMBIENT,gA); glMaterialfv(GL_FRONT,GL_DIFFUSE,gD);
    glutSolidTorus(0.05f,0.3f,8,12);
    glPopMatrix();

    if (ch1BlackoutTimer == 0.0f) {
        glDisable(GL_LIGHTING); glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glColor4f(1,0.6f,0.1f,0.8f);
        glPushMatrix(); glTranslatef(0,3.1f,0); glutSolidSphere(0.12f,8,8); glPopMatrix();
        glDisable(GL_BLEND); glEnable(GL_LIGHTING);
    }

    // Statues
    float statueColors[4][3] = {{0.90f,0.90f,0.95f},{1.00f,0.75f,0.00f},{0.10f,0.40f,1.00f},{0.00f,0.85f,0.10f}};
    float statuePos[4][2] = {{-3.0f,-3.0f},{3.0f,-3.0f},{3.0f,3.0f},{-3.0f,3.0f}};

    for (int i = 0; i < 4; ++i) {
        // Statue pillar (fully colored if selected, otherwise stone texture)
        glPushMatrix();
        glTranslatef(statuePos[i][0], 0.4f, statuePos[i][1]);
        if (platePressed[i]) {
            glDisable(GL_LIGHTING);
            drawPrism(0.4f, 0.8f, 0.4f, 0, false, statueColors[i][0], statueColors[i][1], statueColors[i][2], 1.0f, 1.0f);
            glEnable(GL_LIGHTING);
        } else {
            drawPrism(0.4f, 0.8f, 0.4f, texStone, hasStoneTex, 0.45f, 0.45f, 0.45f, 1.0f, 1.0f);
        }
        glPopMatrix();

        // Glowing Animal Symbol (Rotating Box)
        glPushMatrix();
        glTranslatef(statuePos[i][0], 1.0f, statuePos[i][1]);
        glRotatef(statueBoxRot, 0.0f, 1.0f, 0.0f); // Rotate around Y axis like dragon animation
        
        GLuint activeTex = 0;
        bool hasActiveTex = false;
        if (i == 0) { activeTex = texWolf; hasActiveTex = hasWolfTex; }
        else if (i == 1) { activeTex = texLion; hasActiveTex = hasLionTex; }
        else if (i == 2) { activeTex = texEagle; hasActiveTex = hasEagleTex; }
        else { activeTex = texSnake; hasActiveTex = hasSnakeTex; }

        if (ch1Solved || platePressed[i]) {
            glDisable(GL_LIGHTING);
            drawPrism(0.4f, 0.4f, 0.4f, activeTex, hasActiveTex, statueColors[i][0], statueColors[i][1], statueColors[i][2], 1.0f, 1.0f);
            glEnable(GL_LIGHTING);
        } else {
            GLfloat sA[] = { statueColors[i][0] * 0.2f, statueColors[i][1] * 0.2f, statueColors[i][2] * 0.2f, 1.0f };
            GLfloat sD[] = { statueColors[i][0] * 0.5f, statueColors[i][1] * 0.5f, statueColors[i][2] * 0.5f, 1.0f };
            glMaterialfv(GL_FRONT, GL_AMBIENT, sA);
            glMaterialfv(GL_FRONT, GL_DIFFUSE, sD);
            drawPrism(0.4f, 0.4f, 0.4f, activeTex, hasActiveTex, statueColors[i][0], statueColors[i][1], statueColors[i][2], 1.0f, 1.0f);
        }
        glPopMatrix();
    }

    // Central Pedestal
    glPushMatrix(); glTranslatef(0,0.25f,0);
    drawPrism(0.7f,0.5f,0.7f,texBox,hasBoxTex,0.40f,0.40f,0.40f,1,1.5f);
    glPopMatrix();

    // Sliding Lid (slides open when solved)
    glPushMatrix(); glTranslatef(0, 0.52f, -0.35f - pedestalOpenProgress * 0.5f);
    drawPrism(0.72f,0.06f,0.72f,texBox,hasBoxTex,0.50f,0.50f,0.50f,1,1);
    glPopMatrix();

    // Golden Key (visible only when solved)
    if (ch1Solved && !hasKey) {
        static float keyRot = 0.0f; keyRot += 1.5f;
        glPushMatrix();
        glTranslatef(0, 0.75f+0.05f*sin(keyRot*PI/180.0f), 0);
        glRotatef(keyRot, 0,1,0);
        glDisable(GL_LIGHTING); glColor3f(1,0.85f,0.1f);
        glPushMatrix(); glTranslatef(0,0.15f,0); glutSolidTorus(0.03f,0.08f,6,8); glPopMatrix();
        glPushMatrix(); glTranslatef(0,-0.05f,0); drawPrism(0.04f,0.3f,0.04f,0,false,1,0.85f,0.1f,1,1); glPopMatrix();
        glPushMatrix(); glTranslatef(0.06f,-0.15f,0); drawPrism(0.08f,0.06f,0.03f,0,false,1,0.85f,0.1f,1,1); glPopMatrix();
        glEnable(GL_LIGHTING);
        glPopMatrix();
    }

    // Exit Doors
    glPushMatrix(); glTranslatef(-1.2f,1.05f,-5.2f);
    drawPrism(0.9f,2.1f,0.08f,texWood,hasWoodTex,0.45f,0.28f,0.12f,1,3);
    glTranslatef(0,0,0.041f); drawDoorText("SUN"); glPopMatrix();

    glPushMatrix(); glTranslatef(1.2f,1.05f,-5.2f);
    drawPrism(0.9f,2.1f,0.08f,texWood,hasWoodTex,0.45f,0.28f,0.12f,1,3);
    glTranslatef(0,0,0.041f); drawDoorText("MOON"); glPopMatrix();
}

// --- Chamber 1 Lighting ---
void setupChamber1Lighting() {
    glClearColor(0.02f,0.01f,0.04f,1);
    GLfloat fogC[4]={0.02f,0.01f,0.04f,1};
    glFogfv(GL_FOG_COLOR, fogC);
    glFogf(GL_FOG_DENSITY, 0.03f);

    if (ch1BlackoutTimer > 0.0f) {
        GLfloat a[]={0,0,0,1}; glLightModelfv(GL_LIGHT_MODEL_AMBIENT, a);
        glDisable(GL_LIGHT0); glDisable(GL_LIGHT1); glDisable(GL_LIGHT2); glDisable(GL_LIGHT3);
    } else {
        GLfloat a[]={0.50f,0.45f,0.55f,1}; glLightModelfv(GL_LIGHT_MODEL_AMBIENT, a);
        glDisable(GL_LIGHT0); glDisable(GL_LIGHT1); glDisable(GL_LIGHT2);
        GLfloat lA[]={0.25f,0.25f,0.30f,1}, lD[]={0.95f,0.90f,0.75f,1};
        GLfloat lS[]={0.30f,0.30f,0.30f,1}, lP[]={0,3.2f,0,1};
        glLightfv(GL_LIGHT3,GL_AMBIENT,lA); glLightfv(GL_LIGHT3,GL_DIFFUSE,lD);
        glLightfv(GL_LIGHT3,GL_SPECULAR,lS); glLightfv(GL_LIGHT3,GL_POSITION,lP);
        glLightf(GL_LIGHT3,GL_CONSTANT_ATTENUATION,0.5f);
        glLightf(GL_LIGHT3,GL_LINEAR_ATTENUATION,0.05f);
        glLightf(GL_LIGHT3,GL_QUADRATIC_ATTENUATION,0.01f);
        glEnable(GL_LIGHT3);
    }
}

// --- Chamber 1 Update Logic ---
void updateChamber1() {
    if (fadeAlpha > 0.0f) { fadeAlpha -= 0.04f; if (fadeAlpha < 0.0f) fadeAlpha = 0.0f; }

    statueBoxRot += 1.2f;
    if (statueBoxRot >= 360.0f) statueBoxRot -= 360.0f;

    if (ch1Solved && pedestalOpenProgress < 1.0f) {
        pedestalOpenProgress += 0.02f; if (pedestalOpenProgress > 1.0f) pedestalOpenProgress = 1.0f;
    }
    if (ch1BlackoutTimer > 0.0f) { ch1BlackoutTimer -= 0.016f; if (ch1BlackoutTimer < 0.0f) ch1BlackoutTimer = 0.0f; }
    if (ch1RumbleTimer > 0.0f) { ch1RumbleTimer -= 0.016f; if (ch1RumbleTimer < 0.0f) ch1RumbleTimer = 0.0f; }

    if (isFallingInTrap) {
        trapFallY += 0.12f; trapFade += 0.02f;
        if (trapFade >= 1.0f) {
            camX=0; camY=1; camZ=4.8f; rotX=10; rotY=0;
            isFallingInTrap=false; trapFallY=0; trapFade=0;
            resetChamber1();
        }
    } else {
        const float PLAYER_SPEED = 0.06f, TURN_SPEED = 1.8f;

        if (keyStates['a']||keyStates['A']||specialKeyStates[GLUT_KEY_LEFT]) rotY += TURN_SPEED;
        if (keyStates['d']||keyStates['D']||specialKeyStates[GLUT_KEY_RIGHT]) rotY -= TURN_SPEED;

        float moveX=0, moveZ=0; bool isMoving=false;
        if (keyStates['w']||keyStates['W']||specialKeyStates[GLUT_KEY_UP]) {
            float rad=rotY*PI/180.0f; moveX=-sin(rad)*PLAYER_SPEED; moveZ=-cos(rad)*PLAYER_SPEED; isMoving=true;
        }
        if (keyStates['s']||keyStates['S']||specialKeyStates[GLUT_KEY_DOWN]) {
            float rad=rotY*PI/180.0f; moveX=sin(rad)*PLAYER_SPEED; moveZ=cos(rad)*PLAYER_SPEED; isMoving=true;
        }

        if (isMoving) {
            float nX=camX+moveX, nZ=camZ+moveZ;
            float dist=sqrt(nX*nX+nZ*nZ); bool col=false;
            if (dist > 5.8f) col=true;
            if (dist < 0.6f) col=true;
            float sP[4][2]={{-3,-3},{3,-3},{3,3},{-3,3}};
            for (int i=0;i<4;++i) { float dx=nX-sP[i][0],dz=nZ-sP[i][1]; if(sqrt(dx*dx+dz*dz)<0.5f) col=true; }
            if (!hasKey && nZ < -4.8f && fabs(nX) < 1.8f) col=true;
            if (!col) { camX=nX; camZ=nZ; }
        }



        if (ch1Solved && !hasKey) { float dx=camX, dz=camZ; if(sqrt(dx*dx+dz*dz)<0.9f) hasKey=true; }

        if (hasKey && camZ < -4.5f) {
            float dxL=camX-(-1.2f), dzL=camZ-(-5.2f);
            if (sqrt(dxL*dxL+dzL*dzL)<0.6f) {
                currentGameState = STATE_CHAMBER_2;
                camX = 0.0f; camY = 1.0f; camZ = 4.8f;
                rotX = 10.0f; rotY = 0.0f;
                fadeAlpha = 1.0f;
                resetChamber2();
            }
            float dxR=camX-1.2f, dzR=camZ-(-5.2f);
            if (sqrt(dxR*dxR+dzR*dzR)<0.6f) { isFallingInTrap=true; trapFallY=0; trapFade=0; }
        }
    }
}

// --- Chamber 1 HUD ---
void drawChamber1HUD() {
    if (currentGameState == STATE_CHAMBER_1) {
        glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.04f,0.02f,0.08f,0.85f);
        glBegin(GL_QUADS); glVertex2f(0,0); glVertex2f(WINDOW_WIDTH,0); glVertex2f(WINDOW_WIDTH,120); glVertex2f(0,120); glEnd();
        glColor4f(0.04f,0.02f,0.08f,0.85f);
        glBegin(GL_QUADS); glVertex2f(0,WINDOW_HEIGHT-65.0f); glVertex2f(WINDOW_WIDTH,WINDOW_HEIGHT-65.0f); glVertex2f(WINDOW_WIDTH,WINDOW_HEIGHT); glVertex2f(0,WINDOW_HEIGHT); glEnd();
        glDisable(GL_BLEND);

        glColor3f(0.7f,0.5f,1); renderBitmapString(25,WINDOW_HEIGHT-25,GLUT_BITMAP_HELVETICA_18,"CHAMBER 1: THE HALL OF ANCIENT SYMBOLS");
        glColor3f(0.9f,0.75f,0.5f);
        renderBitmapString(25,WINDOW_HEIGHT-50,GLUT_BITMAP_HELVETICA_12,
            "Wall Inscription: \"The hunter stalks before the king. The king stands before the sky. The sky watches over the silent one.\"");

        if (!ch1Solved) {
            glColor3f(1,1,1); renderBitmapString(30,90,GLUT_BITMAP_HELVETICA_12,"Objective: Select the four statues in the correct sequence.");
            std::string seqStr = "Selected statues: ";
            if (plateOrder.empty()) seqStr += "None (Approach a statue and press [ENTER]/[E] to select)";
            else { const char* names[4]={"Wolf","Lion","Eagle","Snake"}; for(size_t i=0;i<plateOrder.size();++i){seqStr+=names[plateOrder[i]]; if(i<plateOrder.size()-1)seqStr+=" -> ";} }
            glColor3f(0.4f,0.8f,1); renderBitmapString(30,65,GLUT_BITMAP_HELVETICA_12,seqStr.c_str());
        } else if (!hasKey) {
            glColor3f(1,0.85f,0.1f); renderBitmapString(30,90,GLUT_BITMAP_HELVETICA_12,"SUCCESS! The central pedestal lid has slid open.");
            glColor3f(1,1,1); renderBitmapString(30,65,GLUT_BITMAP_HELVETICA_12,"Objective: Approach the central pedestal and collect the Golden Key.");
        } else {
            glColor3f(0.2f,0.9f,0.3f); renderBitmapString(30,95,GLUT_BITMAP_HELVETICA_12,"KEY ACQUIRED! The two exit doors are now active.");
            glColor3f(0.9f,0.85f,0.5f); renderBitmapString(30,75,GLUT_BITMAP_HELVETICA_12,"Door Riddle: \"I arrive each morning without being called. I leave each evening without being chased.\"");
            glColor3f(1,1,1); renderBitmapString(30,50,GLUT_BITMAP_HELVETICA_12,"Objective: Walk through the correct door - Left Door (Sun) or Right Door (Moon)");
        }
        glColor3f(0.7f,0.7f,0.7f); renderBitmapString(30,20,GLUT_BITMAP_HELVETICA_10,"Controls: WASD / Arrow Keys to Move & Turn | R to reset to outside");

        if (ch1BlackoutTimer > 0.0f) {
            glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(0,0,0,ch1BlackoutTimer);
            glBegin(GL_QUADS); glVertex2f(0,0); glVertex2f(WINDOW_WIDTH,0); glVertex2f(WINDOW_WIDTH,WINDOW_HEIGHT); glVertex2f(0,WINDOW_HEIGHT); glEnd();
            glColor3f(1,0.2f,0.2f); renderBitmapString(WINDOW_WIDTH/2-180.0f,WINDOW_HEIGHT/2,GLUT_BITMAP_HELVETICA_18,"WRONG SEQUENCE! THE TORCHES BLOW OUT!");
            glDisable(GL_BLEND);
        }
        if (isFallingInTrap) {
            glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(0.3f,0,0,trapFade);
            glBegin(GL_QUADS); glVertex2f(0,0); glVertex2f(WINDOW_WIDTH,0); glVertex2f(WINDOW_WIDTH,WINDOW_HEIGHT); glVertex2f(0,WINDOW_HEIGHT); glEnd();
            glColor3f(1,0.1f,0.1f); renderBitmapString(WINDOW_WIDTH/2-160.0f,WINDOW_HEIGHT/2,GLUT_BITMAP_HELVETICA_18,"YOU CHOSE WRONG! TRAP DETONATED!");
            glColor3f(0.8f,0.8f,0.8f); renderBitmapString(WINDOW_WIDTH/2-100.0f,WINDOW_HEIGHT/2-30.0f,GLUT_BITMAP_HELVETICA_12,"Resetting chamber...");
            glDisable(GL_BLEND);
        }
    } else if (currentGameState == STATE_VICTORY) {
        glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.08f,0.07f,0.02f,0.9f);
        glBegin(GL_QUADS); glVertex2f(0,0); glVertex2f(WINDOW_WIDTH,0); glVertex2f(WINDOW_WIDTH,WINDOW_HEIGHT); glVertex2f(0,WINDOW_HEIGHT); glEnd();
        glDisable(GL_BLEND);
        glColor3f(1,0.85f,0.1f); renderBitmapString(WINDOW_WIDTH/2-150.0f,WINDOW_HEIGHT/2+60.0f,GLUT_BITMAP_HELVETICA_18,"star CHAMBER 1 COMPLETE! star");
        glColor3f(0.9f,0.9f,0.9f); renderBitmapString(WINDOW_WIDTH/2-250.0f,WINDOW_HEIGHT/2+10.0f,GLUT_BITMAP_HELVETICA_12,"You correctly identified the Sun as the answer to the riddle and safely escaped!");
        renderBitmapString(WINDOW_WIDTH/2-160.0f,WINDOW_HEIGHT/2-20.0f,GLUT_BITMAP_HELVETICA_12,"The pathway to Chamber 2 lies ahead.");
        glColor3f(0,0.9f,1); renderBitmapString(WINDOW_WIDTH/2-140.0f,WINDOW_HEIGHT/2-80.0f,GLUT_BITMAP_HELVETICA_12,"PRESS [R] TO PLAY AGAIN FROM START");
    }
}

void handleChamber1Interaction() {
    float statuePos[4][2] = {{-3.0f,-3.0f},{3.0f,-3.0f},{3.0f,3.0f},{-3.0f,3.0f}};
    for (int i = 0; i < 4; ++i) {
        float dx = camX - statuePos[i][0];
        float dz = camZ - statuePos[i][1];
        if (sqrt(dx * dx + dz * dz) < 1.0f) {
            if (!platePressed[i]) {
                platePressed[i] = true;
                plateOrder.push_back(i);
                std::cout << "Selected statue " << i << std::endl;
                ch1RumbleTimer = 0.15f;
                if (plateOrder.size() == 4) {
                    if (plateOrder[0] == 0 && plateOrder[1] == 1 && plateOrder[2] == 2 && plateOrder[3] == 3) {
                        ch1Solved = true;
                        std::cout << "SUCCESS! Statues selected in the correct order!" << std::endl;
                    } else {
                        ch1BlackoutTimer = 0.8f;
                        ch1RumbleTimer = 0.5f;
                        plateOrder.clear();
                        for (int k = 0; k < 4; ++k) {
                            platePressed[k] = false;
                        }
                        std::cout << "WRONG ORDER! Resetting statues..." << std::endl;
                    }
                }
            }
            break;
        }
    }
}
