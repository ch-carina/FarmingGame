
/*----------------------------------------------------
 HEADER COMMENT

 CropAnimation.cpp

 Controls crop sprite animations

                                   Author: Carina Chao
                                   Date: 2026/07/22
 ----------------------------------------------------*/

#include "crop_animation.h"

#include "texture.h"
#include "sprite.h"

static CropAnimInfo g_CropAnimations[CropType_MAX][CropGrowth_MAX];
static CropAnimInfo g_CropRankAnimations[CropRank_MAX];


void CropAnimation_Initialize()
{
    //----------------------------------
    // CARROT
    //----------------------------------

    g_CropAnimations[CropType_Carrot][CropGrowth_Planted] =
    {
        Texture_Load(L"assets/Crops/Carrot/Carrot_Planted.png",false),96,96,0,1,0.20f,1
    };

    g_CropAnimations[CropType_Carrot][CropGrowth_Sprouted] =
    {
        Texture_Load(L"assets/Crops/Carrot/Carrot_Sprout.png",false), 96,96,0,12,0.20f,4
    };

    g_CropAnimations[CropType_Carrot][CropGrowth_Teen] =
    {
       Texture_Load(L"assets/Crops/Carrot/Carrot_Teen.png",false), 96,96,0,12,0.20f,4
    };
    
    g_CropAnimations[CropType_Carrot][CropGrowth_Ready] =
    {
       Texture_Load(L"assets/Crops/Carrot/Carrot_Ready.png",false),96,96,0,12,0.20f,4
    };

    //----------------------------------
    // WHEAT
    //----------------------------------
    g_CropAnimations[CropType_Wheat][CropGrowth_Planted] =
    {
        Texture_Load(L"assets/Crops/Wheat/Wheat_Planted.png",false),96,96,0,1,0.20f,1
    };

    g_CropAnimations[CropType_Wheat][CropGrowth_Sprouted] =
    {
        Texture_Load(L"assets/Crops/Wheat/Wheat_Sprout.png",false), 96,96,0,12,0.20f,4
    };

    g_CropAnimations[CropType_Wheat][CropGrowth_Teen] =
    {
       Texture_Load(L"assets/Crops/Wheat/Wheat_Teen.png",false), 96,96,0,12,0.20f,4
    };

    g_CropAnimations[CropType_Wheat][CropGrowth_Ready] =
    {
       Texture_Load(L"assets/Crops/Wheat/Wheat_Ready.png",false),96,96,0,16,0.20f,4
    };

    //----------------------------------
    // CORN
    //----------------------------------
    g_CropAnimations[CropType_Corn][CropGrowth_Planted] =
    {
        Texture_Load(L"assets/Crops/Corn/Corn_Planted.png",false),96,96,0,1,0.20f,1
    };

    g_CropAnimations[CropType_Corn][CropGrowth_Sprouted] =
    {
        Texture_Load(L"assets/Crops/Corn/Corn_Sprout.png",false), 96,96,0,12,0.20f,4
    };

    g_CropAnimations[CropType_Corn][CropGrowth_Teen] =
    {
       Texture_Load(L"assets/Crops/Corn/Corn_Teen.png",false), 96,96,0,8,0.20f,4
    };

    g_CropAnimations[CropType_Corn][CropGrowth_Ready] =
    {
       Texture_Load(L"assets/Crops/Corn/Corn_Ready.png",false),96,96,0,8,0.20f,4
    };

    //----------------------------------
    // Lettuce
    //----------------------------------

    g_CropAnimations[CropType_Lettuce][CropGrowth_Planted] =
    {
        Texture_Load(L"assets/Crops/Lettuce/Lettuce_Planted.PNG",false),96,96,0,1,0.20f,1
    };

    g_CropAnimations[CropType_Lettuce][CropGrowth_Sprouted] =
    {
        Texture_Load(L"assets/Crops/Lettuce/Lettuce_Sprout.PNG",false), 96,96,0,12,0.20f,4
    };

    g_CropAnimations[CropType_Lettuce][CropGrowth_Teen] =
    {
       Texture_Load(L"assets/Crops/Lettuce/Lettuce_Teen.PNG",false), 96,96,0,8,0.20f,4
    };

    g_CropAnimations[CropType_Lettuce][CropGrowth_Ready] =
    {
       Texture_Load(L"assets/Crops/Lettuce/Lettuce_Ready.PNG",false),96,96,0,8,0.20f,4
    };

    //----------------------------------
    // Blueberry
    //----------------------------------

    g_CropAnimations[CropType_Blueberry][CropGrowth_Planted] =
    {
        Texture_Load(L"assets/Crops/Blueberry/Blueberry_Planted.PNG",false),96,96,0,1,0.20f,1
    };

    g_CropAnimations[CropType_Blueberry][CropGrowth_Sprouted] =
    {
        Texture_Load(L"assets/Crops/Blueberry/Blueberry_Sprout.PNG",false), 96,96,0,12,0.20f,4
    };

    g_CropAnimations[CropType_Blueberry][CropGrowth_Teen] =
    {
       Texture_Load(L"assets/Crops/Blueberry/Blueberry_Teen.PNG",false), 96,96,0,8,0.20f,4
    };

    g_CropAnimations[CropType_Blueberry][CropGrowth_Ready] =
    {
       Texture_Load(L"assets/Crops/Blueberry/Blueberry_Ready.PNG",false),96,96,0,8,0.20f,4
    };

    //----------------------------------
    // CROP RANK
    //----------------------------------

    g_CropRankAnimations[CropRank_Normal] =
    {
        Texture_Load(L"assets/Crops/cropRank_Normal.png", false), 96,96,0,4,0.15f,2
    };

    g_CropRankAnimations[CropRank_Watered] =
    {
        Texture_Load(L"assets/Crops/cropRank_Gold.png", false), 96,96,0,4,0.15f,2
    };
}

void CropAnimation_Finalize()
{

    for (int type = 0; type < CropType_MAX; type++)
    {
        for (int stage = 0; stage < CropGrowth_MAX; stage++)
        {
            Texture_Release(
                g_CropAnimations[type][stage].textureID);
        }
    }

    for (int rank = 0; rank < CropRank_MAX; rank++)
        Texture_Release(g_CropRankAnimations[rank].textureID);

}

CropAnimInfo CropAnimation_GetInfo(CropType type, CropGrowthState stage)
{
    return g_CropAnimations[type][stage];
}

CropAnimInfo CropAnimation_GetRankInfo(CropRank rank)
{
    return g_CropRankAnimations[rank];
}




