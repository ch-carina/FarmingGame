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
#include "crop.h"

enum EnemyType //create different types of enemies 
{
	EnemyType_Rabbit, //0
	EnemyType_Crow,//1
	EnemyType_Deer, //2 
	EnemyType_Bear,
	EnemyType_Mole, 
	EnemyType_MAX
};

enum EnemyState
{
	EnemyState_None,
	EnemyState_Spawn,
	EnemyState_Alive,
	EnemyState_Eating,
	EnemyState_Return,
	EnemyState_Burrowing
};

enum EnemyAnimState
{
    EnemyAnim_Run,
    EnemyAnim_Eating,
    EnemyAnim_Escape,
	EnemyAnim_Burrow, 
	EnemyAnim_Appear, 
    EnemyAnim_MAX
};

enum EnemySpawnSide
{
	EnemySpawn_TopLeft, 
	EnemySpawn_TopRight, 
	EnemySpawn_Bottom, 
	EnemySpawn_MAX
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

	float dirX, dirY; //direction of enemy movement
	int targetPlotIndex; //index of the crop plot the enemy is targeting
	float targetX, targetY; //coordinates of the crop plot the enemy is targeting
	float eatingTimer; //timer for how long the enemy has been eating the crop
	float burrowTimer;
};

constexpr float ENEMY_DRAW_SIZE = 64.0f; 

void EnemyInitialize();

void EnemyFinalize();

void EnemyUpdate(float delta_time);

void EnemyDraw();

void EnemyCreate(EnemyType type, EnemySpawnSide side, float targetX, float targetY, int plotIndex);

int EnemyGetCount();

void Enemy_CheckCropSpawns(float delta_time);

void Enemy_Destroy(int index);

void Enemy_Cleanup();

CollisionCircle Enemy_GetCollisionCircle(int index);

EnemyType Enemy_GetTypeForCrop(CropType cropType);


#endif ENEMY