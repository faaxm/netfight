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

#ifndef _projectile_
#define _projectile_

#include "util.h"

#define MAX_PROJECTILES 512
#define MAX_PROJECTILE_TIME 5.0

#define PROJ_MINE_EXPL_DELAY 0.7

typedef void (*projectile_draw_func)(void* proj, float x, float y, double curTime);
typedef int (*projectile_hit_func)(void* proj, int playerID);
typedef void (*projectile_update_func)(void* proj, double curTime);

typedef struct Projectile {
    int inUse; ///< is this projectile active?
    Vect2D pos; ///< position of the projectile
    Vect2D vel; ///< velocity of the projectile
    int type; ///< type of the projectile
    int playerID; ///< ID of the player who fired it
    double timeShot; ///< the time at which the projectile was shot
    double lifetime; ///< how long should this projectile life?
    float size; ///< radius of the projectile
    int visibleForPlayerID; ///< which player can see this projectile? (-1 = everybody)
    float damage; ///< amount of damage inflicted on a hit
    /* callbacks */
    projectile_draw_func draw_func;
    projectile_hit_func hit_func;
    projectile_update_func update_func;
} Projectile;

typedef struct ProjectileGlobals {
    Projectile projectile[MAX_PROJECTILES];
    double lastUpdate;
} ProjectileGlobals;

void DoInitProjectiles(void);
void DoCreateProjectile(Vect2D* pos, float* angle, int* type);
void DoAddShotProjectiles(Vect2D pos, float angle, int type, int playerID);
void DoAddProjectile(Vect2D pos, Vect2D vel, int type, int playerID);
void DoUpdateProjectiles(void);

void projectile_hit(Projectile* proj, int playerID);
void projectile_mine_hit(Projectile* proj, int playerID);
void projectile_update(Projectile* proj, double curTime);
void projectile_mine_update(Projectile* proj, double curTime);

void DoDrawProjectiles(void);

void projectile_fast_draw(Projectile* proj, float posx, float posy, double curTime);
void projectile_mine_draw(Projectile* proj, float posx, float posy, double curTime);
void DoDrawBasicProjectile(Projectile* proj, float posx, float posy, double curTime);
void DoDrawMineProjectile(Projectile* proj, float posx, float posy, double curTime);
void DoDrawBombProjectile(Projectile* proj, float posx, float posy, double curTime);

int DoCheckProjectilePlayerCollision(int startI, Vect2D p1, Vect2D p2, float pSize);

#endif
