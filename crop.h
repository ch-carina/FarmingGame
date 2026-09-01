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

enum CropRank
{
    CropRank_Normal, //not a watered crop 
    CropRank_Watered, //watered crop 
    CropRank_MAX
};

struct Crop
{
    CropType type; 
    float x, y; 

    float growthTime; 
    CropGrowthState growthStage;
    
    bool wateredCrop; 
    CropRank rank;
    bool growthPaused; // used only for tutorial right now. Proabably will only be used there
    float waterVfxTimer;
    
    int cropCost; 
    int cropProfit; 

    float animationTimer;
    int currentFrame;

    float rankAnimTimer;
    int   rankFrame;

    bool isActive;
};

constexpr float CROP_DISPLAY_SIZE = 64.0f; //Size of crop sprite when drawn on screen

void CropInitialize();

void CropFinalize();

int CropCreate(CropType type, float x, float y);

void CropUpdate(float deltaTime);

void CropDraw();

void Crop_Destroy(int index); 

void Crop_Water(int index);

int Crop_GetRank(int index);

void Crop_SetGrowthPaused(int index, bool paused);

const Crop& CropGet(int index);


#endif //CROP_H
