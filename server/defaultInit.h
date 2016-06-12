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