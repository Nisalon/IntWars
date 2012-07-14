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
#ifndef _CLIENT_H
#define _CLIENT_H

#include "common.h"

struct ClientInfo
{
	ClientInfo()
	{
		keyChecked = false;
		name = type = NULL;
		ticks = 0;
		skinNo = 0;
	}

	~ClientInfo()
	{
		if(name != NULL)
			delete[] name;
		if(type != NULL)
			delete[] type;
	}

	void setName(char *name)
	{
		if(this->name != NULL)
			delete[] this->name;

		nameLen = strlen(name);
		this->name = new int8[nameLen+1];
		memcpy(this->name, name, nameLen+1);
	}

	void setType(char *type)
	{
		if(this->type != NULL)
			delete[] this->type;

		typeLen = strlen(type);
		this->type = new int8[typeLen+1];
		memcpy(this->type, type, typeLen+1);
	}

	uint32 getTicks()
	{
		ticks++;
		return ticks;
	}
	bool keyChecked;
	uint64 userId;
	uint32 ticks;
	uint32 netId;
	uint32 nameLen;
	uint32 typeLen;
	uint32 skinNo;
	int8 *name;
	int8 *type;

};
#define peerInfo(p) ((ClientInfo*)p->data)

#endif