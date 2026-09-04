
/*----------------------------------------------------
 HEADER COMMENT

 Crop.cpp

 Controls planted crops

                                   Author: Carina Chao
                                   Date: 2026/07/22
 ----------------------------------------------------*/
#include "crop.h"
#include "crop_animation.h"
#include "crop_plot.h"

#include "sprite.h"
#include "texture.h"
#include "explosion.h"
#include "draw_queue.h"

static constexpr int CROP_MAX = 32;

static Crop g_Crops[CROP_MAX];
static int g_CropCount = 0;

struct CropGrowthTiming
{
    float seedDuration;
    float sproutDuration;
    float teenDuration;
};

static constexpr CropGrowthTiming g_CropGrowthTimings[CropType_MAX] =
{
    { 3.0f, 4.0f, 4.0f }, // CropType_Carrot
    { 3.0f, 3.0f, 4.0f }, // CropType_Wheat
    { 3.0f, 5.0f, 5.0f }, // CropType_Lettuce
    { 3.0f, 4.0f, 5.0f }, // CropType_Corn
    { 3.0f, 7.0f, 7.0f }, // CropType_Blueberry
};

static constexpr float WATER_VFX_PLAY_DURATION = 0.6f; 
static constexpr float WATER_VFX_PAUSE = 1.5f; 
static constexpr float WATER_VFX_CYCLE = WATER_VFX_PLAY_DURATION + WATER_VFX_PAUSE;


void CropInitialize()
{

    for (int i = 0; i < CROP_MAX; i++)
    {
        g_Crops[i].isActive = false;
        g_Crops[i].growthTime = 0.0f;
        g_Crops[i].animationTimer = 0.0f;
        g_Crops[i].currentFrame = 0;
		g_Crops[i].wateredCrop = false;
		g_Crops[i].rank = CropRank_Normal;
        g_Crops[i].waterVfxTimer = 0.0f; 
		g_Crops[i].growthPaused = false;
    }

    CropAnimation_Initialize();
}

void CropFinalize()
{
    CropAnimation_Finalize();
}

int CropCreate(CropType type, float x, float y)
{

    if (g_CropCount >= CROP_MAX)
    {
        return -1;
    }

    Crop& crop = g_Crops[g_CropCount];

    crop.type = type;

    crop.x = x;
    crop.y = y;

    crop.growthTime = 0.0f;
	crop.growthStage = CropGrowth_Planted;
	crop.wateredCrop = false;
	crop.rank = CropRank_Normal;
	crop.animationTimer = 0.0f;
    crop.growthPaused = false;
	crop.currentFrame = 0;
	crop.isActive = true;
    crop.waterVfxTimer = 0.0f; 

    int newIndex = g_CropCount; 
    g_CropCount++;
    return newIndex;
}

void Crop_SetGrowthPaused(int index, bool paused) 
{
    if (index < 0 || index >= g_CropCount) return;
    g_Crops[index].growthPaused = paused;
}

void CropUpdate(float deltaTime)
{

    for (int i = 0; i < g_CropCount; i++)
    {
        Crop& crop = g_Crops[i];

        if (!crop.isActive)
        {
            continue;
        }

        CropGrowthState previousStage = crop.growthStage;

        // -----------------------------
        // Update growth time
        // -----------------------------
        if (!crop.growthPaused)
        {
            crop.growthTime += deltaTime;
        }

        const CropGrowthTiming& timing = g_CropGrowthTimings[crop.type];

        float seedTime = timing.seedDuration;
        float sproutTime = seedTime + timing.sproutDuration;
        float teenTime = sproutTime + timing.teenDuration;

        if (crop.growthTime < seedTime)
        {
            crop.growthStage = CropGrowth_Planted;
        }
        else if (crop.growthTime < sproutTime)
        {
            crop.growthStage = CropGrowth_Sprouted;
        }
        else if (crop.growthTime < teenTime)
        {
            crop.growthStage = CropGrowth_Teen;
        }
        else
        {
            crop.growthStage = CropGrowth_Ready;
        }
        // -----------------------------
        // If growth stage changed,
        // restart animation from frame 0
        // -----------------------------
        if (previousStage != crop.growthStage)
        {
            crop.currentFrame = 0;
            crop.animationTimer = 0.0f;
        }

        // -----------------------------
        // Update crop animation
        // -----------------------------
        CropAnimInfo anim =
            CropAnimation_GetInfo(crop.type,crop.growthStage);
        
        if (anim.frameCount <= 0)
        {
            continue; // nothing to animate for this type/stage
        }

        crop.animationTimer += deltaTime;


        if (crop.animationTimer >= anim.frameRate)
        {
            crop.animationTimer = 0.0f;

            crop.currentFrame++;

            if (crop.currentFrame >= anim.frameCount)
            {
                crop.currentFrame = 0;
            }
        }

		//-----------------------------
        //Update watered crop status + repeating splash VFX
		//-----------------------------

        if (crop.wateredCrop == true && crop.growthStage == CropGrowth_Ready)
        {
            CropAnimInfo rankAnim = CropAnimation_GetRankInfo(crop.rank);
            crop.rankAnimTimer += deltaTime;
            if (crop.rankAnimTimer >= rankAnim.frameRate)
            {
                crop.rankAnimTimer = 0.0f;
                crop.rankFrame = (crop.rankFrame + 1) % rankAnim.frameCount;
            }
        }

    }
}

void CropDraw()
{
    for (int i = 0; i < g_CropCount; i++)
    {
        Crop& crop = g_Crops[i];
        if (!crop.isActive) continue;

        CropAnimInfo anim = CropAnimation_GetInfo(crop.type, crop.growthStage);
        if (anim.columns <= 0) continue;

        int frame = anim.startFrame + crop.currentFrame;
        int column = frame % anim.columns;
        int row = frame / anim.columns;
        int sourceX = column * anim.frameWidth;
        int sourceY = row * anim.frameHeight;

        float sortY = crop.y + CROP_DISPLAY_SIZE;

        DrawQueue_Push(anim.textureID, crop.x, crop.y, CROP_DISPLAY_SIZE, CROP_DISPLAY_SIZE,
            sourceX, sourceY, anim.frameWidth, anim.frameHeight, sortY);

        if (crop.wateredCrop && crop.growthStage == CropGrowth_Ready)
        {
            CropAnimInfo rankAnim = CropAnimation_GetRankInfo(crop.rank);
            int rCol = crop.rankFrame % rankAnim.columns;
            int rRow = crop.rankFrame / rankAnim.columns;

            constexpr float BADGE_SIZE = 32.0f;
            DrawQueue_Push(rankAnim.textureID,
                crop.x + CROP_DISPLAY_SIZE - BADGE_SIZE, crop.y,
                BADGE_SIZE, BADGE_SIZE,
                rCol * rankAnim.frameWidth, rRow * rankAnim.frameHeight,
                rankAnim.frameWidth, rankAnim.frameHeight,
                sortY + 0.01f); // sorts just after its crop, so the badge stays on top of it
        }
    }
}

const Crop& CropGet(int index)
{
    return g_Crops[index];
}

void Crop_Destroy(int index)
{
	if (index < 0 || index >= g_CropCount)
	{
		return;
	}
	g_Crops[index].isActive = false;
}

void Crop_ClearAll()
{
    for (int i = 0; i < CROP_MAX; i++)
    {
        g_Crops[i].isActive = false;
    }
    g_CropCount = 0;
}

void Crop_Water(int index)
{
    if (index < 0 || index >= g_CropCount)
    {
        return;
    }
    g_Crops[index].wateredCrop = true;
    g_Crops[index].rank = CropRank_Watered;  
    g_Crops[index].waterVfxTimer = 0.0f;
    Explosion_Create(waterSplashVFX, g_Crops[index].x, g_Crops[index].y, false);
}

int Crop_GetRank(int index)
{
    return g_Crops[index].rank;
}
