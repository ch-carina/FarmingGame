/*----------------------------------------------------
 HEADER COMMENT

 draw_queue.cpp
 Idea is so that the game has depth and player goes 
 behind and in front of objects 
								   Author: Carina Chao
								   Date: 2026/08/30
 ----------------------------------------------------*/
#include "draw_queue.h"
#include "sprite.h"
#include <algorithm>
using namespace DirectX;

struct DrawRequest
{
	int textureID;
	float x, y, width, height;
	int srcX, srcY, srcWidth, srcHeight;
	float sortY;
	float angle;
	XMFLOAT4 color;
};

static constexpr int MAX_DRAW_REQUESTS = 512;
static DrawRequest g_Requests[MAX_DRAW_REQUESTS];
static int g_RequestCount = 0;

void DrawQueue_Clear()
{
	g_RequestCount = 0;
}

void DrawQueue_Push(int textureID,
	float x, float y, float width, float height,
	int srcX, int srcY, int srcWidth, int srcHeight,
	float sortY,
	float angle,
	const XMFLOAT4& color)
{
	if (g_RequestCount >= MAX_DRAW_REQUESTS) return;

	DrawRequest& r = g_Requests[g_RequestCount++];
	r.textureID = textureID;
	r.x = x; r.y = y; r.width = width; r.height = height;
	r.srcX = srcX; r.srcY = srcY; r.srcWidth = srcWidth; r.srcHeight = srcHeight;
	r.sortY = sortY;
	r.angle = angle;
	r.color = color;
}

void DrawQueue_Flush()
{
	std::sort(g_Requests, g_Requests + g_RequestCount,
		[](const DrawRequest& a, const DrawRequest& b) { return a.sortY < b.sortY; });

	for (int i = 0; i < g_RequestCount; i++)
	{
		const DrawRequest& r = g_Requests[i];
		Sprite_Draw(r.textureID, r.x, r.y, r.width, r.height,
			r.srcX, r.srcY, r.srcWidth, r.srcHeight,
			r.angle, { 1.0f, 1.0f }, r.color);
	}

	g_RequestCount = 0;
}
