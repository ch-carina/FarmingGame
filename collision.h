/*----------------------------------------------------
 HEADER COMMENT

 Collision.h 
                                   Author: Carina Chao
                                   Date: 2026/07/01
 ----------------------------------------------------*/

#pragma once
#ifndef GAME_COLLISION_H
#define GAME_COLLISION_H		
#include <DirectXMath.h>


struct CollisionCircle
{
	DirectX::XMFLOAT2 center;
	float radius;
};

struct CollisionBox
{
	float width; 
	float height; 
};

bool CollisionCircle_IsOverlap(const CollisionCircle& circle1, const CollisionCircle& circle2);

#endif //GAME_COLLISION_H
