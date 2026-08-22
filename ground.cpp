/*----------------------------------------------------
 HEADER COMMENT

 ground.cpp
 Random dirt/grass background fill for tiles not covered by a plot
                                   Author: Carina Chao
                                   Date: 2026/08/12
 ----------------------------------------------------*/

#include "ground.h"
#include "texture.h"
#include "sprite.h"
#include "config.h"
#include <cstdlib>

static constexpr int TILE_SIZE = 96;
static constexpr int GROUND_TEXTURE_COUNT = 3;

static constexpr int GRID_COLS = (SCREEN_WIDTH + TILE_SIZE - 1) / TILE_SIZE;
static constexpr int GRID_ROWS = (SCREEN_HEIGHT + TILE_SIZE - 1) / TILE_SIZE;
static constexpr int GRID_CELL_COUNT = GRID_COLS * GRID_ROWS;

static int g_GroundTextures[GROUND_TEXTURE_COUNT];
static int g_GroundTile[GRID_CELL_COUNT];   // which texture each cell uses, -1 = covered by a plot
static int g_VerticalGrassRun[GRID_COLS];

static constexpr int WEIGHT_DIRT = 7;
static constexpr int WEIGHT_GRASS_1 = 2;
static constexpr int WEIGHT_GRASS_2 = 1;
static constexpr int WEIGHT_TOTAL = WEIGHT_DIRT + WEIGHT_GRASS_1 + WEIGHT_GRASS_2;

static int PickWeightedTile()
{
    int roll = rand() % WEIGHT_TOTAL;

    if (roll < WEIGHT_DIRT) return 0;
    roll -= WEIGHT_DIRT;

    if (roll < WEIGHT_GRASS_1) return 1;
    return 2;
}

static const wchar_t* groundFiles[GROUND_TEXTURE_COUNT] =
{
    L"assets/GroundTiles/Dirt/Dirt_Green.PNG",
    L"assets/GroundTiles/Dirt/Grass_1.PNG",
    L"assets/GroundTiles/Dirt/Grass_2.PNG",
};


void Ground_Initialize()
{
    for (int i = 0; i < GROUND_TEXTURE_COUNT; i++)
    {
        g_GroundTextures[i] = Texture_Load(groundFiles[i], true);
    }
}

void Ground_Finalize()
{
    for (int i = 0; i < GROUND_TEXTURE_COUNT; i++)
    {
        Texture_Release(g_GroundTextures[i]);
    }
}

static bool IsCoveredByPlot(int col, int row, const PlotRegion regions[], int regionCount)
{
    for (int r = 0; r < regionCount; r++)
    {
        const PlotRegion& region = regions[r];

        if (col >= region.gridX && col < region.gridX + region.width &&
            row >= region.gridY && row < region.gridY + region.height)
        {
            return true;
        }
    }
    return false;
}

void Ground_LoadLayout(const PlotRegion regions[], int regionCount)
{
    for (int col = 0; col < GRID_COLS; col++)
    {
        g_VerticalGrassRun[col] = 0;   // reset in case a previous level left stale values here
    }

    for (int row = 0; row < GRID_ROWS; row++)
    {
        int horizontalGrassRun = 0;

        for (int col = 0; col < GRID_COLS; col++)
        {
            int cell = row * GRID_COLS + col;

            if (IsCoveredByPlot(col, row, regions, regionCount))
            {
                g_GroundTile[cell] = -1;
                horizontalGrassRun = 0;
                g_VerticalGrassRun[col] = 0;
                continue;
            }

            int tileIndex = PickWeightedTile();
            bool isGrass = (tileIndex == 1 || tileIndex == 2);

            if (isGrass && (horizontalGrassRun >= 2 || g_VerticalGrassRun[col] >= 2))
            {
                tileIndex = 0;   // Dirt_Green -- either direction already has a grass pair
                isGrass = false;
            }

            g_GroundTile[cell] = tileIndex;

            horizontalGrassRun = isGrass ? horizontalGrassRun + 1 : 0;
            g_VerticalGrassRun[col] = isGrass ? g_VerticalGrassRun[col] + 1 : 0;
        }
    }
}

void Ground_Draw()
{
    for (int row = 0; row < GRID_ROWS; row++)
    {
        for (int col = 0; col < GRID_COLS; col++)
        {
            int tileIndex = g_GroundTile[row * GRID_COLS + col];

            if (tileIndex < 0)
            {
                continue;
            }

            float x = (float)(col * TILE_SIZE);
            float y = (float)(row * TILE_SIZE);

            Sprite_Draw(g_GroundTextures[tileIndex], x, y, TILE_SIZE, TILE_SIZE, 0, 0, 96, 96, 0.0f);
        }
    }
}
