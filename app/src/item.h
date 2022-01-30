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

#ifndef _item_
#define _item_

#include "util.h"

#define MAX_ITEMS_PER_LEVEL 256
#define ITEM_RADIUS 0.4

#define ITEM_HEALTH_BONUS 30

enum {
    item_weapon_basic = 0,
    item_weapon = 1,
    item_weapon_spread = item_weapon,
    item_weapon_fast,
    item_weapon_bomb,
    item_weapon_mine,
    item_mine2,

    item_item = 10,
    item_health = item_item,
    item_shield,
    item_super_fast,

    item_max,

    /* these are for internal use only!!! */
    item_weapon_mine_expl,

    item_max_max
};

typedef struct PlayerItem {
    int type;
    int ammo;
    double reloadTime;
} PlayerItem;

typedef struct LevelItem {
    int inUse;
    int type;
    Vect2D pos;
} LevelItem;

typedef struct ItemGlobals {
    LevelItem levelItem[MAX_ITEMS_PER_LEVEL];
    int levelItemCount;
} ItemGlobals;

void DoInitItems(void);
void DoAddItem(float x, float y, int type);
void DoActivateItem(int seed);
void DoDisposeItems(void);
void DoHandlePlayerItemCollision(void);
void DoDisableItem(int itemID);
void DoGiveItem(int playerID, int itemID);
void DoDrawItems(void);

#endif
