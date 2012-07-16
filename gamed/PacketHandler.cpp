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
#include "PacketHandler.h"


PacketHandler::PacketHandler(ENetHost *server, BlowFish *blowfish)
{
	_server = server;
	_blowfish = blowfish;
	memset(_handlerTable,0,sizeof(_handlerTable));
	registerHandler(&PacketHandler::handleKeyCheck,        PKT_KeyCheck, CHL_HANDSHAKE);
	registerHandler(&PacketHandler::handleLoadPing,        PKT_C2S_Ping_Load_Info, CHL_C2S);
	registerHandler(&PacketHandler::handleSpawn,           PKT_C2S_CharLoaded, CHL_C2S);
	registerHandler(&PacketHandler::handleMap,             PKT_C2S_ClientReady, CHL_LOADING_SCREEN);
	registerHandler(&PacketHandler::handleSynch,           PKT_C2S_SynchVersion, CHL_C2S);
	registerHandler(&PacketHandler::handleGameNumber,      PKT_C2S_GameNumberReq, CHL_C2S);
	registerHandler(&PacketHandler::handleQueryStatus,     PKT_C2S_QueryStatusReq, CHL_C2S);
	registerHandler(&PacketHandler::handleStartGame,       PKT_C2S_StartGame, CHL_C2S);
	registerHandler(&PacketHandler::handleNull,            PKT_C2S_Exit, CHL_C2S);
	registerHandler(&PacketHandler::handleView,            PKT_C2S_ViewReq, CHL_C2S);
	registerHandler(&PacketHandler::handleNull,            PKT_C2S_Click, CHL_C2S);
	registerHandler(&PacketHandler::handleNull,            PKT_C2S_OpenShop, CHL_C2S);
	registerHandler(&PacketHandler::handleAttentionPing,   PKT_C2S_AttentionPing, CHL_C2S);
	registerHandler(&PacketHandler::handleChatBoxMessage , PKT_ChatBoxMessage, CHL_COMMUNICATION);
	registerHandler(&PacketHandler::handleMove,            PKT_C2S_MoveReq, CHL_C2S);
	registerHandler(&PacketHandler::handleNull,            PKT_C2S_MoveConfirm, CHL_C2S);
	registerHandler(&PacketHandler::handleSkillUp,         PKT_C2S_SkillUp, CHL_C2S);
	registerHandler(&PacketHandler::handleEmotion,         PKT_C2S_Emotion, CHL_C2S);
	registerHandler(&PacketHandler::handleNull,            PKT_C2S_LockCamera, CHL_C2S);
	registerHandler(&PacketHandler::handleNull,            PKT_C2S_StatsConfirm, CHL_C2S);
	
}

PacketHandler::~PacketHandler()
{
	
}

void PacketHandler::registerHandler(bool (PacketHandler::*handler)(HANDLE_ARGS), PacketCmd pktcmd,Channel c)
{
	_handlerTable[pktcmd][c] = handler;
}

void PacketHandler::printPacket(uint8 *buf, uint32 len)
{
	PDEBUG_LOG(Logging,"   ");
	for(uint32 i = 0; i < len; i++)
	{
		PDEBUG_LOG(Logging,"%02X ", static_cast<uint8>(buf[i]));
		if((i+1)%16 == 0)
			PDEBUG_LOG(Logging,"\n   ");
	}
	PDEBUG_LOG(Logging,"\n");
}

void PacketHandler::printLine(uint8 *buf, uint32 len)
{
	for(uint32 i = 0; i < len; i++)
		PDEBUG_LOG(Logging,"%02X ", static_cast<uint8>(buf[i]));
	PDEBUG_LOG(Logging,"\n");
}

bool PacketHandler::sendPacket(ENetPeer *peer, uint8 *data, uint32 length, uint8 channelNo, uint32 flag)
{
	//PDEBUG_LOG_LINE(Logging," Sending packet:\n");
	//if(length < 300)
	//	printPacket(data, length);

	if(length >= 8)
		_blowfish->Encrypt(data, length-(length%8)); //Encrypt everything minus the last bytes that overflow the 8 byte boundary

	ENetPacket *packet = enet_packet_create(data, length, flag);
	if(enet_peer_send(peer, channelNo, packet) < 0)
	{
		PDEBUG_LOG_LINE(Logging,"Warning fail, send!");
		return false;
	}
	return true;
}

bool PacketHandler::broadcastPacket(uint8 *data, uint32 length, uint8 channelNo, uint32 flag)
{
	//PDEBUG_LOG_LINE(Logging," Broadcast packet:\n");
	//printPacket(data, length);

	if(length >= 8)
		_blowfish->Encrypt(data, length-(length%8)); //Encrypt everything minus the last bytes that overflow the 8 byte boundary

	ENetPacket *packet = enet_packet_create(data, length, flag);

	enet_host_broadcast(_server, channelNo, packet);
	return true;
}

bool PacketHandler::handlePacket(ENetPeer *peer, ENetPacket *packet, uint8 channelID)
{
	if(packet->dataLength >= 8)
	{
		if(peerInfo(peer)->keyChecked)
			_blowfish->Decrypt(packet->data, packet->dataLength-(packet->dataLength%8)); //Encrypt everything minus the last bytes that overflow the 8 byte boundary
	}

	PacketHeader *header = reinterpret_cast<PacketHeader*>(packet->data);	
	bool (PacketHandler::*handler)(HANDLE_ARGS) = _handlerTable[header->cmd][channelID];
	
	if(handler)
	{
		return (*this.*handler)(peer,packet);
	}
	else
	{
		PDEBUG_LOG_LINE(Logging,"Unknown packet: CMD %X(%i) CHANNEL %X(%i)\n", header->cmd, header->cmd,channelID,channelID);
		printPacket(packet->data, packet->dataLength);
	}
	return false;	
}