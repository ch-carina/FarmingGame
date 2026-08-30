/*----------------------------------------------------
 HEADER COMMENT

 crop_plot.h
                                   Author: Carina Chao
                                   Date: 2026/07/22
 ----------------------------------------------------*/

#pragma once
#ifndef CROP_PLOT_H
#define CROP_PLOT_H		
#include <DirectXMath.h>
#include "crop.h"
#include "collision.h"

enum PlotType
{

    PlotType_DirtTL,
    PlotType_DirtTC,
    PlotType_DirtTR,

    PlotType_DirtML,
    PlotType_DirtC,
    PlotType_DirtMR,

    PlotType_DirtBL,
    PlotType_DirtBC,
    PlotType_DirtBR,

    PlotType_MAX


};

struct PlotRegion
{
    int gridX, gridY;   // top-left corner, in tile units (not pixels)
    int width, height;  // size, in tiles
};

struct CropPlot
{
    float x, y; 

    bool occupied; 
    float spawnCooldownTimer;
    bool hasActiveEnemy; 

    int cropIndex;

    CropType cropType;
    PlotType plotType; 
    
    CollisionBox cropCollision;

    bool hasScarecrow;
    float scarecrowTimer;
};

constexpr float PLOT_SIZE = 96.0f;

int CropPlot_GetIndexAt(float x, float y); // -1 if no plot sits at this exact pixel position

CropPlot* CropPlot_Get(int index);

int CropPlot_GetCount();

void CropPlot_Initialize();
void CropPlot_Update(float delta_time);
void CropPlot_Draw();

void CropPlot_LoadRegions(const PlotRegion regions[], int regionCount);

void CropPlot_Plant(int index, CropType cropType);
void CropPlot_Harvest(int index);

bool CircleVsBox(const CollisionCircle& playerCollision, const CollisionBox& cropPlotBox);
int CropPlot_GetPlayerPlot();
CollisionBox CropPlot_GetCollision(int index);

void CropPlot_PlaceScarecrow(int index);
bool CropPlot_IsAdjacentToScarecrow(int index);

#endif //CROP_PLOT_H
