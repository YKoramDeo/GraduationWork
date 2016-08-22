#pragma once

// ------ ------ ------ ------ ------ ------ ------ ------ ------ ------ 
//
//						defaultInit_H
//
//		프로그램을 실행하기 위해 rmsdyddl(본인)이 
//		별도로 필요한 내용을 선언한 곳
//
// ------ ------ ------ ------ ------ ------ ------ ------ ------ ------

#include "stdafx.h"
#include "protocol.h"

#define NIL						-9999	// 내가 정한 NULL이 아닌 default 값
#define NUM_OF_MONSTER_PATH		4

#define MAX_INT 0x80000000
#define MIN_INT 0x7FFFFFFF

#define DEFAULT_POS_X 247.92f;
#define DEFAULT_POS_Y -4.29f;
#define DEFAULT_POS_Z -1.23f;

struct Vector3
{
	float x;
	float y;
	float z;
};

struct Quaternian
{
	float x;
	float y;
	float z;
	float w;
};

struct Player
{
	int roomNo;
	Vector3	pos;
};

struct RoomInfo
{
	int no;
	int chiefID;
	int partner_1_ID;
	int partner_2_ID;
	int partner_3_ID;
	bool partner_1_ready;
	bool partner_2_ready;
	bool partner_3_ready;
};

struct OverlapEx
{
	WSAOVERLAPPED	originalOverlap;			// 기존의 Overlapped 구조체
	WSABUF			wsaBuf;						// 기존의 WSABUF
	int				operation;					// 현재 Send or Recv 연산을 진행하는 것인지 여부를 저장하는 변수
	unsigned char	buffer[MAX_BUFF_SIZE];		// IOCP Send / Recv Buffer
	int				packetSize;					// 받은 packet의 양을 저장하는 변수.
};

struct Client
{
	bool			isConnect;
	SOCKET			socket;
	Player			player;
	OverlapEx		recvOverlap;
	unsigned char	packetBuf[MAX_BUFF_SIZE];	// Recv되는 패킷이 조립되는 Buffer / Send 에서는 사용하지 않으므로 확장 구조체에 포함되지 않음.
	int				previousDataSize;			// 이전의 받은 양을 저장하는 변수 / Send 에서는 사용하지 않으므로 확장 구조체에 포함되지 않음.
};