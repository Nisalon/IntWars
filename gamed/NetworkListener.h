/*
IntWars playground server for League of Legends protocol testing
Copyright (C) 2012  Intline9 <Intline9@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef _NETWORK_LISTENER_H
#define _NETWORK_LISTENER_H

#include <enet/enet.h>
#include <intlib/base64.h>
#include <intlib/blowfish.h>

#include "common.h"
#include "PacketHandler.h"

#define PEER_MTU 996
class NetworkListener
{
	public:
		NetworkListener();
		~NetworkListener();

		bool initialize(ENetAddress *address, const char *baseKey);
		void netLoop();

	private:
		bool _isAlive;
		PacketHandler *_handler;
		ENetHost *_server;
		BlowFish *_blowfish;
};

#endif

