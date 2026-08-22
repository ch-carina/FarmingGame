#pragma once
/*----------------------------------------------------
 HEADER COMMENT

 ground.h
 Random dirt/grass background fill for tiles not covered by a plot
                                   Author: Carina Chao
                                   Date: 2026/08/12
 ----------------------------------------------------*/
#pragma once
#ifndef GROUND_H
#define GROUND_H

#include "crop_plot.h"  // for PlotRegion

void Ground_Initialize();
void Ground_Finalize();
void Ground_Draw();

void Ground_LoadLayout(const PlotRegion regions[], int regionCount);

#endif // GROUND_H