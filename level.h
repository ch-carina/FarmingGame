/*----------------------------------------------------
 HEADER COMMENT

 Level.h

 Controls level layouts

                                   Author: Carina Chao
                                   Date: 2026/07/23
 ----------------------------------------------------*/
#pragma once
#ifndef LEVEL_H
#define LEVEL_H

#include "crop_plot.h"
#include "inventory.h"

enum LevelType
{
    Level1,
    Level2,
    Level3,

    Level_MAX
};

struct ShopItem
{
    ItemType item;
    int price;
};

struct LevelLayout
{
    const PlotRegion* regions;
    int regionCount;
    float timeLimit;
    const ShopItem* shopItems;
    int shopItemCount;
    int moneyQuota;
};

enum LevelResult
{
    LevelResult_None,
    LevelResult_Cleared,
    LevelResult_Failed
};

const LevelLayout& Level_GetCurrentLayout();

void Level_Initialize();
void Level_Finalize();

void Level_Load(LevelType level);

void Level_Update(float delta_time);
void Level_DrawHUD();
void Level_DrawResult();

LevelType Level_GetCurrent();

bool Level_IsShowingResult();
float Level_GetTimeRemaining();

void Level_SetCheckpoint(); // call after money/inventory reach the state you want to fall back to

#endif //LEVEL_H