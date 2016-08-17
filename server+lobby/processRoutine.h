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
}