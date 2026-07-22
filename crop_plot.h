/*----------------------------------------------------
 HEADER COMMENT

 crop_plot.h
                                   Author: Carina Chao
                                   Date: 2026/07/022
 ----------------------------------------------------*/

#pragma once
#ifndef CROP_PLOT_H
#define CROP_PLOT_H		
#include <DirectXMath.h>
#include "crop.h"

enum PlotType
{
    PlotType_DirtBL,
    PlotType_DirtBC,
    PlotType_DirtBR,
    PlotType_DirtTL,
    PlotType_DirtTR,
    PlotType_DirtTC,
    PlotType_DirtML,
    PlotType_DirtMR,
    PlotType_DirtC,
    PlotType_MAX
};

struct CropPlot
{
    float x, y; 

    bool occupied; 

    CropType cropType;
    PlotType plotType; 
};


void CropPlot_Initialize();
void CropPlot_Update();
void CropPlot_Draw();

void CropPlot_Plant(int index, CropType cropType);
void CropPlot_Harvest(int index);


#endif //CROP_PLOT_H
