#pragma once
#ifndef DRAW_QUEUE_H
#define DRAW_QUEUE_H
#include <DirectXMath.h>

void DrawQueue_Clear();

void DrawQueue_Push(int textureID,
	float x, float y, float width, float height,
	int srcX, int srcY, int srcWidth, int srcHeight,
	float sortY,
	float angle = 0.0f,
	const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f });

void DrawQueue_Flush(); // sorts everything pushed since the last Clear() by sortY, draws it, then clears

#endif //DRAW_QUEUE_H