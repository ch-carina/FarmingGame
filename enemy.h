/*----------------------------------------------------
 HEADER COMMENT

 Enemy.h
                                   Author: Carina Chao
                                   Date: 2026/07/01
 ----------------------------------------------------*/
#ifndef ENEMY
#define ENEMY
#include <DirectXMath.h>
#include "collision.h"

enum EnemyType //create different types of enemies 
{
	EnemyType_Rabbit, //0
	EnemyType_Crow,//1
	EnemyType_Deer, //2 
	EnemyType_Bear,
	EnemyType_MAX
};

enum EnemyState
{
	EnemyState_None,
	EnemyState_Spawn,
	EnemyState_Alive,
	EnemyState_Eating,
	EnemyState_Return
};

enum EnemyAnimState
{
    EnemyAnim_Run,
    EnemyAnim_Eating,
    EnemyAnim_Escape,
    EnemyAnim_MAX
};

struct Enemy
{
	EnemyType type; //what type of enemy is it
	EnemyState state; //what state is the enemy in

	EnemyAnimState animState; //Animation state of enemy type 
	float animTimer;
	int currentFrame;

	float x, y; //starting position for enemy

	float speed; //Animal Move Speed 

	int totalHit; //Each animal can needs a certain amount of hits before it runs off in a different direction. 

	bool isDestroyed;
};

void EnemyInitialize();

void EnemyFinalize();

void EnemyUpdate(float delta_time);

void EnemyDraw();

void EnemyCreate(EnemyType type, float x, float y);

int EnemyGetCount();

void Enemy_Destroy(int index);

void Enemy_Cleanup();

CollisionCircle Enemy_GetCollisionCircle(int index);


#endif ENEMY