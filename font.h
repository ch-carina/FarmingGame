/*----------------------------------------------------
 HEADER COMMENT

 font.h
 controlling font base for game so that dont have to use
 sprite sheet every time 
                                   Author: Carina Chao
                                   Date: 2026/08/12
 ----------------------------------------------------*/
#pragma once
#ifndef FONT_H
#define FONT_H 
#include <DirectXMath.h>

void Font_Initialize(); 
void Font_Finalize(); 

//Draws text top-left and anchored and xy using ascii atlas 
void Font_Print(const char* text, float x, float y, float scale =1.0f,
    const DirectX::XMFLOAT4& color = {1.0f,1.0f,1.0f,1.0f});

//Size the text would take up if printed at this scale 
DirectX::XMFLOAT2 Font_MeasureText(const char* text, float scale = 1.0f);


#endif // FONT_H