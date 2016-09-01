#pragma once

// ------ ------ ------ ------ ------ ------ ------ ------ ------ ------ 
//
//						iocpNetwork_H
//
//		IOCP를 사용하기 위해서 기본적으로 필요로 하는 
//		구조체와 함수를 선언 및 정의한 곳
//
// ------ ------ ------ ------ ------ ------ ------ ------ ------ ------

#include "protocol.h"
#include "defaultInit.h"
#include "tbb_hash_map.h"

/*
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
	int				id;
	bool			isConnect;
	SOCKET			socket;
	Player			player;
	OverlapEx		recvOverlap;
	unsigned char	packetBuf[MAX_BUFF_SIZE];	// Recv되는 패킷이 조립되는 Buffer / Send 에서는 사용하지 않으므로 확장 구조체에 포함되지 않음.
	int				previousDataSize;			// 이전의 받은 양을 저장하는 변수 / Send 에서는 사용하지 않으므로 확장 구조체에 포함되지 않음.
};
*/
// 해당되는 내용은 lock-free SET과 동기화하가 위해 defaultInit부분으로 변경
// 여기에 주석 처리해서 남겨두는 이유는 다른 프로그램에서 사용될 때 참고할 수 있도록 남겨둠

extern bool gShutdown;
extern HANDLE ghIOCP;

void DisplayErrMsg(char*, int);
void DisplayDebugText(std::string);

void InitializeServer(void);
void StopServer(void);
void WorkerThreadFunc(void);
void AcceptThreadFunc(void);
void SendPacket(const int, const unsigned char*);

extern void ProcessPacket(int, unsigned char*);
extern bool BeCompeletedSendPacket(BYTE, BYTE);

extern void BroadcastingExceptIndex(const int, const unsigned char*);
extern void Broadcasting(const unsigned char*);
extern void BroadcastingExceptIndex_With_UpdateRoomInfo(const int);
extern void UpdateRoomInfo(const unsigned int);