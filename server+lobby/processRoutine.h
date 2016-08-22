#pragma once

// ------ ------ ------ ------ ------ ------ ------ ------ ------ ------ 
//
//						processRoutine_H
//
//		IOCP에서 실제로 packet을 처리하는 함수를 별도로 선언하는 곳
//
// ------ ------ ------ ------ ------ ------ ------ ------ ------ ------

#include "stdafx.h"
#include "protocol.h"
#include "iocpNetwork.h"
#include "lock-free-SET_RoomInfo.h"

bool BeCompeletedSendPacket(BYTE, BYTE);
void ProcessPacket(int, unsigned char*);

namespace OnReceivePacket
{
	void Notify(int, unsigned char*);
	void JoinRoom(int, unsigned char*);
	/**********************변 경 사 항**********************/
	void Connect(int, unsigned char*);
	void PlayerMove(int, unsigned char*);
	void PlayerLight(int, unsigned char*);
	void PlayerShout(int, unsigned char*);
	void PlayerGetItem(int, unsigned char*);
	void MonsterMove(int, unsigned char*);
}

void InitializeMonster(void);
void InitializeItem(void);
void SendMonsterSetInfoPacket(const int);
bool HasMonsterArrivedAtDestination(void);
void SetMonsterNewPatrolPath(void);
void SendMonsterSetPatrolPosPacket(void);
/**********************변 경 사 항**********************/