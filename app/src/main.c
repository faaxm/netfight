/***
 * Copyright (C) Falko Axmann.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 ****/

#include "myopengl.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bitFont.h"
#include "item.h"
#include "level.h"
#include "main.h"
#include "main_server.h"
#include "myMath.h"
#include "net_client.h"
#include "player.h"
#include "projectile.h"

Globals g;

void DoGameLoop(void);

int main(int argc, char* argv[])
{
    char serverName[128] = "localhost";
    int i;
    long curTime;

    time(&curTime);
    srandom(curTime);

    glfwInit();

    // init enet
    if (enet_initialize() != 0) {
        printf("An error occurred while initializing ENet.\n");
        return 0;
    }
    atexit(enet_deinitialize);

    // process commandline arguments
    for (i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0) { // start in server mode
            printf("\nStarting server ...\n\n");
            RunServer();
            return 0;
        } else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            strcpy(serverName, argv[i + 1]);
        }
    }

    // init globals
    g.screen_width = 640;
    g.screen_height = 480;

    g.done = 0;

    // init graphics
    // glfwOpenWindow(g.screen_width, g.screen_height,8,8,8,8,32,0,0);
    g.window = glfwCreateWindow(g.screen_width, g.screen_height, "Evil Triangle", NULL, NULL);
    glfwMakeContextCurrent(g.window);
    makeRasterFont();

    // init network
    printf("Connecting to server: %s\n", serverName);
    DoConnectToServer(serverName);

    // init level
    // DoLoadLevel(argv[0]);
    DoLoadLevel("data/test.lvl", &g.levelData);

    // init players
    DoInitPlayers();
    DoInitProjectiles();

    // show credits after the game started
    g.creditsStartTime = glfwGetTime();

    while (!g.done) {
        DoGameLoop();

        if (glfwWindowShouldClose(g.window)) {
            g.done = 1;
        }
    }

    DoDisposeLevel(&g.levelData);
    DoDisconnectFromServer();

    glfwTerminate();

    return 0;
}

void DoGameLoop(void)
{
    int width, height;
    float windowRatio, textYPos;

    // Get window size
    glfwGetFramebufferSize(g.window, &width, &height);
    windowRatio = (float)width / (float)height;
    // Set viewport
    glViewport(0, 0, width, height);

    glClearColor(0.1f, 0.1f, 0.2f, 0.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Setup projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-10 * windowRatio * 2 + 10, 10, -10, 10);

    // Setup modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    DoFocusCamOnPlayer();

    // move players
    DoUpdatePlayers();
    DoMovePlayers();
    // move projectiles
    DoUpdateProjectiles();

    // draw the level
    DoDrawLevel(&g.levelData);
    // draw items
    DoDrawItems();
    // draw the players
    DoDrawPlayers();
    // draw the projectiles
    DoDrawProjectiles();
    // get net info
    DoGetNetworkData();

    DoDrawHUD();

    if (glfwGetTime() - g.creditsStartTime < 5) {
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
        glBegin(GL_POLYGON);
        glVertex2f(-7, 5);
        glVertex2f(-7, -6);
        glVertex2f(7, -6);
        glVertex2f(7, 5);
        glEnd();
        glColor4f(0.0f, 0.5f, 1.0f, 1.0f);
        printStringAt_fixed("       Welcome to        ", -5, 2.5);
        printStringAt_fixed("      NetFight 1.0       ", -5, 1.5);
        glColor3f(1.0f, 0.0f, 0.0f);
        printStringAt_fixed("    ( EVIL TRIANGLE )    ", -5, 0.5);
        glColor3f(0.0f, 0.5f, 1.0f);
        printStringAt_fixed("Developed by Falko Axmann", -5, -1.5);

        printStringAt_fixed("   Press 'h' for help    ", -5, -5.0);

        glPopMatrix();
    }

    if (glfwGetKey(g.window, 'H')) { /* display help screen */
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
        glBegin(GL_POLYGON);
        glVertex2f(-9, 9);
        glVertex2f(-9, -3);
        glVertex2f(5, -3);
        glVertex2f(5, 9);
        glEnd();
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        textYPos = 8;
        printStringAt_fixed("Help - Controls                  ", -8, textYPos);
        textYPos -= 0.8;
        printStringAt_fixed(" Forward: ........ KP 8, UP      ", -8, textYPos);
        textYPos -= 0.8;
        printStringAt_fixed(" Backward: ....... KP 5, DOWN    ", -8, textYPos);
        textYPos -= 0.8;
        printStringAt_fixed(" Left: ........... KP 4, LEFT    ", -8, textYPos);
        textYPos -= 0.8;
        printStringAt_fixed(" Right: .......... KP 6, RIGHT   ", -8, textYPos);
        textYPos -= 0.8;
        printStringAt_fixed(" Sidestep Left: .. KP 1, A       ", -8, textYPos);
        textYPos -= 0.8;
        printStringAt_fixed(" Sidestep Right: . KP 3, S       ", -8, textYPos);
        textYPos -= 0.8;
        printStringAt_fixed(" Shoot: .......... SPACE         ", -8, textYPos);
        textYPos -= 0.8;
        printStringAt_fixed(" Use Item: ....... SHIFT         ", -8, textYPos);
        textYPos -= 0.8;
        printStringAt_fixed(" Next Weapon: .... KP 7, Q       ", -8, textYPos);
        textYPos -= 0.8;
        printStringAt_fixed(" Next Item: ...... KP 9, W       ", -8, textYPos);
        textYPos -= 0.8;
        printStringAt_fixed(" Scoreboard: ..... TAB           ", -8, textYPos);
        textYPos -= 0.8;
        printStringAt_fixed("                                 ", -8, textYPos);
        textYPos -= 0.8;
        //printStringAt_fixed(" Quit: ........... Q              ", -8, textYPos);
        textYPos -= 0.8;

        glPopMatrix();
    }

    glfwSwapBuffers(g.window);
    glfwPollEvents();
}
