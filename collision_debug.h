/*----------------------------------------------------
 HEADER COMMENT

 Collision_debug.h
								   Author: Carina Chao
								   Date: 2026/07/06
 ----------------------------------------------------*/
#pragma once
#ifndef GAME_COLLISION_DEBUG_H
#define GAME_COLLISION_DEBUG_H		
#include <DirectXMath.h>
#include "collision.h"

void CollisionDebug_Initialize();
void CollisionDebug_Finalize();

void CollisionDebug_Draw(const CollisionCircle& circle);

#endif //GAME_COLLISION_DEBUG_H
