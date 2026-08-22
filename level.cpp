
/*----------------------------------------------------
 HEADER COMMENT

 Level.cpp

 Controls level layouts

                                   Author: Carina Chao
                                   Date: 2026/07/23
 ----------------------------------------------------*/

#include "level.h"
#include "crop_plot.h"
#include "ground.h"
#include "sellBox.h"
#include "shop.h"
#include "font.h"
#include "sprite.h"
#include "texture.h"
#include "input_keyboard.h"
#include "config.h"
#include <cstdio>

static LevelType g_CurrentLevel = Level1;
static float g_TimeRemaining = 0.0f;

static LevelResult g_Result = LevelResult_None;
static bool g_ShowResult = false;

static int g_PanelCapLeftID = TEXTURE_INVALID_ID;
static int g_PanelCapMidID = TEXTURE_INVALID_ID;
static int g_PanelCapRightID = TEXTURE_INVALID_ID;
static float g_PanelCapWidth = 0.0f;

static constexpr float RESULT_PANEL_WIDTH = 500.0f;
static constexpr float RESULT_PANEL_HEIGHT = 220.0f;

static int g_CheckpointMoney = 0;
static InventorySnapshot g_CheckpointInventory;

//---------------- 
//Level 1 plots  
//----------------
static constexpr PlotRegion g_Level1Regions[] =
{
    { 2, 1, 2, 2 },   // 2x2 plot (4 slots), top-left tile at (2,1)
    { 6, 2, 3, 3 },
    { 10, 4, 2, 2 },
};

static constexpr ShopItem g_Level1ShopItems[] =
{
    { ItemType_CarrotSeed, 10 },
    { ItemType_WheatSeed, 20 },
};

//---------------- 
//Level 2 plots  
//----------------

static constexpr PlotRegion g_Level2Regions[] =
{
    { 1, 4, 2, 2 },
    { 4, 2, 3, 3 },
    { 8, 1, 2, 2 },   // second 2x2 plot
    { 11, 4, 2, 2 },
};

static constexpr ShopItem g_Level2ShopItems[] =
{
    { ItemType_CarrotSeed, 20 },
    { ItemType_WheatSeed, 10 },
    { ItemType_LettuceSeed, 15 },
    { ItemType_CornSeed, 30 },
};

//---------------- 
//Level 3 plots  
//----------------

static constexpr PlotRegion g_Level3Regions[] =
{
    { 1, 1, 2, 2 },
    { 4, 1, 2, 2 },
    { 7, 1, 2, 2 },
    { 1, 4, 3, 3 },
};

static constexpr ShopItem g_Level3ShopItems[] =
{
    { ItemType_CarrotSeed, 10 },
    { ItemType_WheatSeed, 20 },
    { ItemType_LettuceSeed, 10 },
    { ItemType_CornSeed, 30 },
    { ItemType_BlueberrySeed, 35 },
};

//Controls level information 
//What layout
//How man plots 
//time
//Shop items available
//how many are unlocked 
//quota for the level 
static constexpr LevelLayout g_Levels[Level_MAX] =
{
    { g_Level1Regions, 3 , 90.0f,  g_Level1ShopItems, 2, 200 },
    { g_Level2Regions, 4 , 120.0f,  g_Level2ShopItems, 4, 400 },
    { g_Level3Regions, 5 , 180.0f, g_Level3ShopItems, 5, 600 },
};

static void Draw3Slice(int leftID, int midID, int rightID, float capWidth, float x, float y, float width, float height)
{
    float midWidth = width - capWidth * 2.0f;
    if (midWidth < 0.0f) midWidth = 0.0f;

    Sprite_Draw(leftID, x, y, capWidth, height);
    Sprite_Draw(midID, x + capWidth, y, midWidth, height);
    Sprite_Draw(rightID, x + capWidth + midWidth, y, capWidth, height);
}

const LevelLayout& Level_GetCurrentLayout()
{
    return g_Levels[g_CurrentLevel];
}

void Level_Initialize()
{
    g_PanelCapLeftID = Texture_Load(L"assets/UI/UI_L.PNG", true);
    g_PanelCapMidID = Texture_Load(L"assets/UI/UI_M.PNG", true);
    g_PanelCapRightID = Texture_Load(L"assets/UI/UI_R.PNG", true);
    g_PanelCapWidth = (float)Texture_GetWidth(g_PanelCapLeftID);

    Level_Load(Level1);
}

void Level_Finalize()
{
    Texture_Release(g_PanelCapLeftID);
    Texture_Release(g_PanelCapMidID);
    Texture_Release(g_PanelCapRightID);
}

void Level_Load(LevelType level)
{
    g_CurrentLevel = level;

    const LevelLayout& layout = g_Levels[level];
    CropPlot_LoadRegions(layout.regions, layout.regionCount);
    Ground_LoadLayout(layout.regions, layout.regionCount);
    g_TimeRemaining = layout.timeLimit;

    g_Result = LevelResult_None;
    g_ShowResult = false;
}

void Level_Update(float delta_time)
{
    if (g_ShowResult)
    {
        if (InputKeyboard_IsTrigger(KK_ENTER))
        {
            if (g_Result == LevelResult_Failed)
            {
                SellBox_SetMoney(g_CheckpointMoney);
                Inventory_SetSnapshot(g_CheckpointInventory);
                Level_Load(g_CurrentLevel); // retry — reset back to the last checkpoint
            }
            else if (g_CurrentLevel + 1 < Level_MAX)
            {
                Level_Load((LevelType)(g_CurrentLevel + 1)); // advance — money & inventory carry over
                Level_SetCheckpoint(); // new checkpoint = whatever you carried into this level
            }
        }
        return;
    }

    g_TimeRemaining -= delta_time;
    if (g_TimeRemaining <= 0.0f)
    {
        g_TimeRemaining = 0.0f;
        g_Result = (SellBox_GetMoney() >= g_Levels[g_CurrentLevel].moneyQuota)
            ? LevelResult_Cleared : LevelResult_Failed;
        g_ShowResult = true;

        if (Shop_IsOpen())
        {
            Shop_Close(); // don't let the shop menu sit on top of the result popup
        }
    }
}

void Level_DrawHUD()
{
    int totalSeconds = (int)(g_TimeRemaining + 0.5f);
    if (totalSeconds < 0) totalSeconds = 0;

    char timeStr[16];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", totalSeconds / 60, totalSeconds % 60);

    char goalStr[32];
    snprintf(goalStr, sizeof(goalStr), "GOAL: %d / %d", SellBox_GetMoney(), g_Levels[g_CurrentLevel].moneyQuota);

    constexpr float TIME_SCALE = 3.0f;
    constexpr float GOAL_SCALE = 2.0f;

    DirectX::XMFLOAT2 timeSize = Font_MeasureText(timeStr, TIME_SCALE);
    DirectX::XMFLOAT2 goalSize = Font_MeasureText(goalStr, GOAL_SCALE);

    Font_Print(timeStr, (SCREEN_WIDTH - timeSize.x) * 0.5f, 20.0f, TIME_SCALE);
    Font_Print(goalStr, (SCREEN_WIDTH - goalSize.x) * 0.5f, 20.0f + timeSize.y + 4.0f, GOAL_SCALE);
}

void Level_DrawResult()
{
    if (!g_ShowResult) return;

    float panelX = (SCREEN_WIDTH - RESULT_PANEL_WIDTH) * 0.5f;
    float panelY = (SCREEN_HEIGHT - RESULT_PANEL_HEIGHT) * 0.5f;

    Draw3Slice(g_PanelCapLeftID, g_PanelCapMidID, g_PanelCapRightID, g_PanelCapWidth,
        panelX, panelY, RESULT_PANEL_WIDTH, RESULT_PANEL_HEIGHT);

    bool finalLevelCleared = (g_Result == LevelResult_Cleared) && (g_CurrentLevel + 1 >= Level_MAX);

    const char* headline = (g_Result == LevelResult_Cleared) ? "LEVEL CLEARED!" : "NOT CLEARED";
    const char* prompt = finalLevelCleared ? "ALL LEVELS COMPLETE"
        : (g_Result == LevelResult_Cleared) ? "Press Enter for Next Level" : "Press Enter to Retry";

    constexpr float HEADLINE_SCALE = 4.0f;
    constexpr float PROMPT_SCALE = 2.2f;

    DirectX::XMFLOAT2 headlineSize = Font_MeasureText(headline, HEADLINE_SCALE);
    DirectX::XMFLOAT2 promptSize = Font_MeasureText(prompt, PROMPT_SCALE);

    Font_Print(headline, panelX + (RESULT_PANEL_WIDTH - headlineSize.x) * 0.5f, panelY + 60.0f, HEADLINE_SCALE);
    Font_Print(prompt, panelX + (RESULT_PANEL_WIDTH - promptSize.x) * 0.5f, panelY + RESULT_PANEL_HEIGHT - 70.0f, PROMPT_SCALE);
}

LevelType Level_GetCurrent()
{
    return g_CurrentLevel;
}

bool Level_IsShowingResult()
{
    return g_ShowResult;
}

float Level_GetTimeRemaining()
{
    return g_TimeRemaining;
}

void Level_SetCheckpoint()
{
    g_CheckpointMoney = SellBox_GetMoney();
    g_CheckpointInventory = Inventory_GetSnapshot();
}
