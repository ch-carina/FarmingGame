
/*----------------------------------------------------
 HEADER COMMENT

 CropPlot.cpp

 Controls crop planting areas

                                   Author: Carina Chao
                                   Date: 2026/07/22
 ----------------------------------------------------*/

#include "crop_plot.h"
#include "crop.h"
#include "sprite.h"
#include "texture.h"

static constexpr int PLOT_COUNT = 6;

static CropPlot g_CropPlots[PLOT_COUNT];

static int g_PlotTextures[PlotType_MAX];

const wchar_t* dirtFiles[PlotType_MAX] =
{
    L"assets/GroundTiles/Dirt/Dirt_BL.png",
    L"assets/GroundTiles/Dirt/Dirt_BC.png",
    L"assets/GroundTiles/Dirt/Dirt_BR.png",

    L"assets/GroundTiles/Dirt/Dirt_TL.png",
    L"assets/GroundTiles/Dirt/Dirt_TC.png",
    L"assets/GroundTiles/Dirt/Dirt_TR.png",

    L"assets/GroundTiles/Dirt/Dirt_ML.png",
    L"assets/GroundTiles/Dirt/Dirt_MR.png",
    L"assets/GroundTiles/Dirt/Dirt_Center.png"
};

void CropPlot_Initialize()
{
    for (int i = 0; i < PlotType_MAX; i++)
    {
        g_PlotTextures[i] =
            Texture_Load(dirtFiles[i], true);
    }

    float startX = 300.0f;
    float startY = 550.0f;
    float spacing = 100.0f;

    for (int i = 0; i < PLOT_COUNT; i++)
    {
        g_CropPlots[i].x = startX + (spacing * i);
        g_CropPlots[i].y = startY;

        g_CropPlots[i].occupied = false;
        g_CropPlots[i].cropType = CropType_Carrot;
    }

}

void CropPlot_Update()
{
}

void CropPlot_Draw()
{

    for (int i = 0; i < PLOT_COUNT; i++)
    {
        int textureID =
            g_PlotTextures[g_CropPlots[i].plotType];

        Sprite_Draw(
            textureID,g_CropPlots[i].x,g_CropPlots[i].y,64,64,0,0,64,64,0.0f
        );
    }

}

void CropPlot_Plant(int index, CropType cropType)
{

    if (index < 0 || index >= PLOT_COUNT)
    {
        return;
    }

    if (g_CropPlots[index].occupied)
    {
        return;
    }

    g_CropPlots[index].occupied = true;
    g_CropPlots[index].cropType = cropType;

    CropCreate(
        cropType,
        g_CropPlots[index].x,
        g_CropPlots[index].y
    );

}

void CropPlot_Harvest(int index)
{

    if (index < 0 || index >= PLOT_COUNT)
    {
        return;
    }

    g_CropPlots[index].occupied = false;

}

const CropPlot& CropPlot_Get(int index)
{
    return g_CropPlots[index];
}

int CropPlot_GetCount()
{
    return PLOT_COUNT;
}
