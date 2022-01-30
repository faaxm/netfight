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

#include "projectile.h"
#include "GLFW/glfw3.h"
#include "item.h"
#include "level.h"
#include "main.h"
#include "myMath.h"
#include "myopengl.h"
#include "net_client.h"
#include "player.h"
#include "util.h"
#include <math.h>
#include <stdio.h>

extern Globals g;
extern PlayerGlobals pg;
ProjectileGlobals wpg;

void DoInitProjectiles(void)
{
    int i;

    for (i = 0; i < MAX_PROJECTILES; i++) {
        wpg.projectile[i].inUse = 0;
    }

    wpg.lastUpdate = glfwGetTime();
}

void DoCreateProjectile(Vect2D* pos, float* angle, int* type)
{
    DoSendProjectileStart(*pos, *angle, *type);
}

void DoAddShotProjectiles(Vect2D pos, float angle, int type, int playerID)
{
    Vect2D pDir;
    float a;

    angle *= kPI / 180;

    switch (type) {
    case item_weapon_basic:
        pDir.x = cos(angle);
        pDir.y = sin(angle);
        DoAddProjectile(pos, pDir, type, playerID);
        break;

    case item_weapon_spread:
        for (a = angle - 0.5f; a < angle + 0.5f; a += 0.1f) {
            pDir.x = cos(a);
            pDir.y = sin(a);
            DoAddProjectile(pos, pDir, type, playerID);
        }
        break;

    case item_weapon_fast:
        for (a = angle - 0.15f; a < angle + 0.16f; a += 0.15f) {
            pDir.x = cos(a);
            pDir.y = sin(a);
            DoAddProjectile(pos, pDir, type, playerID);
        }
        break;

    case item_weapon_mine:
        pDir.x = 0.0f;
        pDir.y = 0.0f;
        DoAddProjectile(pos, pDir, type, playerID);
        break;

    case item_weapon_mine_expl:
        for (a = 0.0f; a < 2 * kPI; a += 0.05f) {
            pDir.x = cos(a);
            pDir.y = sin(a);
            DoAddProjectile(pos, pDir, type, playerID);
        }
        break;
    }
}

void DoAddProjectile(Vect2D pos, Vect2D vel, int type, int playerID)
{
    int i;

    // printf("Add projectile at: %f %f vec: %f %f\n", pos.x, pos.y, vel.x, vel.y);

    for (i = 0; i < MAX_PROJECTILES; i++) {
        if (!wpg.projectile[i].inUse) {
            wpg.projectile[i].inUse = 1;
            wpg.projectile[i].pos = pos;
            wpg.projectile[i].vel = vel;
            wpg.projectile[i].type = type;
            wpg.projectile[i].playerID = playerID;

            wpg.projectile[i].timeShot = glfwGetTime();

            /* init to some defaults */
            wpg.projectile[i].size = 0.0f;
            wpg.projectile[i].hit_func = (projectile_hit_func)projectile_hit;
            wpg.projectile[i].draw_func = (projectile_draw_func)DoDrawBasicProjectile;
            wpg.projectile[i].update_func = (projectile_update_func)projectile_update;
            wpg.projectile[i].visibleForPlayerID = -1;
            wpg.projectile[i].damage = 5;

            switch (type) {
            case item_weapon_basic:
                wpg.projectile[i].vel.x *= 20;
                wpg.projectile[i].vel.y *= 20;
                wpg.projectile[i].lifetime = 1.0;
                break;

            case item_weapon_spread:
                wpg.projectile[i].vel.x *= 15;
                wpg.projectile[i].vel.y *= 15;
                wpg.projectile[i].lifetime = 2.0;
                break;

            case item_weapon_fast:
                wpg.projectile[i].vel.x *= 20;
                wpg.projectile[i].vel.y *= 20;
                wpg.projectile[i].lifetime = 0.3;
                wpg.projectile[i].draw_func = (projectile_draw_func)projectile_fast_draw;
                break;

            case item_weapon_mine:
                wpg.projectile[i].lifetime = 120.0f;
                wpg.projectile[i].size = 4.0f;
                wpg.projectile[i].draw_func = (projectile_draw_func)DoDrawMineProjectile;
                wpg.projectile[i].hit_func = (projectile_hit_func)projectile_mine_hit;
                wpg.projectile[i].update_func = (projectile_update_func)projectile_mine_update;
                wpg.projectile[i].visibleForPlayerID = playerID;
                break;

            case item_weapon_mine_expl:
                wpg.projectile[i].vel.x *= 20;
                wpg.projectile[i].vel.y *= 20;
                wpg.projectile[i].lifetime = 0.5;
                wpg.projectile[i].damage = 10;
                wpg.projectile[i].draw_func = (projectile_draw_func)projectile_mine_draw;
                break;

            default:
                printf("DoAddProjectile: unknown projectile type!\n");
                break;
            }

            break;
        }
    }
}

void DoUpdateProjectiles(void)
{
    double curTime;
    double movScale;
    int i;
    int playerHit;
    Vect2D movVect, oldVect;

    curTime = glfwGetTime();
    movScale = curTime - wpg.lastUpdate;
    wpg.lastUpdate = curTime;

    for (i = 0; i < MAX_PROJECTILES; i++) {
        if (wpg.projectile[i].inUse) {
            movVect.x = wpg.projectile[i].vel.x * movScale;
            movVect.y = wpg.projectile[i].vel.y * movScale;

            /* does the projectile collide with a wall? */
            if (checkLevelCollision(wpg.projectile[i].pos, movVect, 0.2f, &g.levelData)) {
                wpg.projectile[i].inUse = 0;
            }
            /* save old projectile position */
            oldVect = wpg.projectile[i].pos;
            /* move the projectile */
            wpg.projectile[i].pos.x += wpg.projectile[i].vel.x * movScale;
            wpg.projectile[i].pos.y += wpg.projectile[i].vel.y * movScale;

            /* check for player collision */
            playerHit = 0; // the playerID of the player beeing hit...
            while (playerHit >= 0) {
                playerHit = DoCheckProjectilePlayerCollision(
                    playerHit, oldVect, wpg.projectile[i].pos, wpg.projectile[i].size);
                if (playerHit >= 0
                    && (playerHit != wpg.projectile[i].playerID || wpg.projectile[i].type >= item_weapon_mine)) {
                    /* call the hit_func and reset the inUse state depending on its return value */
                    if (wpg.projectile[i].hit_func) {
                        wpg.projectile[i].hit_func(&wpg.projectile[i], playerHit);
                    }
                }
                if (playerHit >= 0)
                    playerHit++;
            }
            /* call the update func */
            wpg.projectile[i].update_func(&wpg.projectile[i], curTime);
        }
    }
}

void projectile_hit(Projectile* proj, int playerID)
{
    DoHitPlayer(playerID, proj->playerID, proj->damage);
    proj->inUse = 0;
}

void projectile_mine_hit(Projectile* proj, int playerID)
{
    if (proj->visibleForPlayerID != -1 && proj->timeShot + 3.0f < glfwGetTime()) {
        /* this projectile should now be visible for everyone */
        proj->visibleForPlayerID = -1;
        /* set a new shot time so that we can wait some time before
        we fire the explosion projectiles
        */
        proj->timeShot = glfwGetTime();
    }
}

void projectile_update(Projectile* proj, double curTime)
{
    if (curTime - proj->timeShot > proj->lifetime) {
        proj->inUse = 0;
    }
}

void projectile_mine_update(Projectile* proj, double curTime)
{
    /* see if we are in explosion mode */
    if (proj->visibleForPlayerID == -1) {
        /* shall we explode? */
        if (curTime - proj->timeShot > PROJ_MINE_EXPL_DELAY) {
            proj->inUse = 0;
            DoAddShotProjectiles(proj->pos, 0.0f, item_weapon_mine_expl, proj->playerID);
        }
    }
}

void DoDrawProjectiles(void)
{
    int i;
    double curTime;

    curTime = glfwGetTime();

    glBegin(GL_LINES);
    for (i = 0; i < MAX_PROJECTILES; i++) {
        if (wpg.projectile[i].inUse) {
            glColor3f(0.5f, 0.5f, 1.0f);
            if (wpg.projectile[i].visibleForPlayerID == pg.myPlayerID || wpg.projectile[i].visibleForPlayerID == -1) {
                wpg.projectile[i].draw_func(
                    &wpg.projectile[i], wpg.projectile[i].pos.x, wpg.projectile[i].pos.y, curTime);
            }
        }
    }
    glEnd();
}

void projectile_fast_draw(Projectile* proj, float posx, float posy, double curTime)
{
    float color_r;
    float color_g;
    float color_b;

    color_r = 1.0f;
    color_g = (cos((curTime - proj->timeShot) * 10) + 1) / 4;
    color_b = 0.0f;

    glColor3f(color_r, color_g, color_b);
    DoDrawBasicProjectile(proj, posx, posy, curTime);
}

void projectile_mine_draw(Projectile* proj, float posx, float posy, double curTime)
{
    static float curColor = 0.0f;
    static double lastUpdate = 0.0;
    float color_r;
    float color_g;
    float color_b;

    /* calculate the color */
    color_r = cos(curColor) * 0.5f + 0.5f;
    color_g = 0.2f;
    color_b = sin(curColor) * 0.5f + 0.5f;

    /* should we change the color? */
    if (curTime - lastUpdate > 0.05f) {
        curColor += 0.3f;
        if (curColor > 2 * kPI)
            curColor -= 2 * kPI;
        lastUpdate = curTime;
    }

    glColor3f(color_r, color_g, color_b);
    DoDrawBasicProjectile(proj, posx, posy, curTime);
}

void DoDrawBasicProjectile(Projectile* proj, float posx, float posy, double curTime)
{
    glVertex2f(posx, posy - 0.1f);
    glVertex2f(posx, posy + 0.1f);
    glVertex2f(posx - 0.1f, posy);
    glVertex2f(posx + 0.1f, posy);
}

void DoDrawMineProjectile(Projectile* proj, float posx, float posy, double curTime)
{
    if (proj->timeShot + 3.0f < curTime || proj->visibleForPlayerID == -1) {
        glColor3f(1.0f, sin(curTime * 10.0f) * 0.5f + 0.5f, 0.0f);
    } else {
        glColor3f(0.0f, 1.0f, 0.0f);
    }

    glVertex2f(posx - 0.3f, posy - 0.3f);
    glVertex2f(posx + 0.3f, posy - 0.3f);

    glVertex2f(posx + 0.3f, posy - 0.3f);
    glVertex2f(posx + 0.3f, posy + 0.3f);

    glVertex2f(posx + 0.3f, posy + 0.3f);
    glVertex2f(posx - 0.3f, posy + 0.3f);

    glVertex2f(posx - 0.3f, posy + 0.3f);
    glVertex2f(posx - 0.3f, posy - 0.3f);

    glVertex2f(posx, posy - 0.1f);
    glVertex2f(posx, posy + 0.1f);
    glVertex2f(posx - 0.1f, posy);
    glVertex2f(posx + 0.1f, posy);
}

void DoDrawBombProjectile(Projectile* proj, float posx, float posy, double curTime)
{
}

int DoCheckProjectilePlayerCollision(int startI, Vect2D p1, Vect2D p2, float pSize)
{
    float dist1, dist2;
    int i;

    if (startI < 0)
        return startI;

    for (i = startI; i < pg.playerCount; i++) {
        if (pg.player[i].inUse) {
            /*dist1 = vectDist(pg.player[i].pos, p1);
            dist2 = vectDist(pg.player[i].pos, p2);
            if(dist1 < (PLAYER_RADIUS+pSize) || dist2 < (PLAYER_RADIUS+pSize)){
                    return i;
            }*/
            if (lineSphereIntersection(p1, p2, pg.player[i].pos, PLAYER_RADIUS + pSize)) {
                return i;
            }
        }
    }

    return -1;
}
