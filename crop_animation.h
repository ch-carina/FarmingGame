
/*----------------------------------------------------
 HEADER COMMENT

 crop_animation.h

 Controls crop sprite animations

                                   Author: Carina Chao
                                   Date: 2026/07/22
 ----------------------------------------------------*/
#pragma once
#ifndef CROP_ANIMATION_H
#define CROP_ANIMATION_H

#include "crop.h"

struct CropAnimInfo
{
    int textureID;

    int frameWidth;
    int frameHeight;

    int startFrame;
    int frameCount;

    float frameRate;

    int columns;
};

void CropAnimation_Initialize();
void CropAnimation_Finalize();

CropAnimInfo CropAnimation_GetInfo(
    CropType type,
    CropGrowthState stage);

#endif
