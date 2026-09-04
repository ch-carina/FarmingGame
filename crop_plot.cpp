
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
#include "collision.h"
#include"game_player.h"
#include "config.h"
#include "draw_queue.h"
#include <algorithm>
#include <cmath>

using namespace std;

static constexpr int MAX_PLOTS = 64;   // generous ceiling across all levels; raise if a level needs more

static CropPlot g_CropPlots[MAX_PLOTS];
static int g_PlotCount = 0;
static int g_PlotTextures[PlotType_MAX];

static int g_ScarecrowTextureID = TEXTURE_INVALID_ID;
static constexpr float SCARECROW_DURATION = 10.0f;

const wchar_t* dirtFiles[PlotType_MAX] =
{
    L"assets/GroundTiles/Dirt/Dirt_TL.png",
    L"assets/GroundTiles/Dirt/Dirt_TC.png",
    L"assets/GroundTiles/Dirt/Dirt_TR.png",


    L"assets/GroundTiles/Dirt/Dirt_ML.png",
    L"assets/GroundTiles/Dirt/Dirt_Center.png",
    L"assets/GroundTiles/Dirt/Dirt_MR.png",

    L"assets/GroundTiles/Dirt/Dirt_BL.png",
    L"assets/GroundTiles/Dirt/Dirt_BC.png",
    L"assets/GroundTiles/Dirt/Dirt_BR.png"
};

CropPlot* CropPlot_Get(int index)
{
    if (index < 0 || index >= g_PlotCount)
    {
        return nullptr;
    }

    return &g_CropPlots[index];
}

int CropPlot_GetCount()
{
    return g_PlotCount;
}

void CropPlot_Initialize()
{
    for (int i = 0; i < PlotType_MAX; i++)
    {
        g_PlotTextures[i] = Texture_Load(dirtFiles[i], true);
    }

    g_ScarecrowTextureID = Texture_Load(L"assets/Crops/Scarecrow.PNG", false);

    g_PlotCount = 0;

}

void CropPlot_Update(float delta_time)
{
    for (int i = 0; i < g_PlotCount; i++)
    {
        if (!g_CropPlots[i].hasScarecrow) continue;

        g_CropPlots[i].scarecrowTimer -= delta_time;
        if (g_CropPlots[i].scarecrowTimer <= 0.0f)
        {
            g_CropPlots[i].hasScarecrow = false;
            g_CropPlots[i].occupied = false;
        }
    }
}

void CropPlot_Draw()
{
    for (int i = 0; i < g_PlotCount; i++)
    {
        int textureID =
            g_PlotTextures[g_CropPlots[i].plotType];

        Sprite_Draw(
            textureID, g_CropPlots[i].x, g_CropPlots[i].y, 96, 96, 0, 0, 96, 96, 0.0f
        );

        if (g_CropPlots[i].hasScarecrow)
        {
			constexpr float SCARECROW_SIZE = CROP_DISPLAY_SIZE*1.3f; // size of the scarecrow sprite when drawn on screen
            constexpr float SCARECROW_OFFSET = (PLOT_SIZE - SCARECROW_SIZE) * 0.5f;

            float scarecrowX = g_CropPlots[i].x + SCARECROW_OFFSET;
            float scarecrowY = g_CropPlots[i].y + SCARECROW_OFFSET;

            DrawQueue_Push(g_ScarecrowTextureID,
                scarecrowX, scarecrowY, CROP_DISPLAY_SIZE, CROP_DISPLAY_SIZE, 0, 0, 96, 96,
                scarecrowY + CROP_DISPLAY_SIZE);
        }
    }
}

void CropPlot_LoadRegions(const PlotRegion regions[], int regionCount)
{
    Crop_ClearAll();
    g_PlotCount = 0;

    for (int r = 0; r < regionCount; r++)
    {
        const PlotRegion& region = regions[r];

        for (int row = 0; row < region.height; row++)
        {
            for (int col = 0; col < region.width; col++)
            {
                if (g_PlotCount >= MAX_PLOTS)
                {
                    return; // safety: stop rather than overflow the fixed-size array
                }

                CropPlot& plot = g_CropPlots[g_PlotCount];

                plot.x = (region.gridX + col) * PLOT_SIZE;
                plot.y = (region.gridY + row) * PLOT_SIZE;

                plot.cropCollision = { PLOT_SIZE, PLOT_SIZE, plot.x, plot.y };

                plot.occupied = false;
                plot.spawnCooldownTimer = 0.0f;
                plot.hasActiveEnemy = false;
                plot.hasScarecrow = false;
                plot.scarecrowTimer = 0.0f;

                // same border classification as before, now using THIS region's own width/height
                if (row == 0 && col == 0)                                  plot.plotType = PlotType_DirtTL;
                else if (row == 0 && col == region.width - 1)              plot.plotType = PlotType_DirtTR;
                else if (row == region.height - 1 && col == 0)             plot.plotType = PlotType_DirtBL;
                else if (row == region.height - 1 && col == region.width - 1) plot.plotType = PlotType_DirtBR;
                else if (row == 0)                                          plot.plotType = PlotType_DirtTC;
                else if (row == region.height - 1)                          plot.plotType = PlotType_DirtBC;
                else if (col == 0)                                          plot.plotType = PlotType_DirtML;
                else if (col == region.width - 1)                           plot.plotType = PlotType_DirtMR;
                else                                                        plot.plotType = PlotType_DirtC;

                g_PlotCount++;
            }
        }
    }
}

void CropPlot_Plant(int index, CropType cropType)
{

    if (index < 0 || index >= g_PlotCount)
    {
        return;
    }

    if (g_CropPlots[index].occupied)
    {
        return;
    }

    g_CropPlots[index].occupied = true;
    g_CropPlots[index].cropType = cropType;

	constexpr float CROP_OFFSET = (PLOT_SIZE - CROP_DISPLAY_SIZE) * 0.5f;

    g_CropPlots[index].cropIndex = CropCreate(
        cropType,
        g_CropPlots[index].x + CROP_OFFSET,
        g_CropPlots[index].y + CROP_OFFSET
    );

}

void CropPlot_Harvest(int index)
{
    if (index < 0 || index >= g_PlotCount) return;

    Crop_Destroy(g_CropPlots[index].cropIndex);

    g_CropPlots[index].occupied = false;
    g_CropPlots[index].hasActiveEnemy = false;
    g_CropPlots[index].spawnCooldownTimer = 0.0f;
    g_CropPlots[index].cropIndex = -1;
}

//check if player is in a crop plot area
bool CircleVsBox(const CollisionCircle& playerCollision, const CollisionBox& cropPlotBox)
{
    float closestX = std::max(cropPlotBox.x,
        std::min(playerCollision.center.x, cropPlotBox.x + cropPlotBox.width));

    float closestY = std::max(cropPlotBox.y,
        std::min(playerCollision.center.y, cropPlotBox.y + cropPlotBox.height));

    float dx = playerCollision.center.x - closestX;
    float dy = playerCollision.center.y - closestY;

    return (dx * dx + dy * dy) <=
        (playerCollision.radius * playerCollision.radius);
}

int CropPlot_GetPlayerPlot()
{
    CollisionCircle playerCircle =
        GamePlayer_GetCollisionCircle();

    for (int i = 0; i < g_PlotCount; i++)
    {
		const CollisionBox& box = g_CropPlots[i].cropCollision;
        if (playerCircle.center.x >= box.x && playerCircle.center.x < box.x + box.width &&
            playerCircle.center.y >= box.y && playerCircle.center.y < box.y + box.height)
        {
            return i;
        }
    }
    return -1;
}

int CropPlot_GetIndexAt(float x, float y)
{
    for (int i = 0; i < g_PlotCount; i++)
    {
        if (g_CropPlots[i].x == x && g_CropPlots[i].y == y)
        {
            return i;
        }
    }
    return -1;
}

CollisionBox CropPlot_GetCollision(int index)
{
    return g_CropPlots[index].cropCollision;
}

void CropPlot_PlaceScarecrow(int index)
{
    if (index < 0 || index >= g_PlotCount) return;
    if (g_CropPlots[index].occupied) return;

    g_CropPlots[index].occupied = true;
    g_CropPlots[index].hasScarecrow = true;
    g_CropPlots[index].scarecrowTimer = SCARECROW_DURATION;
}

bool CropPlot_IsAdjacentToScarecrow(int index)
{
    if (index < 0 || index >= g_PlotCount) return false;

    float px = g_CropPlots[index].x;
    float py = g_CropPlots[index].y;

    for (int i = 0; i < g_PlotCount; i++)
    {
        if (i == index || !g_CropPlots[i].hasScarecrow) continue;

        float dx = g_CropPlots[i].x - px;
        float dy = g_CropPlots[i].y - py;

        bool cardinalNeighbor =
            (fabsf(dx) < 1.0f && fabsf(fabsf(dy) - PLOT_SIZE) < 1.0f) ||
            (fabsf(dy) < 1.0f && fabsf(fabsf(dx) - PLOT_SIZE) < 1.0f);

        if (cardinalNeighbor) return true;
    }
    return false;
}
