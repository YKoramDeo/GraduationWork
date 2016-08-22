#pragma once

// ------ ------ ------ ------ ------ ------ ------ ------ ------ ------ 
//
//						protocol_H
//
//		client 와의 communication을 하기 위해 약속을 정의 해 놓은 곳
//
// ------ ------ ------ ------ ------ ------ ------ ------ ------ ------

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
	Disconnect,
	Notify,
	CreateRoom,
	JoinRoom,
	RenewalRoomInfo
};

enum Notice
{
	RECV_SET_ID = 100,
	MAKE_ROOM,
	QUIT_ROOM,
	UPDATE_ROOM,
	JOIN_FAIL,
	GAME_READY,
	CANCEL_READY,
	GAME_START
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

	struct Disconnect
	{
		BYTE size;
		BYTE type;
		int id;
	};

	struct Notify
	{
		BYTE size;
		BYTE type;
		int id;
		int notice;
	};

	struct CreateRoom
	{
		BYTE size;
		BYTE type;
		int roomNo;
		int chiefNo;
		int partner_1_ID;
		int partner_2_ID;
		int partner_3_ID;
	};

	struct JoinRoom
	{
		BYTE size;
		BYTE type;
		int roomNo;
	};

	struct RenewalRoomInfo
	{
		BYTE size;
		BYTE type;
		int roomNo;
		int chiefNo;
		int partner_1_ID;
		bool partner_1_ready;
		int partner_2_ID;
		bool partner_2_ready;
		int partner_3_ID;
		bool partner_3_ready;
	};
}
#pragma pack(pop)

