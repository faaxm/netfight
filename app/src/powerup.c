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

#include "powerup.h"
#include <GLFW/glfw3.h>

#include "myopengl.h"
#include <math.h>

void DoStartPowerupMode(Player* player, int type)
{
    double curTime = glfwGetTime();

    switch (type) {
    case item_shield:
        // player->abs_vel = PLAYER_SPEED * 1.45f;
        player->powerupDeactivationTime = curTime + 10.0f;
        break;

    case item_super_fast:
        player->abs_vel = PLAYER_SPEED * 1.45f;
        player->powerupDeactivationTime = curTime + 5.0f;
        break;
    }

    player->activePowerup = type;
    player->powerupActivationTime = curTime;
}

void DoUpdatePowerup(Player* player, double curTime)
{
    if (player->powerupDeactivationTime <= curTime) {
        player->abs_vel = PLAYER_SPEED;
        player->activePowerup = -1;
    }
}

void DoDrawPowerup_Shield(float posx, float posy)
{
    float angle;
    float rad;
    double curTime = glfwGetTime();

    rad = PLAYER_RADIUS + 0.4f + sin(curTime * 20.0f) * 0.1f;
    glColor3f(0.0f, sin(curTime * 5.0f) * 0.5f + 0.5f, 1.0f);
    glBegin(GL_LINE_LOOP);
    for (angle = 0.0f; angle < kPI * 2; angle += 0.6f) {
        glVertex2f(posx + cos(angle) * rad, posy + sin(angle) * rad);
    }
    glEnd();

    rad = PLAYER_RADIUS + 0.4f - sin(curTime * 20.0f) * 0.1f;
    glColor3f(cos(curTime * 5.0f) * 0.5f + 0.5f, 0.0f, 1.0f);
    glBegin(GL_LINE_LOOP);
    for (angle = 0.0f; angle < kPI * 2; angle += 0.6f) {
        glVertex2f(posx + cos(angle) * rad, posy + sin(angle) * rad);
    }
    glEnd();
}

void DoDrawPowerup(Player* player)
{
    switch (player->activePowerup) {
    case item_shield:
        DoDrawPowerup_Shield(player->pos.x, player->pos.y);
        break;
    }
}
