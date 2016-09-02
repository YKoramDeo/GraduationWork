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

/**********************변 경 사 항**********************/
int gItemArr[NUM_OF_ITEM];
/**********************변 경 사 항**********************/

int main(int argc, char *argv[])
{
	std::vector<std::thread*> workerThreads;
	std::thread acceptThread;

	InitializeServer();
	/**********************변 경 사 항**********************/
	InitializeItem();
	/**********************변 경 사 항**********************/

	acceptThread = std::thread(AcceptThreadFunc);
	for (int i = 0; i < NUM_THREADS; ++i)
		workerThreads.push_back(new std::thread(WorkerThreadFunc));

	while (!gShutdown) Sleep(1000);
	//  다음과 같이 Main Thread를 1초마다 잠들게 하여
	// Processor가 Main Thread를 점유하는 일을 최대한 저해하도록 한다.

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