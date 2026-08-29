/*---------------------------------s-------------------
 HEADER COMMENT

 water.cpp

 track water plots and placement and blocking 

                                   Author: Carina Chao
                                   Date: 2026/08/29
 ----------------------------------------------------*/
#pragma once
#ifndef WATER_H
#define WATER_H
#include "crop_plot.h" 
#include "collision.h" // for CollisionCircle

bool Water_IsBlocked(const CollisionCircle& circle);

void Water_Initialize();
void Water_Finalize();
void Water_Update(float delta_time);
void Water_Draw();

void Water_LoadRegions(const PlotRegion regions[], int regionCount);

bool Water_IsBlocked(float x, float y); // true if this pixel position falls inside a water tile

#endif //WATER_H