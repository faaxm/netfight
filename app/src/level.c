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

#include "level.h"
#include "item.h"
#include "myMath.h"
#include "myopengl.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

void DoLoadLevel(char* levelName, LevelData* levelData)
{
    FILE* levelFile;
    int lineCount;
    int respawnPointCount;
    char cmdCode;

    int curPolyID = 0;
    float fillRed = 0, fillGreen = 0, fillBlue = 0;
    float strokeRed = 1, strokeGreen = 1, strokeBlue = 1;
    float x1, x2, y1, y2;

    DoInitItems();

    levelFile = fopen(levelName, "rb");
    if (!levelFile) {
        printf("Error: Level file not found: %s\n", levelName);
        return;
    }
    lineCount = 0;
    respawnPointCount = 0;
    while (fscanf(levelFile, "%c\n", &cmdCode) > 0) {
        if (cmdCode == 'l') { /* its a line */
            fscanf(levelFile, "%f %f %f %f\n", &x1, &y1, &x2, &y2);
            /* set line poly id */
            levelData->line[lineCount].polyID = curPolyID;
            /* set line positions */
            levelData->line[lineCount].start.x = x1;
            levelData->line[lineCount].start.y = y1;
            levelData->line[lineCount].end.x = x2;
            levelData->line[lineCount].end.y = y2;
            /* set line colors */
            levelData->line[lineCount].stroke.r = strokeRed;
            levelData->line[lineCount].stroke.g = strokeGreen;
            levelData->line[lineCount].stroke.b = strokeBlue;
            levelData->line[lineCount].fill.r = fillRed;
            levelData->line[lineCount].fill.g = fillGreen;
            levelData->line[lineCount].fill.b = fillBlue;
            // printf("Read line: %f	%f	%f	%f\n", x1, y1, x2, y2);

            lineCount++;

            if (lineCount >= MAX_LINES_PER_LEVEL) {
                printf("#ERROR:  Level %s is far too large...\n\n", levelName);
                break;
            }
        } else if (cmdCode == 's') { /* stroke color information */
            fscanf(levelFile, "%f %f %f\n", &strokeRed, &strokeGreen, &strokeBlue);
            strokeRed /= 255;
            strokeGreen /= 255;
            strokeBlue /= 255;
        } else if (cmdCode == 'f') { /* stroke color information */
            fscanf(levelFile, "%f %f %f\n", &fillRed, &fillGreen, &fillBlue);
            fillRed /= 255;
            fillGreen /= 255;
            fillBlue /= 255;
        } else if (cmdCode == 'n') { /* polygon id is given */
            fscanf(levelFile, "%i\n", &curPolyID);
        } else if (cmdCode == 'r') { /* rectangle */
            fscanf(levelFile, "%f %f %f %f\n", &x1, &y1, &x2, &y2);
            if (fillGreen > 0.5f) { /* respawn point */
                levelData->respawnPoint[respawnPointCount].pos.x = x1;
                levelData->respawnPoint[respawnPointCount].pos.y = y1;
                respawnPointCount++;
            } else if (fillBlue > 0.5f) { /* item */
                int itemType;

                itemType = (x2 - 1) * 2 + 0.4f + item_item;
                DoAddItem(x1, y1, itemType);
            } else if (fillRed > 0.5f) { /* weapon */
                int itemType;

                itemType = (x2 - 1) * 2 + 0.4f + item_weapon;
                DoAddItem(x1, y1, itemType);
            }
        }
    }
    levelData->lineCount = lineCount;
    levelData->respawnPointCount = respawnPointCount;
    fclose(levelFile);
}

Vect2D DoGetRespawnPoint(LevelData* levelData)
{
    int pointIndex;

    pointIndex = random() % levelData->respawnPointCount;
    // printf("Index used: %i\n", pointIndex);
    // printf("PointData: %f %f\n\n", levelData->respawnPoint[pointIndex].pos.x,
    // levelData->respawnPoint[pointIndex].pos.y);

    return levelData->respawnPoint[pointIndex].pos;
}

void DoDrawLevel(LevelData* levelData)
{
    int i;
    int lastPolygon = 999;

    glDisable(GL_DEPTH_TEST);

    /* draw the polygons */
    glColor3f(levelData->line[0].fill.r, levelData->line[0].fill.g, levelData->line[0].fill.b);
    glBegin(GL_POLYGON);
    for (i = 0; i < levelData->lineCount; i++) {
        if (levelData->line[i].fill.r >= 0) {
            if (levelData->line[i].polyID != lastPolygon) {
                lastPolygon = levelData->line[i].polyID;
                glEnd();
                glColor3f(levelData->line[i].fill.r, levelData->line[i].fill.g, levelData->line[i].fill.b);
                glBegin(GL_POLYGON);
                glVertex2f(levelData->line[i].start.x, levelData->line[i].start.y);
            }
            glVertex2f(levelData->line[i].end.x, levelData->line[i].end.y);
        }
    }
    glEnd();

    /* draw the lines */
    glBegin(GL_LINES);
    for (i = 0; i < levelData->lineCount; i++) {
        if (levelData->line[i].stroke.r >= 0) {
            glColor3f(levelData->line[i].stroke.r, levelData->line[i].stroke.g, levelData->line[i].stroke.b);
            glVertex2f(levelData->line[i].start.x, levelData->line[i].start.y);
            glVertex2f(levelData->line[i].end.x, levelData->line[i].end.y);
        }
    }
    glEnd();

    /* draw points */
    glPointSize(5.0f);
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_POINTS);
    for (i = 0; i < levelData->lineCount; i++) {
        glVertex2f(levelData->line[i].start.x, levelData->line[i].start.y);
        glVertex2f(levelData->line[i].end.x, levelData->line[i].end.y);
    }
    glEnd();
}

void DoDisposeLevel(LevelData* levelData)
{
}

int checkLevelCollision(Vect2D pos, Vect2D movementVect, float colRad, LevelData* levelData)
{
    float dist;
    int i;
    float movLength, distLength;
    Vect2D lpDistVect, newpos;

    movLength = sqrt(movementVect.x * movementVect.x + movementVect.y * movementVect.y);

    for (i = 0; i < levelData->lineCount; i++) {
        // check normal collision depending on movement vector
        dist = calcRayLineIntersect(pos, movementVect, levelData->line[i].start, levelData->line[i].end);
        if (dist < MYMATH_DIST_INF && dist > 0) {
            dist *= movLength;

            if (dist <= colRad) {
                /*glColor3f(1.0f, 0.0f, 0.0f);
                glBegin(GL_LINES);
                        glVertex2f(pos.x, pos.y);
                        glVertex2f(pos.x + movementVect.x*dist/movLength, pos.y + movementVect.y*dist/movLength);
                glEnd();*/

                return 1;
            }
        }

        // check normal distance to wall
        lpDistVect.x = -(levelData->line[i].start.y - levelData->line[i].end.y);
        lpDistVect.y = levelData->line[i].start.x - levelData->line[i].end.x;
        distLength = sqrt(lpDistVect.x * lpDistVect.x + lpDistVect.y * lpDistVect.y);

        newpos.x = pos.x + movementVect.x;
        newpos.y = pos.y + movementVect.y;

        dist = calcRayLineIntersect(newpos, lpDistVect, levelData->line[i].start, levelData->line[i].end);
        if (dist < MYMATH_DIST_INF) {
            dist *= distLength;
            if (dist < 0)
                dist *= -1;

            if (dist <= colRad) {
                return 1;
            }
        }
    }

    return 0;
}
