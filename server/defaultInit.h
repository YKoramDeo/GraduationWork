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

#define NIL						-999	// 내가 정한 NULL이 아닌 default 값
#define NUM_OF_MONSTER_PATH		4
#define NUM_OF_ITEM				6

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

struct Monster
{
	Vector3 pos;
	Vector3 patrolPos;
};

extern Monster gMonster;
extern std::mutex gLock;
extern Vector3 monsterPath[NUM_OF_MONSTER_PATH];
extern int gItemArr[NUM_OF_ITEM];