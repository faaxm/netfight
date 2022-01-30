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
#include <math.h>
#include <stdio.h>

#include "bitFont.h"
#include "item.h"
#include "net_client.h"
#include "player.h"
#include "util.h"

extern PlayerGlobals pg;
ItemGlobals ig;

void DoInitItems(void)
{
    ig.levelItemCount = 0;
}

void DoAddItem(float x, float y, int type)
{
    ig.levelItem[ig.levelItemCount].inUse = 1;
    ig.levelItem[ig.levelItemCount].pos.x = x;
    ig.levelItem[ig.levelItemCount].pos.y = y;
    ig.levelItem[ig.levelItemCount].type = type;

    ig.levelItemCount++;
}

void DoActivateItem(int seed)
{
    int itemID = seed % ig.levelItemCount;

    ig.levelItem[itemID].inUse = 1;
}

void DoDisposeItems(void)
{
}

void DoHandlePlayerItemCollision(void)
{
    int i, j;
    Vect2D playerPos;
    float itemDist;

    i = pg.myPlayerID;
    if (pg.player[i].inUse) {
        playerPos = pg.player[i].pos;
        for (j = 0; j < ig.levelItemCount; j++) {
            if (ig.levelItem[j].inUse) {
                itemDist = vectDist(playerPos, ig.levelItem[j].pos);
                if (itemDist < PLAYER_RADIUS + ITEM_RADIUS) {
                    // printf("Player %i gets item %i...\n", i, j);
                    DoSendPlayerGotItem(i, j);
                    DoGiveItem(i, j);
                    // ig.levelItem[j].inUse = 0;
                }
            }
        }
        /* player in use stuff */
    }
}

void DoDisableItem(int itemID)
{
    ig.levelItem[itemID].inUse = 0;
}

void DoGiveItem(int playerID, int itemID)
{
    DoGiveItemToPlayer(playerID, ig.levelItem[itemID].type);
    DoDisableItem(itemID);
}

void DoDrawItems(void)
{
    int i;
    char str[128];

    glMatrixMode(GL_MODELVIEW);
    glLineWidth(1.0f);
    for (i = 0; i < ig.levelItemCount; i++) {
        if (ig.levelItem[i].inUse) {
            if (ig.levelItem[i].type < item_item)
                glColor3f(1.0f, 0.0f, (float)ig.levelItem[i].type / 20);
            else
                glColor3f(0.0f, 1.0f, (float)ig.levelItem[i].type / 20);

            glPushMatrix();
            glTranslatef(ig.levelItem[i].pos.x, ig.levelItem[i].pos.y, 0);
            switch (ig.levelItem[i].type) {
            case item_health:
#include "shapes/health.h"
                break;

            case item_shield:
#include "shapes/shield.h"
                break;

            case item_super_fast:
#include "shapes/super_fast.h"
                break;

            case item_weapon_spread:
#include "shapes/spread.h"
                break;

            case item_weapon_fast:
#include "shapes/fast.h"
                break;

            case item_weapon_mine:
#include "shapes/mine.h"
                break;

            default:
                glBegin(GL_LINES);
                glVertex2f(-0.2f, -0.2f);
                glVertex2f(+0.2f, +0.2f);

                glVertex2f(+0.2f, -0.2f);
                glVertex2f(-0.2f, +0.2f);
                glEnd();

                sprintf(str, "Item: %2i", ig.levelItem[i].type);
                printStringAt_dyn(str, 0.2f, 0.2f);
                break;
            }
            glPopMatrix();
        }
    }
    glLineWidth(1.0f);
}
