#ifndef _PACKETS_H
#define _PACKETS_H

#include <enet/enet.h>
#include "common.h"
#include "ChatBox.h"

#if defined( __GNUC__ )
#pragma pack(1)
#else
#pragma pack(push,1)
#endif

//@@@@@@@@@@@@@ IMPORTANT @@@@@@@@@@@@@@@//
//The netId is a uint16 id and then uint16 flags, the flags are yet to be fully understood
struct PacketHeader
{
	PacketHeader()
	{
		netId = 0;
	}

	PacketCmd cmd;
	uint32 netId;
};

struct GameHeader
{
	GameHeader()
	{
		netId = ticks = 0;
	}

	GameCmd cmd;
	uint32 netId;
	uint32 ticks;
};
typedef struct _SynchBlock
{
	_SynchBlock()
	{
		userId = 0xFFFFFFFFFFFFFFFF;
		netId = 0x1E;
		teamId = 0x64;
		unk2 = 0;
		flag = 0; //1 for bot?
		memset(data1, 0, 64);
		memset(data2, 0, 64);
		unk3 = 0x19;
	}

	uint64 userId;
	uint16 netId;
	uint32 skill1;
	uint32 skill2;
	uint8 flag;
	uint32 teamId;
	uint8 data1[64];
	uint8 data2[64];
	uint32 unk2;
	uint32 unk3;
} SynchBlock;

struct ClientReady
{
	uint32 cmd;
	uint32 playerId;
	uint32 teamId;
};

typedef struct _SynchVersionAns
{
	_SynchVersionAns()
	{
		header.cmd = PKT_S2C_SynchVersion;
		ok = ok2 = 1;
		memcpy(version, "Version 1.0.0.141 [PUBLIC]", 27);
		memcpy(gameMode, "CLASSIC", 8);
		memset(zero, 0, 2232);
		end1 = 0xE2E0;
		end2 = 0xA0;
	}

	PacketHeader header;
	uint8 ok;
	uint32 mapId;
	SynchBlock players[12];
	uint8 version[27];      //Ending zero so size 26+0x00
	uint8 ok2;              //1
	uint8 unknown[228];     //Really strange shit
	uint8 gameMode[8];
	uint8 zero[2232];
	uint16 end1;            //0xE2E0
	uint8 end2;             //0xA0 || 0x08
} SynchVersionAns;

typedef struct _PingLoadInfo
{
	PacketHeader header;
	uint32 unk1;
	uint64 userId;
	float loaded;
	float ping;
	float f3;
	uint8 unk4;
}PingLoadInfo;

uint8 *createDynamicPacket(uint8 *str, uint32 size);

typedef struct _LoadScreenInfo
{
	_LoadScreenInfo()
	{
		//Zero this complete buffer
		memset(this, 0, sizeof(_LoadScreenInfo));

		cmd = PKT_S2C_LoadScreenInfo;
		blueMax = redMax = 6;		
	}

	uint32 cmd;
	uint32 blueMax;
	uint32 redMax;
	uint32 unk3;
	uint64 bluePlayerIds[6]; //Team 1, 6 players max
	uint8 blueData[144];
	uint64 redPlayersIds[6]; //Team 2, 6 players max
	uint8 redData[144];
	uint32 bluePlayerNo;
	uint32 redPlayerNo;
} LoadScreenInfo;

typedef struct _LoadScreenPlayer
{
	static _LoadScreenPlayer* create(PacketCmd cmd, int8 *str, uint32 size)
	{
		//Builds packet buffer
		uint32 totalSize = sizeof(_LoadScreenPlayer)+size+1;
		uint8* buf = new uint8[totalSize];
		memset(buf, 0, totalSize);

		//Set defaults
		_LoadScreenPlayer *packet = (_LoadScreenPlayer *)buf;
		packet->cmd = cmd;
		packet->length = size;
		packet->unk2 = 0xA;
		packet->skinId = 0;
		memcpy(packet->getDescription(), str, packet->length);
		return packet;
	}

	static void destroy(_LoadScreenPlayer *packet)
	{
		delete [](uint8*)packet;
	}

	uint32 cmd;
	uint32 unk2;
	uint64 userId;
	uint32 skinId;
	uint32 length;
	uint8 description;
	uint8 *getDescription()
	{
		return &description;
	}

	uint32 getPacketLength()
	{
		return sizeof(_LoadScreenPlayer)+length;
	}
} LoadScreenPlayer;


typedef struct _KeyCheck
{
	_KeyCheck()
	{
		cmd = PKT_KeyCheck;
		playerNo = 0;
		checkId = 0;

	}

	uint8 cmd;
	uint8 partialKey[3];   //Bytes 1 to 3 from the blowfish key for that client
	uint32 playerNo;
	uint64 userId;         //uint8 testVar[8];   //User id + padding
	uint64 checkId;        //uint8 checkVar[8];  //Encrypted testVar
} KeyCheck;

typedef struct _AttentionPing
{
	_AttentionPing()
	{

	}
	_AttentionPing(_AttentionPing *ping)
	{
		cmd = ping->cmd;
		unk1 = ping->unk1;
		x = ping->x;
		y = ping->y;
		z = ping->z;
		unk2 = ping->unk2;
		type = ping->type;
	}

	uint8 cmd;
	uint32 unk1;
	float x;
	float y;
	float z;
	uint32 unk2;
	uint8 type;
} AttentionPing;

typedef struct _AttentionPingAns
{
	_AttentionPingAns(AttentionPing *ping)
	{
		attentionPing = AttentionPing(ping);
		attentionPing.cmd = PKT_S2C_AttentionPing;
	}

	AttentionPing attentionPing;
	uint32 response;
} AttentionPingAns;

typedef struct _ViewReq
{
	uint8 cmd;
	uint32 unk1;
	float x;
	float zoom;
	float y;
	float y2;		//Unk
	uint32 width;	//Unk
	uint32 height;	//Unk
	uint32 unk2;	//Unk
	uint8 requestNo;
} ViewReq;

struct MovementVector
{
	uint16 x;
	uint16 y;
};

struct MovementReq
{
	PacketHeader header;
	uint8 count;
	float x;
	float y;
	float z;
	uint32 zero;
	uint16 unk;
	uint16 vectorNo;
	uint32 netId;
	MovementVector vectors;

	MovementVector *getVector(uint32 index)
	{
		if(index >= vectorNo)
			return NULL;

		return &(&vectors)[index]; //A very fancy way of getting the struct from the dynamic buffer
	}

	uint32 size()
	{
		return sizeof(MovementReq)+((vectorNo-1)*sizeof(MovementVector));
	}
};

struct MovementAns
{
	MovementAns()
	{
		header.cmd = PKT_S2C_MoveAns;
	}

	GameHeader header;
	uint16 ok;
	uint16 unk;
	uint16 vectorNo;
	uint32 netId;
	MovementVector vectors;

	MovementVector *getVector(uint32 index)
	{
		if(index >= vectorNo)
			return NULL;
		return &(&vectors)[index]; //A very fancy way of getting the struct from the dynamic buffer
	}

	static MovementAns *create(uint32 vectorNo)
	{
		MovementAns *packet = (MovementAns*)new uint8[size(vectorNo)];
		memset(packet, 0, size(vectorNo));
		packet->header.cmd = PKT_S2C_MoveAns;
		packet->vectorNo = vectorNo;
		return packet;
	}

	static void destroy(MovementAns *packet)
	{
		delete [](uint8*)packet;
	}

	static uint32 size(uint32 vectorNo)
	{
		return sizeof(MovementAns)+((vectorNo-1)*sizeof(MovementVector));
	}

	uint32 size()
	{
		return size(vectorNo);
	}
};

typedef struct _MoveReq
{
	uint8 cmd;
	uint8 sub_cmd1;
	uint16 sub_cmd1Pad; //padding
	uint16 sub_cmd1Def; // defintion of cmd
	float x1; //position where is clicked on
	float y1; //-||-
	float z1; //-||-
	uint32 pad2; //padding
	uint32 unk1; //Unk
	uint8 sub_cmd2;
	uint16 sub_cmd2Pad; //padding
	uint16 sub_cmd2Def; // definition of cmd
	short x2; // I don't get that shit
	short y2; // -||-
	short z2; // -||-
} MoveReq;

typedef struct _ViewAns
{
	_ViewAns()
	{
		cmd = PKT_S2C_ViewAns;
		unk1 = 0;
	}

	uint8 cmd;
	uint32 unk1;
	uint8 requestNo;
} ViewAns;

typedef struct _FogUpdate
{
	_FogUpdate()
	{
		cmd = PKT_S2C_FogUpdate;
		x1 = 0;
		y1 = 0;
		x2 = 0.5;
		y2 = 0.5;
	}
	uint8 cmd;
	float x1;
	float y1;
	float x2;
	float y2;
} FogUpdate;

typedef struct _QueryStatus
{
	_QueryStatus()
	{
		header.cmd = PKT_S2C_QueryStatusAns;
		ok = 1;
	}
	PacketHeader header;
	uint8 ok;
} QueryStatus;

typedef struct _SynchVersion
{
	PacketHeader header;
	uint32 unk1;
	uint32 unk2;
	uint8 version[50]; //Dunno how big and when usefull data begins
} SynchVersion;

typedef struct _WorldSendGameNumber
{
	_WorldSendGameNumber()
	{
		header.cmd = PKT_World_SendGameNumber;
		memset(data, 0, sizeof(data));
		gameId = 0;
	}

	PacketHeader header;
	uint64 gameId;
	uint8 data[0x80];
} WorldSendGameNumber;

typedef struct _ChatBoxMessage
{
	uint32 unk;
	ChatMessageType cmd;
	uint32 msgLength;	
} ChatBoxMessage;

typedef struct _StatePacket
{
	_StatePacket(PacketCmd state)
	{
		header.cmd = state;
	}
	PacketHeader header;
} StatePacket;

struct FogUpdate2
{
	FogUpdate2()
	{
		header.cmd = (PacketCmd)0xC5;
		header.netId = 0x40000019;
	}
	PacketHeader header;
	float x;
	float y;
	uint32 radius;
	uint8 unk1;
};

typedef struct _HeroSpawn
{
	_HeroSpawn()
	{
		header.cmd = PKT_S2C_HeroSpawn;
		unk1 = 0;
		memset(&name, 0, 128+40); //Set name + type to zero

		x = 130880;
		y = 502;
	}

	PacketHeader header;
	uint32 netId; //Also found something about locking flag//Perhaps first 4 bits is type and rest is netId?? or something?? //Linked for mastery's (first uitn32, and also animation (looks like it) and possible more) often looks like XX 00 00 40 where XX is around 10-30
	uint32 gameId; //1-number of players
	uint32 x;       //Some coordinates, no idea how they work yet
	uint32 y;
	uint16 unk1;
	uint8 name[128];
	uint8 type[40];
} HeroSpawn;

typedef struct _SkillUpPacket
{
	PacketHeader header;
	uint8 skill;
} SkillUpPacket;

typedef struct _SkillUpResponse
{
	_SkillUpResponse()
	{
		header.cmd = PKT_S2C_SkillUp;
		level = 0x0000;
	}
	PacketHeader header;
	uint8 skill;
	uint16 level; //?


} SkillUpResponse;

#if defined( __GNUC__ )
#pragma pack()
#else
#pragma pack(pop)
#endif

#endif