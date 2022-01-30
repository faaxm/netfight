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

#include "net_client.h"
#include "item.h"
#include "netPackets.h"
#include "player.h"
#include "projectile.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

ClientGlobals cg;
extern PlayerGlobals pg;

void DoConnectToServer(char* serverName)
{
    ENetAddress address;
    ENetEvent event;

    /* client setup */
    cg.client = enet_host_create(NULL /* create a client host */, 1 /* only allow 1 outgoing connection */, 0,
        57600 / 8 /* 56K modem with 56 Kbps downstream bandwidth */,
        57600 / 8 /* 56K modem with 14 Kbps upstream bandwidth */);

    if (cg.client == NULL) {
        printf("An error occurred while trying to create an ENet client host.\n");
        exit(EXIT_FAILURE);
    }

    /* connect to the server */
    enet_address_set_host(&address, serverName);
    address.port = SERVER_PORT;

    /* Initiate the connection, allocating the two channels 0 and 1. */
    cg.server = enet_host_connect(cg.client, &address, 2, 0);

    if (cg.server == NULL) {
        fprintf(stderr, "No available peers for initiating an ENet connection.\n");
        exit(EXIT_FAILURE);
    }

    /* Wait up to 5 seconds for the connection attempt to succeed. */
    if (enet_host_service(cg.client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        puts("Connection to the server succeeded.\n");
    } else {
        /* Either the 5 seconds are up or a disconnect event was */
        /* received. Reset the peer in the event the 5 seconds   */
        /* had run out without any significant event.            */
        enet_peer_reset(cg.server);

        printf("Connection to the server (%s) failed.\n", serverName);
        exit(EXIT_FAILURE);
    }
}

void DoDisconnectFromServer(void)
{
    enet_host_destroy(cg.client);
}

void DoSendPlayerPos(Vect2D pos, Vect2D vel, float rot, float vel_rot)
{
    NetPack_PlayerPos pkt;

    pkt.type = pkt_PlayerPos;
    pkt.playerID = pg.myPlayerID;
    pkt.posX = pos.x * NET_PLAYER_POS_SCALE;
    pkt.posY = pos.y * NET_PLAYER_POS_SCALE;
    pkt.velX = vel.x * NET_PLAYER_POS_SCALE;
    pkt.velY = vel.y * NET_PLAYER_POS_SCALE;
    pkt.rot = rot * NET_PLAYER_POS_SCALE;

    /* Create a reliable packet of size 7 containing "packet" */
    ENetPacket* packet = enet_packet_create(&pkt, sizeof(NetPack_PlayerPos),
        /*ENET_PACKET_FLAG_RELIABLE*/ 0);

    /* Send the packet to the peer over channel id 0. */
    /* One could also broadcast the packet by         */
    /* enet_host_broadcast (host, 0, packet);         */
    enet_peer_send(cg.server, 0, packet);
    /* One could just use enet_host_service() instead. */
    enet_host_flush(cg.client);
}

void DoSendProjectileStart(Vect2D pos, float angle, int type)
{
    NetPack_PlayerAction pkt;

    pkt.type = pkt_PlayerAction;
    pkt.playerID = pg.myPlayerID;
    pkt.posX = pos.x * NET_PLAYER_POS_SCALE;
    pkt.posY = pos.y * NET_PLAYER_POS_SCALE;
    pkt.angle = angle * NET_PLAYER_POS_SCALE;
    pkt.actionType = pkt_action_shot1;
    pkt.subType = type;

    /* Create a reliable packet of size 7 containing "packet" */
    ENetPacket* packet = enet_packet_create(&pkt, sizeof(NetPack_PlayerAction), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(cg.server, 0, packet);
    enet_host_flush(cg.client);
}

void DoSendPlayerAction(int type)
{
    NetPack_PlayerAction pkt;

    pkt.type = pkt_PlayerAction;
    pkt.playerID = pg.myPlayerID;
    pkt.posX = 0;
    pkt.posY = 0;
    pkt.angle = 0;
    pkt.actionType = pkt_action_shot1;
    pkt.subType = type;

    /* Create a reliable packet of size 7 containing "packet" */
    ENetPacket* packet = enet_packet_create(&pkt, sizeof(NetPack_PlayerAction), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(cg.server, 0, packet);
    enet_host_flush(cg.client);
}

void DoSendPlayerKilled(int killedPlayer, int shotOwner)
{
    NetPack_PlayerKilled pkt;

    pkt.type = pkt_PlayerKilled;
    pkt.killedPlayer = killedPlayer;
    pkt.shotOwner = shotOwner;

    /* Create a reliable packet of size 7 containing "packet" */
    ENetPacket* packet = enet_packet_create(&pkt, sizeof(NetPack_PlayerKilled), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(cg.server, 0, packet);
    enet_host_flush(cg.client);
}

void DoSendPlayerStats(int playerID, int score)
{
    NetPack_PlayerStats pkt;

    pkt.type = pkt_PlayerStats;
    pkt.playerID = playerID;
    pkt.score = score;

    /* Create a reliable packet of size 7 containing "packet" */
    ENetPacket* packet = enet_packet_create(&pkt, sizeof(NetPack_PlayerStats), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(cg.server, 0, packet);
    enet_host_flush(cg.client);
}

void DoSendPlayerGotItem(int playerID, int itemID)
{
    NetPack_PlayerGotItem pkt;

    pkt.type = pkt_PlayerGotItem;
    pkt.playerID = playerID;
    pkt.itemID = itemID;

    /* Create a reliable packet of size 7 containing "packet" */
    ENetPacket* packet = enet_packet_create(&pkt, sizeof(NetPack_PlayerGotItem), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(cg.server, 0, packet);
    enet_host_flush(cg.client);
}

void DoSendPlayerInfo(int playerID, char* name)
{
    NetPack_PlayerInfo pkt;

    pkt.type = pkt_PlayerInfo;
    pkt.playerID = playerID;
    strncpy(pkt.name, name, MAX_PLAYER_NAME);

    /* Create a reliable packet of size 7 containing "packet" */
    ENetPacket* packet = enet_packet_create(&pkt, sizeof(NetPack_PlayerInfo), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(cg.server, 0, packet);
    enet_host_flush(cg.client);
}

void DoGetNetworkData(void)
{
    ENetEvent event;

    while (enet_host_service(cg.client, &event, 10) > 0) {
        switch (event.type) {
        case ENET_EVENT_TYPE_RECEIVE:
            DoHandleNetPacket(event.packet->data);
            /* Clean up the packet now that we're done using it. */
            enet_packet_destroy(event.packet);

            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            printf("Got disconected.\n");
            break;
        }
    }
}

void DoHandleNetPacket(void* data)
{
    NetPack_Unknown* pkt;

    pkt = (NetPack_Unknown*)data;
    switch (pkt->type) {
    case pkt_PlayerPos: {
        NetPack_PlayerPos* npkt = (NetPack_PlayerPos*)data;

        // printf("Received player data update\n");

        if (npkt->playerID < MAX_PLAYERS && npkt->playerID != pg.myPlayerID) {
            int pID = npkt->playerID;
            if (pID != 0) {
                if (pID >= pg.playerCount) {
                    pg.playerCount = pID + 1;
                    // printf("new player detected. ID: %i\n", (int)pID);
                }
                pg.player[pID].pos.x = (float)npkt->posX / NET_PLAYER_POS_SCALE;
                pg.player[pID].pos.y = (float)npkt->posY / NET_PLAYER_POS_SCALE;
                pg.player[pID].vel.x = (float)npkt->velX / NET_PLAYER_POS_SCALE;
                pg.player[pID].vel.y = (float)npkt->velY / NET_PLAYER_POS_SCALE;
                pg.player[pID].rot = (float)npkt->rot / NET_PLAYER_POS_SCALE;

                // player has to be visible
                pg.player[pID].inUse = 1;
            }
        }
    } break;

    case pkt_PlayerID: {
        NetPack_IDUpdate* npkt = (NetPack_IDUpdate*)data;

        DoSetLocalPlayer(npkt->playerID);
        /*printf("Received my new player id: %i\n", (int)npkt->playerID);
        pg.player[pg.myPlayerID].drawPlayer = 0;
        pg.myPlayerID = npkt->playerID;
        pg.player[pg.myPlayerID].drawPlayer = 1;
        if(pg.myPlayerID >= pg.playerCount) pg.playerCount = pg.myPlayerID + 1;*/
    } break;

    case pkt_PlayerAction: {
        NetPack_PlayerAction* npkt = (NetPack_PlayerAction*)data;
        Vect2D pos;
        float angle;

        pos.x = (float)npkt->posX / NET_PLAYER_POS_SCALE;
        pos.y = (float)npkt->posY / NET_PLAYER_POS_SCALE;
        angle = (float)npkt->angle / NET_PLAYER_POS_SCALE;

        /* decide if it is a shot or an item */
        if (npkt->subType >= item_item) {
            DoActivatePlayerPowerup(npkt->playerID, npkt->subType);
        } else {
            DoAddShotProjectiles(pos, angle, npkt->subType, npkt->playerID);
        }
    } break;

    case pkt_PlayerKilled: {
        NetPack_PlayerKilled* npkt = (NetPack_PlayerKilled*)data;
        PlayerWasKilled(npkt->killedPlayer, npkt->shotOwner);
    } break;

    case pkt_PlayerInfo: {
        NetPack_PlayerInfo* npkt = (NetPack_PlayerInfo*)data;
        DoSetPlayerInfo(npkt->playerID, npkt->name);
    } break;

    case pkt_Welcome: {
        DoSendPlayerInfo(pg.myPlayerID, pg.player[pg.myPlayerID].playerName);
        DoSendPlayerStats(pg.myPlayerID, pg.player[pg.myPlayerID].score);
    } break;

    case pkt_PlayerStats: {
        NetPack_PlayerStats* npkt = (NetPack_PlayerStats*)data;
        DoSetPlayerStats(npkt->playerID, npkt->score);
    } break;

    case pkt_PlayerGotItem: {
        NetPack_PlayerGotItem* npkt = (NetPack_PlayerGotItem*)data;
        if (npkt->playerID != pg.myPlayerID)
            DoGiveItem(npkt->playerID, npkt->itemID);
    } break;

    case pkt_NetInfo: {
        NetPack_NetInfo* npkt = (NetPack_NetInfo*)data;
        switch (npkt->msgType) {
        case pkt_net_PlayerGone:
            PlayerDisconnected(npkt->playerID);
            break;
        }
    } break;

    case pkt_MapUpdate: {
        NetPack_MapUpdate* npkt = (NetPack_MapUpdate*)data;
        switch (npkt->msgType) {
        case pkt_map_newItem:
            DoActivateItem(npkt->code);
            break;
        }
    } break;
    }
}
