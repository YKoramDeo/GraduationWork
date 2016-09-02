#include "stdafx.h"
#include "protocol.h"
#include "defaultInit.h"
#include "iocpNetwork.h"
#include "processRoutine.h"
#include "tbb_hash_map.h"

bool gShutdown = false;
HANDLE ghIOCP;

TBB_HASH_MAP<Client> gClientInfoMAP;
TBB_HASH_MAP<RoomInfo> gRoomInfoMAP;

/**********************�� �� �� ��**********************/
int gItemArr[NUM_OF_ITEM];
/**********************�� �� �� ��**********************/

int main(int argc, char *argv[])
{
	std::vector<std::thread*> workerThreads;
	std::thread acceptThread;

	InitializeServer();
	/**********************�� �� �� ��**********************/
	InitializeItem();
	/**********************�� �� �� ��**********************/

	acceptThread = std::thread(AcceptThreadFunc);
	for (int i = 0; i < NUM_THREADS; ++i)
		workerThreads.push_back(new std::thread(WorkerThreadFunc));

	while (!gShutdown) Sleep(1000);
	//  ������ ���� Main Thread�� 1�ʸ��� ���� �Ͽ�
	// Processor�� Main Thread�� �����ϴ� ���� �ִ��� �����ϵ��� �Ѵ�.

	acceptThread.join();
	for (auto t : workerThreads)
	{
		t->join();
		delete t;
	}

	StopServer();
	gClientInfoMAP.Clear();
	gRoomInfoMAP.Clear();

	return 0;
}