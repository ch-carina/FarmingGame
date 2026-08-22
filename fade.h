#pragma once
/*----------------------------------------------------
 HEADER COMMENT

 fade.h
 fade header - loading (changing in between)
								   Author: Carina Chao
								   Date: 2026/07/10
 ----------------------------------------------------*/
#ifndef FADE_H
#define FADE_H

#include <DirectXMath.h> 

void Fade_Initialize();
void Fade_Finalize();
void Fade_Update(float delta_time);
void Fade_Draw();

enum FadeType
{
	kIn,kOut
};

void Fade_Start(FadeType type, float fadeTime, const DirectX::XMFLOAT4& color);
bool Fade_IsFinished();

#endif //FADE_H