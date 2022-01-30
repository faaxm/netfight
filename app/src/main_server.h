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

#ifndef _server_
#define _server_

#include "enet/enet.h"
#include "netPackets.h"

#define SERVER_MAX_INPUT_LENGTH 256
#define SERVER_ITEM_UPDATE_TIME 5.0f

typedef struct ServerGlobals {
    ENetHost* server;
    double lastIDUpdate;
    char inputBuffer[SERVER_MAX_INPUT_LENGTH];
    int inputPos;

    double lastItemUpdate;
} ServerGlobals;

void RunServer(void);

#endif
