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

#include "main_server.h"
#include "netPackets.h"
#include "player.h"

#include <GLFW/glfw3.h>
#include <enet/enet.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define kBacklog 10
#define SERVER_MAX_PACKET_SIZE 128

ServerGlobals sg;
PlayerGlobals spg;

void SrvProcessEvents(void);
void SrvUpdatePlayerIDs(void);
void SrvGetInput(void);
void SrvProcessCommand(char* cmd);
void SrvUpdateMapState(void);

int SrvAddPlayer(ENetPeer* peer);
void SrvWelcomeNewPlayer(int pID);
void SrvPlayerDisconnected(int pID);

void RunServer(void)
{
    ENetAddress address;
    int i;
    long timeVal;

    time(&timeVal);
    srandom(timeVal);

    sg.lastItemUpdate = sg.lastIDUpdate = glfwGetTime();

    /* init players */
    for (i = 0; i < MAX_PLAYERS; i++) {
        spg.player[i].inUse = 0;
    }
    /* init cmd line input buffer */
    sg.inputPos = 0;

    /* Bind the server to the default localhost.     */
    /* A specific host address can be specified by   */
    /* enet_address_set_host (& address, "x.x.x.x"); */

    address.host = ENET_HOST_ANY;
    /* Bind the server to port 1234. */
    address.port = SERVER_PORT;

    sg.server = enet_host_create(&address /* the address to bind the server host to */,
        MAX_PLAYERS /* allow up to 32 clients and/or outgoing connections */, 0,
        0 /* assume any amount of incoming bandwidth */, 0 /* assume any amount of outgoing bandwidth */);
    if (sg.server == NULL) {
        fprintf(stderr, "An error occurred while trying to create an ENet server host.\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        SrvProcessEvents();
        SrvUpdatePlayerIDs();
        SrvGetInput();
        SrvUpdateMapState();
    }
}

void SrvProcessEvents(void)
{
    ENetEvent event;
    int playerID;

    /* Wait up to 1000 milliseconds for an event. */
    while (enet_host_service(sg.server, &event, 3) > 0) {
        switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT:
            printf("A new client connected from %x:%u.\n", event.peer->address.host, event.peer->address.port);
            playerID = SrvAddPlayer(event.peer);
            /* Store any relevant client information here. */
            event.peer->data = malloc(sizeof(int));
            *(int*)event.peer->data = playerID;

            break;

        case ENET_EVENT_TYPE_RECEIVE:
            // printf("Packet received.\n");
            /*printf ("A packet of length %u containing %s was received from %s on channel %u.\n",
        event.packet -> dataLength,
        event.packet -> data,
        event.peer -> data,
        event.channelID);*/
            /* forward the packet to all clients */
            enet_host_broadcast(sg.server, 0, event.packet);
            enet_host_flush(sg.server);
            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            // printf ("%s disconected.\n", event.peer -> data);
            playerID = *(int*)event.peer->data;
            SrvPlayerDisconnected(playerID);
            /* Reset the peer's client information. */
            if (event.peer->data)
                free(event.peer->data);
            event.peer->data = NULL;
        }
    }
}

void SrvUpdatePlayerIDs(void)
{
    int i, err;
    NetPack_IDUpdate pkt;

    pkt.type = pkt_PlayerID;

    for (i = 0; i < spg.playerCount; i++) {
        if (spg.player[i].badID) {
            pkt.playerID = i;

            /* send update packet */
            /* Create a reliable packet of size 7 containing "packet" */
            ENetPacket* packet = enet_packet_create(&pkt, sizeof(NetPack_IDUpdate), ENET_PACKET_FLAG_RELIABLE);

            /* Send the packet to the peer over channel id 0. */
            /* One could also broadcast the packet by         */
            /* enet_host_broadcast (host, 0, packet);         */
            err = enet_peer_send(spg.player[i].peer, 1, packet);
            /* One could just use enet_host_service() instead. */
            enet_host_flush(sg.server);

            // printf("Sending id to player: %i with error: %i\n", (int)i, err);

            SrvWelcomeNewPlayer(i);

            spg.player[i].badID = 0;
        }
    }
}

void SrvGetInput(void)
{
    int lastChar;

    return;

    while (lastChar = getchar() != EOF) {
        printf(":%c", (unsigned char)lastChar);
        if (lastChar == '\n') {
            sg.inputBuffer[sg.inputPos] = 0;
            SrvProcessCommand(sg.inputBuffer);
            sg.inputPos = 0;
        } else {
            sg.inputBuffer[sg.inputPos] = lastChar;
            sg.inputPos++;
        }
    }
}

void SrvProcessCommand(char* cmd)
{
    printf("Command entered: %s\n", cmd);
}

void SrvUpdateMapState(void)
{
    double curTime = glfwGetTime();

    if (curTime - sg.lastItemUpdate > SERVER_ITEM_UPDATE_TIME) {
        ENetPacket* packet;
        NetPack_MapUpdate pkt;

        sg.lastItemUpdate = curTime;

        /* send a welcome message to the new player */
        pkt.type = pkt_MapUpdate;
        pkt.msgType = pkt_map_newItem;
        pkt.code = random();

        /* create a packet to send to the all players */
        packet = enet_packet_create(&pkt, sizeof(NetPack_MapUpdate), ENET_PACKET_FLAG_RELIABLE);
        /* send the packet to all players */
        enet_host_broadcast(sg.server, 0, packet);
        enet_host_flush(sg.server);
    }
}

int SrvAddPlayer(ENetPeer* peer)
{
    int pID;
    int i;

    /* get new id */
    pID = -1;
    // Never use player id 0 as that is already used by newly joining players
    for (i = 1; i < MAX_PLAYERS; i++) {
        if (!spg.player[i].inUse) {
            pID = i;
            break;
        }
    }
    if (pID < 0)
        return -1; // no more players allowed...
    if (pID >= spg.playerCount)
        spg.playerCount = pID + 1;

    /* set player peer */
    spg.player[pID].peer = peer;

    /* init player struct */
    spg.player[pID].inUse = 1;

    spg.player[pID].pos.x = 0;
    spg.player[pID].pos.y = 0;
    spg.player[pID].vel.x = 0;
    spg.player[pID].vel.y = 0;
    spg.player[pID].rot = 0;
    /* send the player his id */
    spg.player[pID].badID = 1;

    return pID;
}

void SrvWelcomeNewPlayer(int pID)
{
    ENetPacket* packet;
    NetPack_Welcome welcomeDat;

    /* send a welcome message to the new player */
    welcomeDat.type = pkt_Welcome;
    welcomeDat.playerID = pID;

    /* create a packet to send to the all players */
    packet = enet_packet_create(&welcomeDat, sizeof(NetPack_Welcome), ENET_PACKET_FLAG_RELIABLE);
    /* send the packet to all players */
    enet_host_broadcast(sg.server, 0, packet);
    enet_host_flush(sg.server);
}

void SrvPlayerDisconnected(int pID)
{
    ENetPacket* packet;
    NetPack_NetInfo pkt;

    spg.player[pID].inUse = 0;

    printf("Player %i disconnected\n", pID);

    /* send a welcome message to the new player */
    pkt.type = pkt_NetInfo;
    pkt.msgType = pkt_net_PlayerGone;
    pkt.playerID = pID;

    /* create a packet to send to the all players */
    packet = enet_packet_create(&pkt, sizeof(NetPack_NetInfo), ENET_PACKET_FLAG_RELIABLE);
    /* send the packet to all players */
    enet_host_broadcast(sg.server, 0, packet);
    enet_host_flush(sg.server);
}
