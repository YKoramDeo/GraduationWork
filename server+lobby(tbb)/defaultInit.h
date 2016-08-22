#pragma once

// ------ ------ ------ ------ ------ ------ ------ ------ ------ ------ 
//
//						defaultInit_H
//
//		���α׷��� �����ϱ� ���� rmsdyddl(����)�� 
//		������ �ʿ��� ������ ������ ��
//
// ------ ------ ------ ------ ------ ------ ------ ------ ------ ------

#include "stdafx.h"
#include "protocol.h"

#define NIL						-9999	// ���� ���� NULL�� �ƴ� default ��
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
	WSAOVERLAPPED	originalOverlap;			// ������ Overlapped ����ü
	WSABUF			wsaBuf;						// ������ WSABUF
	int				operation;					// ���� Send or Recv ������ �����ϴ� ������ ���θ� �����ϴ� ����
	unsigned char	buffer[MAX_BUFF_SIZE];		// IOCP Send / Recv Buffer
	int				packetSize;					// ���� packet�� ���� �����ϴ� ����.
};

struct Client
{
	bool			isConnect;
	SOCKET			socket;
	Player			player;
	OverlapEx		recvOverlap;
	unsigned char	packetBuf[MAX_BUFF_SIZE];	// Recv�Ǵ� ��Ŷ�� �����Ǵ� Buffer / Send ������ ������� �����Ƿ� Ȯ�� ����ü�� ���Ե��� ����.
	int				previousDataSize;			// ������ ���� ���� �����ϴ� ���� / Send ������ ������� �����Ƿ� Ȯ�� ����ü�� ���Ե��� ����.
};