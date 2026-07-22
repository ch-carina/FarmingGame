
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


void CropAnimation_Initialize()
{
    //----------------------------------
    // CARROT
    //----------------------------------

    g_CropAnimations[CropType_Carrot][CropGrowth_Planted] =
    {
        Texture_Load(L"assets/Crops/Carrot/Carrot_Planted.png",true),32,32,0,1,0.20f,0
    };

    g_CropAnimations[CropType_Carrot][CropGrowth_Sprouted] =
    {
        Texture_Load(L"assets/Crops/Carrot/Carrot_Sprout.png",true), 32,32,0,12,0.20f,4
    };

    g_CropAnimations[CropType_Carrot][CropGrowth_Teen] =
    {
       Texture_Load(L"assets/Crops/Carrot/Carrot_Teen.png",true), 32,32,0,12,0.20f,4
    };
    
    g_CropAnimations[CropType_Carrot][CropGrowth_Ready] =
    {
       Texture_Load(L"assets/Crops/Carrot/Carrot_Ready.png",true),32,32,0,12,0.20f,4
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

}

CropAnimInfo CropAnimation_GetInfo(CropType type, CropGrowthState stage)
{
    return g_CropAnimations[type][stage];
}




