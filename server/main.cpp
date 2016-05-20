#include "stdafx.h"
#include "protocol.h"
#include "defaultInit.h"
#include "iocpNetwork.h"
#include "processNetwork.h"

bool gShutdown = false;
HANDLE ghIOCP;
Client gClientsList[MAX_USER];

Monster gMonster;
Vector3 tmpMonsterPath = { 200.0f,0.0f,0.0f };
Vector3 monsterPath[NUM_OF_MONSTER_PATH] = { (160.0f,	0.0f,	30.0f),
											 (90.0f,	0.0f,	60.0f),
											 (160.0f,	0.0f,	-60.0f),
											 (200.0f,	0.0f,	0.0f) };
std::mutex gLock;

int main(int argc, char *argv[])
{
	std::vector<std::thread*> workerThreads;
	std::thread acceptThread;

	InitializeServer();
	InitializeMonster();

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
	return 0;
}