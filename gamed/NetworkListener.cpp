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
#include "stdafx.h"
#include "NetworkListener.h"
#include "Log.h"

NetworkListener::NetworkListener()
{

}

NetworkListener::~NetworkListener()
{
	_isAlive = false;

	delete _handler;
	delete _blowfish;
	enet_host_destroy(_server);
}

bool NetworkListener::initialize(ENetAddress *address, const char *baseKey)
{
	if (enet_initialize () != 0)
		return false;
	atexit(enet_deinitialize);

	_server = enet_host_create(address, 32, 0, 0);
	if(_server == NULL)
		return false;

	std::string key = base64_decode(baseKey);
	if(key.length() <= 0)
		return false;
	_blowfish = new BlowFish((uint8*)key.c_str(), 16);
	_handler = new PacketHandler(_server, _blowfish);
	
	return _isAlive = true;
}

void NetworkListener::netLoop()
{
	ENetEvent event;

	while(enet_host_service(_server, & event, 10) >= 0 && _isAlive)
	{
		switch (event.type)
		{
			case ENET_EVENT_TYPE_CONNECT:
				Logging->writeLine("A new client connected connected\n");

				/* Set some defaults */
				event.peer->mtu = PEER_MTU;

				event.peer->data = new ClientInfo();
				peerInfo(event.peer)->setName("IntWars");
				peerInfo(event.peer)->setType("Jayce");
				peerInfo(event.peer)->skinNo = 1;
				peerInfo(event.peer)->netId = 0x40000019;

			break;

		case ENET_EVENT_TYPE_RECEIVE:
			if(!_handler->handlePacket(event.peer, event.packet,event.channelID))
			{
				//enet_peer_disconnect(event.peer, 0);
			}

			/* Clean up the packet now that we're done using it. */
			enet_packet_destroy (event.packet);
		break;

		case ENET_EVENT_TYPE_DISCONNECT:
			Logging->writeLine("Peer disconnected.\n");

			/* Cleanup */
			delete (ClientInfo*)event.peer->data;
		break;
		}
	}
}