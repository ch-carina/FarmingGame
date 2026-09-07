/*----------------------------------------------------
 HEADER COMMENT

 fence.cpp
 Perimeter fencing built around crop plot regions.
 Blocks animals until enough hits break a segment.
                                   Author: Carina Chao
								   Date: 2026/09/04
 ----------------------------------------------------*/
#include "fence.h"
#include "texture.h"
#include "sprite.h"
#include "draw_queue.h"
#include "game_player.h"

enum FenceSlotRole
{
    FenceSlot_TopLeftCorner,
    FenceSlot_TopRightCorner,
    FenceSlot_TopStraight,
    FenceSlot_LeftSide,
    FenceSlot_RightSide,
    FenceSlot_LeftBottomEnd,
    FenceSlot_RightBottomEnd,
    FenceSlot_TopLeftEnd,  
    FenceSlot_TopRightEnd,
    FenceSlot_MiddleDown,
    FenceSlot_MAX
};

struct FenceSlot
{
    float x, y;
    FenceSlotRole role;
    bool built;
    int hitCount;
};

static constexpr int MAX_FENCE_SLOTS = 128;
static FenceSlot g_FenceSlots[MAX_FENCE_SLOTS];
static int g_FenceSlotCount = 0;

static int g_FenceTextures[FenceSlot_MAX];
static constexpr int FENCE_HITS_TO_BREAK = 3;

static bool IsSlotBuiltAt(float x, float y)
{
    for (int i = 0; i < g_FenceSlotCount; i++)
    {
        if (g_FenceSlots[i].x == x && g_FenceSlots[i].y == y) return g_FenceSlots[i].built;
    }
    return false;
}

void Fence_Initialize()
{
    g_FenceTextures[FenceSlot_TopLeftCorner] = Texture_Load(L"assets/GroundTiles/Fence_TL.PNG", true);
    g_FenceTextures[FenceSlot_TopRightCorner] = Texture_Load(L"assets/GroundTiles/Fence_TR.PNG", true);
    g_FenceTextures[FenceSlot_TopStraight] = Texture_Load(L"assets/GroundTiles/Fence_M.PNG", true);
    g_FenceTextures[FenceSlot_LeftSide] = Texture_Load(L"assets/GroundTiles/Fence_FM.PNG", true);
    g_FenceTextures[FenceSlot_RightSide] = Texture_Load(L"assets/GroundTiles/Fence_FM.PNG", true);
    g_FenceTextures[FenceSlot_LeftBottomEnd] = Texture_Load(L"assets/GroundTiles/Fence_Pole.PNG", true);
    g_FenceTextures[FenceSlot_RightBottomEnd] = Texture_Load(L"assets/GroundTiles/Fence_Pole.PNG", true);
    g_FenceTextures[FenceSlot_TopLeftEnd] = Texture_Load(L"assets/GroundTiles/Fence_L.PNG", true);
    g_FenceTextures[FenceSlot_TopRightEnd] = Texture_Load(L"assets/GroundTiles/Fence_R.PNG", true);
    g_FenceTextures[FenceSlot_MiddleDown] = Texture_Load(L"assets/GroundTiles/Fence_MD.png", true);

    g_FenceSlotCount = 0;
}

void Fence_Finalize()
{
    for (int i = 0; i < FenceSlot_MAX; i++)
    {
        Texture_Release(g_FenceTextures[i]);
    }
}

void Fence_LoadRegions(const PlotRegion regions[], int regionCount)
{
    g_FenceSlotCount = 0;

    for (int r = 0; r < regionCount; r++)
    {
        const PlotRegion& region = regions[r];

        // top row: TL corner .. width straights .. TR corner
        for (int col = -1; col <= region.width; col++)
        {
            if (g_FenceSlotCount >= MAX_FENCE_SLOTS) return;

            FenceSlot& slot = g_FenceSlots[g_FenceSlotCount];
            slot.x = (float)(region.gridX + col) * PLOT_SIZE;
            slot.y = (float)(region.gridY - 1) * PLOT_SIZE;
            slot.built = false;
            slot.hitCount = 0;

            if (col == -1)                  slot.role = FenceSlot_TopLeftCorner;
            else if (col == region.width)   slot.role = FenceSlot_TopRightCorner;
            else                             slot.role = FenceSlot_TopStraight;

            g_FenceSlotCount++;
        }

        // left/right sides, below the corner row
        for (int row = 0; row < region.height; row++)
        {
            bool isBottom = (row == region.height - 1);

            if (g_FenceSlotCount < MAX_FENCE_SLOTS)
            {
                FenceSlot& left = g_FenceSlots[g_FenceSlotCount];
                left.x = (float)(region.gridX - 1) * PLOT_SIZE;
                left.y = (float)(region.gridY + row) * PLOT_SIZE;
                left.built = false;
                left.hitCount = 0;
                left.role = isBottom ? FenceSlot_LeftBottomEnd : FenceSlot_LeftSide;
                g_FenceSlotCount++;
            }

            if (g_FenceSlotCount < MAX_FENCE_SLOTS)
            {
                FenceSlot& right = g_FenceSlots[g_FenceSlotCount];
                right.x = (float)(region.gridX + region.width) * PLOT_SIZE;
                right.y = (float)(region.gridY + row) * PLOT_SIZE;
                right.built = false;
                right.hitCount = 0;
                right.role = isBottom ? FenceSlot_RightBottomEnd : FenceSlot_RightSide;
                g_FenceSlotCount++;
            }
        }
    }
}

static bool HasOtherSlotAt(int selfIndex, float x, float y)
{
	for (int i = 0; i < g_FenceSlotCount; i++)
	{
		if (i == selfIndex) continue;
		if (g_FenceSlots[i].x == x && g_FenceSlots[i].y == y) return true;
	}
	return false;
}

void Fence_Draw()
{
    for (int i = 0; i < g_FenceSlotCount; i++)
    {
        if (!g_FenceSlots[i].built) continue;

        int textureID = g_FenceTextures[g_FenceSlots[i].role];
        bool overlapsAnotherRegion = HasOtherSlotAt(i, g_FenceSlots[i].x, g_FenceSlots[i].y);

        if (g_FenceSlots[i].role == FenceSlot_TopLeftCorner || g_FenceSlots[i].role == FenceSlot_TopRightCorner)
        {
            if (overlapsAnotherRegion)
            {
                textureID = g_FenceTextures[FenceSlot_MiddleDown]; // sandwiched between two plots with only one gap tile
            }
            else
            {
                bool connectsDown = IsSlotBuiltAt(g_FenceSlots[i].x, g_FenceSlots[i].y + PLOT_SIZE);
                if (!connectsDown)
                {
                    textureID = (g_FenceSlots[i].role == FenceSlot_TopLeftCorner)
                        ? g_FenceTextures[FenceSlot_TopLeftEnd]
                        : g_FenceTextures[FenceSlot_TopRightEnd];
                }
            }
        }
        else if (g_FenceSlots[i].role == FenceSlot_LeftBottomEnd || g_FenceSlots[i].role == FenceSlot_RightBottomEnd)
        {
            if (overlapsAnotherRegion)
            {
                textureID = g_FenceTextures[FenceSlot_TopStraight]; // same 1-tile-gap situation, at the bottom end of a shorter column instead of the top corner
            }
        }

        DrawQueue_Push(textureID,
            g_FenceSlots[i].x, g_FenceSlots[i].y, PLOT_SIZE, PLOT_SIZE,
            0, 0, 96, 96,
            g_FenceSlots[i].y + PLOT_SIZE);
    }
}

int Fence_GetPlayerSlot()
{
    CollisionCircle playerCircle = GamePlayer_GetCollisionCircle();

    for (int i = 0; i < g_FenceSlotCount; i++)
    {
        if (g_FenceSlots[i].built) continue;

        if (playerCircle.center.x >= g_FenceSlots[i].x && playerCircle.center.x < g_FenceSlots[i].x + PLOT_SIZE &&
            playerCircle.center.y >= g_FenceSlots[i].y && playerCircle.center.y < g_FenceSlots[i].y + PLOT_SIZE)
        {
            return i;
        }
    }
    return -1;
}

bool Fence_IsBuilt(int index)
{
    if (index < 0 || index >= g_FenceSlotCount) return false;
    return g_FenceSlots[index].built;
}

void Fence_Build(int index)
{
    if (index < 0 || index >= g_FenceSlotCount) return;
    g_FenceSlots[index].built = true;
    g_FenceSlots[index].hitCount = 0;
}

void Fence_RegisterHit(int index)
{
    if (index < 0 || index >= g_FenceSlotCount) return;
    if (!g_FenceSlots[index].built) return;

    g_FenceSlots[index].hitCount++;
    if (g_FenceSlots[index].hitCount >= FENCE_HITS_TO_BREAK)
    {
        g_FenceSlots[index].built = false;
        g_FenceSlots[index].hitCount = 0;
    }
}

int Fence_GetBlockingSlot(const CollisionCircle& circle, int ignoreIndex)
{
    for (int i = 0; i < g_FenceSlotCount; i++)
    {
        if (i == ignoreIndex) continue;
        if (!g_FenceSlots[i].built) continue;

        CollisionBox box{ PLOT_SIZE, PLOT_SIZE, g_FenceSlots[i].x, g_FenceSlots[i].y };
        if (CircleVsBox(circle, box)) return i;
    }
    return -1;
}

bool Fence_GetSlotPosition(int index, float& outX, float& outY)
{
    if (index < 0 || index >= g_FenceSlotCount) return false;
    outX = g_FenceSlots[index].x;
    outY = g_FenceSlots[index].y;
    return true;
}

bool Fence_IsBlocked(const CollisionCircle& circle)
{
    return Fence_GetBlockingSlot(circle) != -1;
}