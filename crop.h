/*----------------------------------------------------
 HEADER COMMENT

 crop.h
 header for base of crop 
                                   Author: Carina Chao
                                   Date: 2026/07/22
 ----------------------------------------------------*/

#pragma once
#ifndef CROP_H
#define CROP_H		
#include <DirectXMath.h>

enum CropType
{
    CropType_Carrot, 
    CropType_Wheat, 
    CropType_Lettuce,
    CropType_Corn,
    CropType_Blueberry,
    CropType_MAX
};

enum CropGrowthState
{
    CropGrowth_Planted,
    CropGrowth_Sprouted,
    CropGrowth_Teen, 
    CropGrowth_Ready, 
    CropGrowth_MAX
};

struct Crop
{
    CropType type; 
    float x, y; 

    float growthTime; 
    CropGrowthState growthStage;
    
    bool greatCrop; 
    
    int cropCost; 
    int cropProfit; 

    float animationTimer;
    int currentFrame;

    bool isActive;
};


void CropInitialize();

void CropFinalize();

void CropCreate(CropType type, float x, float y);

void CropUpdate(float deltaTime);
void CropDraw();

const Crop& CropGet(int index);


#endif //CROP_H
