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

#ifndef _netPackets_
#define _netPackets_

#define SERVER_PORT 5723
#define MAX_PLAYER_NAME 32
#define MAX_PLAYER_IDLE_TIME 1

#define pkt_Welcome 'w' // welcomes a new player
#define pkt_PlayerPos 'p' // gives new player position
#define pkt_PlayerStats 's' // gives a player's stats
#define pkt_PlayerAction 'a' // player does something
#define pkt_PlayerLife 'l' // player dies/respawns
#define pkt_PlayerID 'i' // tells a player what id he has
#define pkt_PlayerKilled 'k' // sent if a player was killed
#define pkt_PlayerInfo 'd' // player name, image, etc.
#define pkt_PlayerGotItem 'I' // some player collected some item
#define pkt_NetInfo 'N' // player disconnects, etc.
#define pkt_MapUpdate 'm' // Add items, change map, etc.

#define pkt_action_shot1 1

#define pkt_net_PlayerGone 'd' // a player disconnected

#define pkt_map_newItem 'i' // add a new item to the map

typedef struct NetPack_Unknown {
    char type;
} NetPack_Unknown;

typedef struct NetPack_Welcome {
    char type;
    unsigned char playerID;
} NetPack_Welcome;

typedef struct NetPack_IDUpdate {
    char type;
    unsigned char playerID;
} NetPack_IDUpdate;

typedef struct NetPack_PlayerPos {
    char type;
    unsigned char playerID;
    signed short posX;
    signed short posY;
    signed short velX;
    signed short velY;
    signed short rot;
} NetPack_PlayerPos;

typedef struct NetPack_PlayerAction {
    char type;
    unsigned char playerID;
    char actionType;
    char subType;
    signed short posX;
    signed short posY;
    signed short angle;
} NetPack_PlayerAction;

typedef struct NetPack_PlayerKilled {
    char type;
    unsigned char killedPlayer;
    unsigned char shotOwner;
} NetPack_PlayerKilled;

typedef struct NetPack_PlayerStats {
    char type;
    unsigned char playerID;
    unsigned short score;
} NetPack_PlayerStats;

typedef struct NetPack_PlayerGotItem {
    char type;
    unsigned char playerID;
    unsigned char itemID;
} NetPack_PlayerGotItem;

typedef struct NetPack_PlayerInfo {
    char type;
    unsigned char playerID;
    char name[MAX_PLAYER_NAME];
    // char		imgPath[256];
} NetPack_PlayerInfo;

typedef struct NetPack_NetInfo {
    char type;
    char msgType;
    unsigned char playerID;
} NetPack_NetInfo;

typedef struct NetPack_MapUpdate {
    char type;
    char msgType;
    unsigned long code;
} NetPack_MapUpdate;

#endif
