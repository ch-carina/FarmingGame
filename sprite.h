#pragma once
#ifndef SPRITE_H
#define SPRITE_H

#include <DirectXMath.h>

bool Sprite_Initialize();
void Sprite_Finalize();

// 好きなテクスチャを
// 好きな場所に描画
void Sprite_Draw(int texture_id, float position_x, float position_y,
	const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f });

// 好きなテクスチャを
// 好きな場所に
// 好きなサイズで描画
void Sprite_Draw(
	int texture_id,
	float position_x, float position_y,
	float width, float height,
	const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f });

// 好きなテクスチャを
// 好きな場所に
// 好きな場所のテクスチャを
// 好きなサイズに切り取って描画する
void Sprite_Draw(
	int texture_id,
	float position_x, float position_y,
	int texture_x, int texture_y,
	int texture_width, int texture_height,
	const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f });


// 好きなテクスチャを
// 好きな場所に
// 好きなサイズで
// 好きな場所のテクスチャを
// 好きなサイズに切り取って描画
void Sprite_Draw(
	int texture_id,
	float position_x, float position_y,
	float width, float height,
	int texture_x, int texture_y,
	int texture_width, int texture_height,
	const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f });


// 好きなテクスチャを
// 好きな場所に
// 好きなサイズで
// 好きな場所のテクスチャを
// 好きなサイズに切り取って
// 回転
// 拡大率
// 描画
void Sprite_Draw(
	int texture_id,
	float position_x, float position_y,
	float width, float height,
	int texture_x, int texture_y,
	int texture_width, int texture_height,
	float angle,
	const DirectX::XMFLOAT2& scale = { 1.0f, 1.0f },
	const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f });

#endif //SPRITE_H