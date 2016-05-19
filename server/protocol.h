#pragma once
#include "stdafx.h"

#define MY_SERVER_PORT		4000

#define NUM_THREADS			6
#define MAX_USER			10

#define MAX_BUFF_SIZE		4000

#define OP_RECV				1
#define OP_SEND				2

enum PacketType
{
	SetID,
	Connect,
	Disconnect,
	PlayerMove,
	PlayerLight,
	PlayerShout,
	MonsterSetInfo,
	MonsterMove,
	MonsterSetPatrolPos
};

#pragma pack(push,1)
namespace Packet
{
	struct SetID
	{
		BYTE size;
		BYTE type;
		int id;
	};

	struct Connect
	{
		BYTE size;
		BYTE type;
		int id;
		float posX;
		float posY;
		float posZ;
	};

	struct Disconnect
	{
		BYTE size;
		BYTE type;
		int id;
	};

	namespace Player
	{
		struct Move
		{
			BYTE size;
			BYTE type;
			int id;
			float posX;
			float posY;
			float posZ;

			float dirX;
			float dirY;
			float dirZ;

			float horizental;
			float vertical;
			bool sneak;
		};

		struct Light
		{
			BYTE size;
			BYTE type;
			int id;
			bool on;
			float rotX;
			float rotY;
			float rotZ;
			float rotW;
		};

		struct Shout
		{
			BYTE size;
			BYTE type;
			int id;
			bool shouting;
			float posX;
			float posY;
			float posZ;
		};
	}

	namespace Monster
	{
		struct SetInfo
		{
			BYTE size;
			BYTE type;
			float posX;
			float posY;
			float posZ;
			float patrolPosX;
			float patrolPosY;
			float patrolPosZ;
		};

		struct Move
		{
			BYTE size;
			BYTE type;
			float posX;
			float posY;
			float posZ;
		};

		struct SetPatrolPos
		{
			BYTE size;
			BYTE type;
			float posX;
			float posY;
			float posZ;
		};
	}
}
#pragma pack(pop)

