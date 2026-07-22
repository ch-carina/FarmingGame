/*----------------------------------------------------
 HEADER COMMENT

 EnemyAnimation.h
 controller different enemy sprite animation sates 
                                   Author: Carina Chao
                                   Date: 2026/07/022
 ----------------------------------------------------*/
#pragma once
#ifndef ENEMY_ANIMATION_H
#define ENEMY_ANIMATION_H
#include "enemy.h"
#include "collision.h"
#include <DirectXMath.h> 

struct AnimInfo 
{
	int textureID;

	int frameWidth;
	int frameHeight;

	int startFrame;
	int frameCount;
	float frameRate;

	int columns;
};

struct EnemySpriteInfo
{

	CollisionCircle collision; //collision circle for the enemy

	AnimInfo animation[EnemyAnim_MAX];
};

void EnemyAnimation_Initialize();

void EnemyAnimation_Update(Enemy& enemy, float deltaTime);

AnimInfo EnemyAnimation_GetInfo(EnemyType type, EnemyAnimState state);

const EnemySpriteInfo& EnemyAnimation_GetSpriteInfo(EnemyType type);


#endif //ENEMY_ANIMATION_H