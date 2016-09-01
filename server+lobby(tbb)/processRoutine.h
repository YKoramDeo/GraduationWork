#pragma once

// ------ ------ ------ ------ ------ ------ ------ ------ ------ ------ 
//
//						processRoutine_H
//
//		IOCP���� ������ packet�� ó���ϴ� �Լ��� ������ �����ϴ� ��
//
// ------ ------ ------ ------ ------ ------ ------ ------ ------ ------

#include "stdafx.h"
#include "protocol.h"
#include "iocpNetwork.h"

bool BeCompeletedSendPacket(BYTE, BYTE);
void ProcessPacket(int, unsigned char*);

namespace OnReceivePacket
{
	void Notify(int, unsigned char*);
	void JoinRoom(int, unsigned char*);
}