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

#ifndef __player__
#define __player__

#include "enet/enet.h"
#include "item.h"
#include "util.h"

#define MAX_PLAYERS 32
#define PLAYER_SPEED 10
#define PLAYER_ROT_SPEED 200
#define PLAYER_NET_UPDATE_TIME 0.05
#define PLAYER_RADIUS 0.6

#define PLAYER_MAX_NAME_LENGTH 64

#define MAX_HUD_MSG_LENGTH 128
#define MSG_DISPLAY_TIME 2.0f
#define MAX_PLAYER_ITEMS 32

typedef struct Player {
    int inUse;

    /* physics */
    Vect2D pos; ///< current position
    Vect2D vel; ///< current velocity
    float rot, vel_rot; ///< current rotation, rotation speed
    float abs_vel; ///< absolute value of possible velocity

    /* stats */
    float health;
    int score;

    /* items */
    int curWeapon;
    int curItem;
    PlayerItem item[MAX_PLAYER_ITEMS];
    int activePowerup;
    double powerupActivationTime;
    double powerupDeactivationTime;

    char playerName[PLAYER_MAX_NAME_LENGTH];

    double lastShotTime;

    /* net data */
    /* server */
    ENetPeer* peer;
    int badID;
} Player;

typedef struct PlayerGlobals {
    Player player[MAX_PLAYERS];
    int myPlayerID;
    int playerCount;

    double lastPlayerUpdate;

    /* HUD stuff */
    double msgStartTime;
    char hudMsg[MAX_HUD_MSG_LENGTH];
    int displayScores;

    /* net data */
    /* client */
    double lastNetUpdate;
} PlayerGlobals;

void DoInitPlayers(void);
void DoRespawnPlayer(int i);
void DoUpdatePlayers(void);
void DoCreatePlayerProjectile(void);
void DoUsePlayerPowerup(void);
void DoMovePlayers(void);
void DoDrawPlayers(void);
void DoFocusCamOnPlayer(void);
void DoDisposePlayers(void);

void DoSetLocalPlayer(int pID);
void PlayerDisconnected(int pID);
void DoHitPlayer(int playerHit, int projOwner, float damage);
void PlayerWasKilled(int playerHit, int shotOwner);
void DoSetPlayerInfo(int pID, char* name);
void DoSetPlayerStats(int pID, int score);
void DoGiveItemToPlayer(int pID, int type);
void DoAddItemToInventory(int type);
void DoActivatePlayerPowerup(int pID, int type);

void DoSwitchPlayerWeapon(void);
void DoSwitchPlayerItem(void);

void DoShowMessage(const char* msg);
void DoDrawHUD(void);

void DoDrawPlayerShape(float x, float y, float rot);

#endif
