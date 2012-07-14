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
#ifndef _PACKET_HANDLER_H
#define _PACKET_HANDLER_H
#include <enet/enet.h>
#include "common.h"
#include "Log.h"
#include "ChatBox.h"

#include <intlib/base64.h>
#include <intlib/blowfish.h>


#include "Packets.h"
#include "Client.h"

#define RELIABLE ENET_PACKET_FLAG_RELIABLE
#define UNRELIABLE 0

#define HANDLE_ARGS ENetPeer *peer, ENetPacket *packet

class PacketHandler
{
	public:
		PacketHandler(ENetHost *server, BlowFish *blowfish);
		~PacketHandler();

		bool handlePacket(ENetPeer *peer, ENetPacket *packet, uint8 channelID);

		//Handlers
		bool handleNull(HANDLE_ARGS);
		bool handleKeyCheck(HANDLE_ARGS);
		bool handleLoadPing(HANDLE_ARGS);
		bool handleSpawn(HANDLE_ARGS);
		bool handleMap(HANDLE_ARGS);
		bool handleSynch(HANDLE_ARGS);
		bool handleGameNumber(HANDLE_ARGS);
		bool handleQueryStatus(HANDLE_ARGS);
		bool handleStartGame(HANDLE_ARGS);
		bool handleView(HANDLE_ARGS);
		bool handleMove(HANDLE_ARGS);
		bool affirmMove(HANDLE_ARGS);
		bool handleAttentionPing(HANDLE_ARGS);
		bool handleChatBoxMessage(HANDLE_ARGS);
		bool handleSkillUp(HANDLE_ARGS);
		bool handleEmotion(HANDLE_ARGS);

		//Tools
		void printPacket(uint8 *buf, uint32 len);
		void printLine(uint8 *buf, uint32 len);
		bool sendPacket(ENetPeer *peer, uint8 *data, uint32 length, uint8 channelNo, uint32 flag = RELIABLE);
		bool broadcastPacket(uint8 *data, uint32 length, uint8 channelNo, uint32 flag = RELIABLE);
	private:
		void registerHandler(bool (PacketHandler::*handler)(HANDLE_ARGS), PacketCmd pktcmd,Channel c);
	private:
		ENetHost *_server;
		BlowFish *_blowfish;
		bool (PacketHandler::*_handlerTable[0x100][0x7])(HANDLE_ARGS); 
};
#endif