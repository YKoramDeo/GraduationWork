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

#define NIL						-999	// ���� ���� NULL�� �ƴ� default ��
#define NUM_OF_MONSTER_PATH		4

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
	Vector3	pos;
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
	int				id;
	bool			isConnect;
	SOCKET			socket;
	Player			player;
	OverlapEx		recvOverlap;
	unsigned char	packetBuf[MAX_BUFF_SIZE];	// Recv�Ǵ� ��Ŷ�� �����Ǵ� Buffer / Send ������ ������� �����Ƿ� Ȯ�� ����ü�� ���Ե��� ����.
	int				previousDataSize;			// ������ ���� ���� �����ϴ� ���� / Send ������ ������� �����Ƿ� Ȯ�� ����ü�� ���Ե��� ����.
};

struct Monster
{
	Vector3 pos;
	Vector3 patrolPos;
};

extern Monster gMonster;
extern std::mutex gLock;
extern Vector3 monsterPath[NUM_OF_MONSTER_PATH];