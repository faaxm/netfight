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

#ifndef _level_
#define _level_

#include "util.h"

#define MAX_LINES_PER_LEVEL 512
#define MAX_RESPAWN_POINTS 128

typedef struct LevelLine {
    Vect2D start;
    Vect2D end;
    int polyID;

    ColorRGB fill, stroke;
} LevelLine;

typedef struct RespawnPoint {
    Vect2D pos;
} RespawnPoint;

typedef struct LevelData {
    LevelLine line[MAX_LINES_PER_LEVEL];
    int lineCount;
    RespawnPoint respawnPoint[MAX_RESPAWN_POINTS];
    int respawnPointCount;
} LevelData;

void DoLoadLevel(char* levelName, LevelData* levelData);
Vect2D DoGetRespawnPoint(LevelData* levelData);
void DoDrawLevel(LevelData* levelData);
void DoDisposeLevel(LevelData* levelData);
int checkLevelCollision(Vect2D pos, Vect2D movementVect, float colRad, LevelData* levelData);

#endif
