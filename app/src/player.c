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

#include "GLFW/glfw3.h"
#include "myopengl.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bitFont.h"
#include "level.h"
#include "main.h"
#include "net_client.h"
#include "player.h"
#include "powerup.h"
#include "projectile.h"

#define localPlayer pg.player[pg.myPlayerID]

extern Globals g;
PlayerGlobals pg;

void DoInitPlayers(void)
{
    int i, j;
    double curTime = glfwGetTime();

    pg.myPlayerID = 0;
    pg.playerCount = 1;
    pg.displayScores = 0;

    for (i = 0; i < MAX_PLAYERS; i++) {
        DoRespawnPlayer(i);

        strncpy(pg.player[i].playerName, "unnamed", PLAYER_MAX_NAME_LENGTH);
        pg.player[i].score = 0;
        pg.player[i].inUse = 0;
        pg.player[i].lastShotTime = curTime;

        /* init weapons */
        for (j = 0; j < MAX_PLAYER_ITEMS; j++) {
            pg.player[i].item[j].ammo = 0;
            pg.player[i].item[j].type = j;
            pg.player[i].item[j].reloadTime = 0.05;
        }
        pg.player[i].item[0].ammo = -1; // inf ammo

        pg.player[i].item[item_weapon_spread].reloadTime = 0.2;
        pg.player[i].item[item_weapon_fast].reloadTime = 0.02;
        pg.player[i].item[item_weapon_mine].reloadTime = 1.0;
    }

    pg.lastPlayerUpdate = pg.lastNetUpdate = curTime;
}

void DoRespawnPlayer(int i)
{
    pg.player[i].pos = DoGetRespawnPoint(&g.levelData);
    pg.player[i].vel.x = pg.player[i].vel.y = 0;
    pg.player[i].rot = 0;
    pg.player[i].vel_rot = 0;
    pg.player[i].abs_vel = PLAYER_SPEED;

    pg.player[i].health = 100;

    pg.player[i].curWeapon = 0;
    pg.player[i].curItem = -1;
    pg.player[i].activePowerup = -1;

    pg.player[i].lastShotTime = glfwGetTime();
}

void DoUpdatePlayers(void)
{
    double curTime = glfwGetTime();
    int i;

    // update local player
    if (glfwGetKey(g.window, GLFW_KEY_KP_4) || glfwGetKey(g.window, GLFW_KEY_LEFT)) {
        localPlayer.vel_rot = PLAYER_ROT_SPEED;
    } else if (glfwGetKey(g.window, GLFW_KEY_KP_6) || glfwGetKey(g.window, GLFW_KEY_RIGHT)) {
        localPlayer.vel_rot = -PLAYER_ROT_SPEED;
    } else {
        localPlayer.vel_rot = 0;
    }

    if (glfwGetKey(g.window, GLFW_KEY_KP_8) || glfwGetKey(g.window, GLFW_KEY_UP)) {
        localPlayer.vel.x = localPlayer.abs_vel;
    } else if (glfwGetKey(g.window, GLFW_KEY_KP_5) || glfwGetKey(g.window, GLFW_KEY_DOWN)) {
        localPlayer.vel.x = -localPlayer.abs_vel;
    } else {
        localPlayer.vel.x = 0;
    }

    if (glfwGetKey(g.window, GLFW_KEY_KP_1) || glfwGetKey(g.window, 'A')) {
        localPlayer.vel.y = localPlayer.abs_vel;
    } else if (glfwGetKey(g.window, GLFW_KEY_KP_3) || glfwGetKey(g.window, 'S')) {
        localPlayer.vel.y = -localPlayer.abs_vel;
    } else {
        localPlayer.vel.y = 0;
    }

    /* shall we shoot? */
    if (glfwGetKey(g.window, GLFW_KEY_SPACE)) {
        if (curTime - localPlayer.lastShotTime > localPlayer.item[localPlayer.curWeapon].reloadTime) {
            localPlayer.lastShotTime = curTime;
            DoCreatePlayerProjectile();
        }
    }

    // weapon/item/item usage switching keys
    if (glfwGetKey(g.window, 'Q') || glfwGetKey(g.window, GLFW_KEY_KP_7)) {
        if (!g.actionKeyDown) {
            DoSwitchPlayerWeapon();
        }
        // action key is down...
        g.actionKeyDown = 1;
    } else if (glfwGetKey(g.window, 'W') || glfwGetKey(g.window, GLFW_KEY_KP_9)) {
        if (!g.actionKeyDown) {
            DoSwitchPlayerItem();
        }
        // action key is down...
        g.actionKeyDown = 1;
    } else if (glfwGetKey(g.window, GLFW_KEY_LEFT_SHIFT) || glfwGetKey(g.window, GLFW_KEY_RIGHT_SHIFT)) {
        if (!g.actionKeyDown) {
            DoUsePlayerPowerup();
        }
        g.actionKeyDown = 1;
    } else {
        g.actionKeyDown = 0;
    }

    // HUD keys
    if (glfwGetKey(g.window, GLFW_KEY_TAB)) {
        pg.displayScores = 1;
    } else {
        pg.displayScores = 0;
    }

    // send local player data to server
    if (curTime - pg.lastNetUpdate > PLAYER_NET_UPDATE_TIME) {
        pg.lastNetUpdate = curTime;
        // printf("Updating player at: %f\n", (float)curTime);
        DoSendPlayerPos(localPlayer.pos, localPlayer.vel, localPlayer.rot, localPlayer.vel_rot);
    }

    /* update other players */
    for (i = 0; i < MAX_PLAYERS; i++) {
        if (pg.player[i].inUse) {
            DoUpdatePowerup(&pg.player[i], curTime);
        }
    }
}

void DoCreatePlayerProjectile(void)
{
    Vect2D projPos;
    float projAngle;
    int projType;

    if (localPlayer.item[localPlayer.curWeapon].ammo == 0) {
        DoSwitchPlayerWeapon();
        return;
    }

    projAngle = localPlayer.rot * kPI / 180;

    projPos = localPlayer.pos;
    projPos.x += PLAYER_RADIUS * cos(projAngle);
    projPos.y += PLAYER_RADIUS * sin(projAngle);
    projAngle *= 180 / kPI;

    if (localPlayer.item[localPlayer.curWeapon].ammo > 0)
        localPlayer.item[localPlayer.curWeapon].ammo--;

    projType = localPlayer.item[localPlayer.curWeapon].type;
    // printf("Proj: %4i     of type: %4i  angle: %f \n", localPlayer.curWeapon, projType, projAngle);

    DoCreateProjectile(&projPos, &projAngle, &projType);
}

void DoUsePlayerPowerup(void)
{
    if (localPlayer.curItem >= 0) {
        if (localPlayer.item[localPlayer.curItem].ammo > 0) {
            localPlayer.item[localPlayer.curItem].ammo--;
            DoSendPlayerAction(localPlayer.curItem);
        }
    }
}

void DoMovePlayers(void)
{
    int i;
    float pRot;
    double timeScale;
    Vect2D movementVect;

    timeScale = glfwGetTime() - pg.lastPlayerUpdate;
    pg.lastPlayerUpdate = glfwGetTime();

    for (i = 0; i < pg.playerCount; i++) {
        pRot = pg.player[i].rot * kPI / 180;

        movementVect.x
            = pg.player[i].vel.x * cos(pRot) * timeScale + pg.player[i].vel.y * cos(pRot + kPI / 2) * timeScale;

        movementVect.y
            = pg.player[i].vel.x * sin(pRot) * timeScale + pg.player[i].vel.y * sin(pRot + kPI / 2) * timeScale;

        // forward movement
        if (!checkLevelCollision(pg.player[i].pos, movementVect, PLAYER_RADIUS, &g.levelData)) {
            pg.player[i].pos.x += movementVect.x;
            pg.player[i].pos.y += movementVect.y;
        }
        // rotation
        pg.player[i].rot += pg.player[i].vel_rot * timeScale;
        // clamp rotation
        while (pg.player[i].rot > 360) {
            pg.player[i].rot -= 360;
        }
        while (pg.player[i].rot < 0) {
            pg.player[i].rot += 360;
        }
    }

    /* did anybody get an item? */
    DoHandlePlayerItemCollision();
}

void DoDrawPlayers(void)
{
    int i;

    for (i = 0; i < MAX_PLAYERS; i++) {
        if (pg.player[i].inUse) {
            if (i == pg.myPlayerID)
                glColor3f(1.0f, 0.0f, 0.0f);
            else
                glColor3f(0.5f, 0.5f, 0.5f);
            DoDrawPlayerShape(pg.player[i].pos.x, pg.player[i].pos.y, pg.player[i].rot);

            DoDrawPowerup(&pg.player[i]);

            glColor3f(1.0f, 1.0f, 1.0f);
            printStringAt_dyn(pg.player[i].playerName, pg.player[i].pos.x + 0.5f, pg.player[i].pos.y + 0.5f);
        }
    }
}

void DoFocusCamOnPlayer(void)
{
    glTranslatef(-localPlayer.pos.x, -localPlayer.pos.y, 0.0f);
}

void DoDisposePlayers(void)
{
}

void DoSetLocalPlayer(int pID)
{
    /* copy player data */
    pg.player[pID] = pg.player[pg.myPlayerID];
    strncpy(pg.player[pID].playerName, getenv("USER"), PLAYER_MAX_NAME_LENGTH);

    pg.player[pg.myPlayerID].inUse = 0;
    pg.myPlayerID = pID;
    pg.player[pID].inUse = 1;
    if (pID >= pg.playerCount)
        pg.playerCount = pID + 1;

    DoSendPlayerInfo(pID, pg.player[pID].playerName);
    DoSendPlayerStats(pg.myPlayerID, localPlayer.score);
}

void PlayerDisconnected(int pID)
{
    char str[256];

    sprintf(str, "Player %s disconnected.", pg.player[pID].playerName);
    DoShowMessage(str);

    pg.player[pID].inUse = 0;
}

void DoHitPlayer(int playerHit, int projOwner, float damage)
{
    char str[256];

    if (playerHit == pg.myPlayerID && localPlayer.activePowerup != item_shield) {
        localPlayer.health -= damage;
        if (localPlayer.health <= 0) {
            sprintf(str, "You were killed by %s", pg.player[projOwner].playerName);
            DoShowMessage(str);
            DoRespawnPlayer(playerHit);
            DoSendPlayerKilled(playerHit, projOwner);
        }
    }
}

void PlayerWasKilled(int playerHit, int shotOwner)
{
    char str[256];

    if (shotOwner == pg.myPlayerID) {
        if (playerHit == pg.myPlayerID) {
            sprintf(str, "You committed suicide.");
            if (localPlayer.score > 0)
                localPlayer.score -= 1;
        } else {
            sprintf(str, "You killed %s", pg.player[playerHit].playerName);
            localPlayer.score += 1;
        }
        DoShowMessage(str);

        DoSendPlayerStats(pg.myPlayerID, localPlayer.score);
    }
}

void DoSetPlayerInfo(int pID, char* name)
{
    // printf("Received Player Info: %i  %s\n", pID, name);

    if (pID != pg.myPlayerID) {
        strncpy(pg.player[pID].playerName, name, PLAYER_MAX_NAME_LENGTH);
    }
}

void DoSetPlayerStats(int pID, int score)
{
    pg.player[pID].score = score;
}

void DoGiveItemToPlayer(int pID, int type)
{
    switch (type) {
    case item_health:
        pg.player[pID].health += ITEM_HEALTH_BONUS;
        break;

    default:
        if (pID == pg.myPlayerID)
            DoAddItemToInventory(type);
        break;
    }
}

void DoAddItemToInventory(int type)
{
    int i;

    for (i = 0; i < MAX_PLAYER_ITEMS; i++) {
        if (localPlayer.item[i].type == type) {
            switch (type) {
            case item_weapon_spread:
                localPlayer.item[i].ammo += 20;
                break;

            case item_weapon_fast:
                localPlayer.item[i].ammo += 50;
                break;

            case item_weapon_mine:
                localPlayer.item[i].ammo += 3;

            default:
                localPlayer.item[i].ammo += 1;
            }

            break;
        }
    }
}

void DoActivatePlayerPowerup(int pID, int type)
{
    int i;

    if (pID < 0 || pID >= MAX_PLAYERS) {
        printf("Bad player ID in DoActivatePlayerPowerup: %i\n", pID);
        return;
    }

    for (i = 0; i < MAX_PLAYER_ITEMS; i++) {
        /* find the item of the right type */
        if (pg.player[pID].item[i].type == type) {
            /* do we really own it? */
            // if(pg.player[pID].item[i].ammo > 0){
            //	pg.player[pID].item[i].ammo--;
            DoStartPowerupMode(&pg.player[pID], type);
            //} else {
            //	printf("The player doesn't have enough ammo to use that powerup\n");
            //}
        }
    }
}

void DoSwitchPlayerWeapon(void)
{
    do {
        localPlayer.curWeapon++;
        if (localPlayer.curWeapon == item_item)
            localPlayer.curWeapon = 0;
    } while (localPlayer.item[localPlayer.curWeapon].ammo == 0);
}

void DoSwitchPlayerItem(void)
{
    int lastItem;

    if (localPlayer.curItem < 0)
        localPlayer.curItem = item_item;
    lastItem = localPlayer.curItem;

    do {
        localPlayer.curItem++;
        if (localPlayer.curItem == item_max)
            localPlayer.curItem = item_item;
    } while (localPlayer.item[localPlayer.curItem].ammo == 0 && localPlayer.curItem != lastItem);

    if (localPlayer.item[localPlayer.curItem].ammo == 0)
        localPlayer.curItem = -1;
}

// HUD stuff
void DoShowMessage(const char* msg)
{
    pg.msgStartTime = glfwGetTime();
    strncpy(pg.hudMsg, msg, MAX_HUD_MSG_LENGTH);
}

// draw HUD for player
void DoDrawHUD(void)
{
    char str[128];
    double curTime = glfwGetTime();
    int i;

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_POLYGON);
    glVertex2f(-10, -10);
    glVertex2f(-10, 10);
    glVertex2f(-100, 10);
    glVertex2f(-100, -10);
    glEnd();

    glColor3f(0.8f, 1.0f, 0.8f);

    printStringAt_fixed("Health:", -16, 9);
    sprintf(str, " %3i%%", (int)localPlayer.health);
    printStringAt_fixed(str, -16, 8);

    printStringAt_fixed("Weapon:", -16, 6);
    // sprintf(str, "none");
    switch (localPlayer.item[localPlayer.curWeapon].type) {
    case item_weapon_basic:
        sprintf(str, "Photon");
        break;
    case item_weapon_spread:
        sprintf(str, "Spread");
        break;
    case item_weapon_fast:
        sprintf(str, "Fast");
        break;
    case item_weapon_mine:
        sprintf(str, "Mine");
        break;
    }
    printStringAt_fixed(str, -16, 5);
    sprintf(str, "(%i)", localPlayer.item[localPlayer.curWeapon].ammo);
    printStringAt_fixed(str, -16, 4);

    printStringAt_fixed("Item:", -16, 2);
    if (localPlayer.curItem < 0) {
        sprintf(str, "none");
        printStringAt_fixed(str, -16, 1);
    } else {
        switch (localPlayer.item[localPlayer.curItem].type) {
        case item_shield:
            sprintf(str, "Shield");
            break;
        case item_super_fast:
            sprintf(str, "Super Fast");
            break;
        }
        printStringAt_fixed(str, -16, 1);
        sprintf(str, "(%i)", localPlayer.item[localPlayer.curItem].ammo);
        printStringAt_fixed(str, -16, 0);
    }

    printStringAt_fixed("Score:", -16, -2);
    sprintf(str, " %4i", localPlayer.score);
    printStringAt_fixed(str, -16, -3);

    if (curTime - pg.msgStartTime < MSG_DISPLAY_TIME) {
        glColor3f(1.0f, 1.0f, 0.0f);
        printStringAt_fixed(pg.hudMsg, -9, -9);
    }

    if (pg.displayScores) {
        float textXPos = -9, textYPos = 8;

        glColor3f(0.3f, 0.3f, 1.0f);
        for (i = 0; i < pg.playerCount; i++) {
            if (pg.player[i].inUse) {
                sprintf(str, "%16s %4i", pg.player[i].playerName, pg.player[i].score);
                printStringAt_fixed(str, textXPos, textYPos);

                textYPos -= 0.7;
                if (textYPos < -8) {
                    textXPos = 0;
                    textYPos = 8;
                }
            }
        }
    }

    glPopMatrix();
}

// graphics stuff
void DoDrawPlayerShape(float x, float y, float rot)
{
    glMatrixMode(GL_MODELVIEW_MATRIX);
    glPushMatrix();
    glTranslatef(x, y, 0);
    glRotatef(rot, 0.0f, 0.0f, 1.0f);

    glBegin(GL_POLYGON);
    glVertex2f(0.5f, 0.0f);
    glVertex2f(-0.5f, -0.5f);
    glVertex2f(-0.5f, 0.5f);
    glEnd();
    glColor3f(0.6f, 0.8f, 0.6f);
    glBegin(GL_POLYGON);
    glVertex2f(0.5f, 0.0f);
    glVertex2f(-0.5f, -0.4f);
    glVertex2f(-0.5f, 0.4f);
    glEnd();

    glPopMatrix();
}
