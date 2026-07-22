/*----------------------------------------------------
 HEADER COMMENT

 collision checks
 collision.cpp
								   Author: Carina Chao
								   Date: 2026/07/01
 ----------------------------------------------------*/

#include "collision.h"
using namespace DirectX;

bool CollisionCircle_IsOverlap(const CollisionCircle& circle1, const CollisionCircle& circle2)
{
	XMVECTOR center1 = XMLoadFloat2(&circle1.center);
	XMVECTOR center2 = XMLoadFloat2(&circle2.center);
	float radius1 = circle1.radius;
	float radius2 = circle2.radius;

	// Calculate the distance between the centers. This was taught in math class 
	XMVECTOR delta = center1 - center2;
	float distanceSquared = XMVectorGetX(XMVector2LengthSq(delta));

	// Calculate the sum of the radii
	float radiiSum = radius1 + radius2;

	// Check if the distance is less than to the sum of the radii
	return distanceSquared < radiiSum * radiiSum;
}
