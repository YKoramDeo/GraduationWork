#pragma once

#define NIL						-999	// 내가 정한 NULL이 아닌 default 값
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

struct Monster
{
	Vector3 pos;
	Vector3 patrolPos;
};

Vector3 tmpMonsterPath = { 200.0f,0.0f,0.0f };

Vector3 monsterPath[NUM_OF_MONSTER_PATH] = {	(160.0f,	0.0f,	30.0f),
												(90.0f,		0.0f,	60.0f),
												(160.0f,	0.0f,	-60.0f),
												(200.0f,	0.0f,	0.0f) };