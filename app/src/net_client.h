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

#ifndef _netclient_
#define _netclient_

#include "enet/enet.h"
#include "util.h"

#define NET_PLAYER_POS_SCALE 50

typedef struct ClientGlobals {
    ENetHost* client;
    ENetPeer* server;
} ClientGlobals;

void DoConnectToServer(char* serverName);
void DoDisconnectFromServer(void);

void DoSendPlayerPos(Vect2D pos, Vect2D vel, float rot, float vel_rot);
void DoSendProjectileStart(Vect2D pos, float angle, int type);
void DoSendPlayerAction(int type);
void DoSendPlayerKilled(int killedPlayer, int shotOwner);
void DoSendPlayerStats(int playerID, int score);
void DoSendPlayerGotItem(int playerID, int itemID);
void DoSendPlayerInfo(int playerID, char* name);

void DoGetNetworkData(void);
void DoHandleNetPacket(void* data);

#endif
