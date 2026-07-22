
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

static constexpr int CROP_MAX = 32;

static Crop g_Crops[CROP_MAX];
static int g_CropCount = 0;

static constexpr float CROP_SEED_TIME = 5.0f; 
static constexpr float CROP_SPROUT_TIME = 10.0f; 


void CropInitialize()
{

    for (int i = 0; i < CROP_MAX; i++)
    {
        g_Crops[i].isActive = false;
        g_Crops[i].growthTime = 0.0f;
        g_Crops[i].animationTimer = 0.0f;
        g_Crops[i].currentFrame = 0;
    }

    CropAnimation_Initialize();
}

void CropFinalize()
{
    CropAnimation_Finalize();
}

void CropCreate(CropType type, float x, float y)
{

    if (g_CropCount >= CROP_MAX)
    {
        return;
    }

    Crop& crop = g_Crops[g_CropCount];

    crop.type = type;

    crop.x = x;
    crop.y = y;

    crop.growthTime = 0.0f;

    g_CropCount++;

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
        crop.growthTime += deltaTime;

        if (crop.growthTime < CROP_SEED_TIME)
        {
            crop.growthStage = CropGrowth_Planted;
        }
        else if (crop.growthTime < CROP_SPROUT_TIME)
        {
            crop.growthStage = CropGrowth_Sprouted;
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
    }
}

void CropDraw()
{

    for (int i = 0; i < g_CropCount; i++)
    {
        Crop& crop = g_Crops[i];

        if (!crop.isActive)
        {
            continue;
        }

        CropAnimInfo anim =
            CropAnimation_GetInfo(
                crop.type,
                crop.growthStage);

        int frame =
            anim.startFrame + crop.currentFrame;

        int column =
            frame % anim.columns;

        int row =
            frame / anim.columns;

        int sourceX =
            column * anim.frameWidth;

        int sourceY =
            row * anim.frameHeight;

        Sprite_Draw(
            anim.textureID,

            crop.x,crop.y,

            anim.frameWidth,anim.frameHeight,

            sourceX,sourceY,

            anim.frameWidth,anim.frameHeight,

            0.0f);
    }


}

const Crop& CropGet(int index)
{
    return g_Crops[index];
}
