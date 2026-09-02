
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
#include "upgrade.h"
#include "water.h"
#include "Audio.h"
#include "tutorial.h"
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

static bool g_ShowCountdown = false;
static float g_CountdownTimer = 0.0f;
static constexpr float COUNTDOWN_DURATION = 4.0f; // 1 second each for "3", "2", "1", "GO!"
static constexpr float COUNTDOWN_PANEL_WIDTH = 420.0f;
static constexpr float COUNTDOWN_PANEL_HEIGHT = 260.0f;
static int g_AudioID_LevelStart = -1;
static bool g_GoSoundPlayed = false;

//---------------- 
//Tutorial plots  
//----------------
static constexpr PlotRegion g_TutorialRegions[] =
{
    { 4, 2, 2, 2 },   
};

static constexpr ShopItem g_TutorialShopItems[] =
{
    { ItemType_CarrotSeed, 10 },
};

//---------------- 
//Level 1 plots  
//----------------
static constexpr PlotRegion g_Level1Regions[] =
{
    { 2, 1, 2, 2 },   // 2x2 plot (4 slots), top-left tile at (2,1)
    { 6, 2, 3, 3 },
    { 10, 4, 2, 2 },
};

static constexpr PlotRegion g_Level1WaterRegions[] =
{
    { 3, 3, 2, 2 },
    {10,1,3,2}, 
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

static constexpr PlotRegion g_Level2WaterRegions[] =
{
    { 11, 0, 2, 3 },
};

static constexpr ShopItem g_Level2ShopItems[] =
{
    { ItemType_CarrotSeed, 20 },
    { ItemType_WheatSeed, 10 },
    { ItemType_LettuceSeed, 15 },
    { ItemType_CornSeed, 30 },
    { ItemType_Scarecrow, 30 },
};

//---------------- 
//Level 3 plots  
//----------------

static constexpr PlotRegion g_Level3Regions[] =
{
    { 1, 1, 2, 2 },
    { 4, 1, 2, 2 },
    { 7, 1, 2, 2 },
    { 4, 4, 3, 3 },
};

static constexpr PlotRegion g_Level3WaterRegions[] =
{
    { 9, 4, 3, 3 },
};

static constexpr ShopItem g_Level3ShopItems[] =
{
    { ItemType_CarrotSeed, 10 },
    { ItemType_WheatSeed, 20 },
    { ItemType_LettuceSeed, 10 },
    { ItemType_CornSeed, 30 },
    { ItemType_BlueberrySeed, 35 },
    { ItemType_Scarecrow, 30 },
};

//Controls level information 
//What layout
//How man plots 
//time
//Shop items available
//how many are unlocked 
//quota for the level 
//Water plots 
static constexpr LevelLayout g_Levels[Level_MAX] =
{
    { g_TutorialRegions, 1, 9999.0f, g_TutorialShopItems, 1, 0, nullptr, 0 },
    { g_Level1Regions, 3 , 90.0f,  g_Level1ShopItems, 2, 200, g_Level1WaterRegions, 2 },
    { g_Level2Regions, 4 , 120.0f,  g_Level2ShopItems, 5, 400, g_Level2WaterRegions, 1 },
    { g_Level3Regions, 4 , 180.0f, g_Level3ShopItems, 6, 600, g_Level3WaterRegions, 1 },
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
    Upgrade_Initialize();
    Tutorial_Initialize();
    g_PanelCapLeftID = Texture_Load(L"assets/UI/UI_L.PNG", true);
    g_PanelCapMidID = Texture_Load(L"assets/UI/UI_M.PNG", true);
    g_PanelCapRightID = Texture_Load(L"assets/UI/UI_R.PNG", true);
    g_AudioID_LevelStart = LoadAudio("assets/SFX/level_start.wav");
    g_PanelCapWidth = (float)Texture_GetWidth(g_PanelCapLeftID);

    Level_Load(Level1); // change here for testing different levels
}

void Level_Finalize()
{
    Upgrade_Finalize();
    Tutorial_Finalize();
    UnloadAudio(g_AudioID_LevelStart);
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
    Water_LoadRegions(layout.waterRegions, layout.waterRegionCount);
    g_TimeRemaining = layout.timeLimit;

    g_Result = LevelResult_None;
    g_ShowResult = false;

    if (level == LevelTutorial)
    {
        Tutorial_Begin();
        g_ShowCountdown = false; 
    }
    else
    {
        g_ShowCountdown = true;
        g_CountdownTimer = COUNTDOWN_DURATION;
        g_GoSoundPlayed = false;
    }
}

void Level_Update(float delta_time)
{
    if (g_CurrentLevel == LevelTutorial)
    {
        Tutorial_Update(delta_time);
        if (Tutorial_IsComplete())
        {
            SellBox_SetMoney(g_CheckpointMoney);
            Inventory_SetSnapshot(g_CheckpointInventory);
            Level_Load(Level1);
            Level_SetCheckpoint();
        }
        return;
    }
    if (g_ShowCountdown)
    {
        g_CountdownTimer -= delta_time;

        if (!g_GoSoundPlayed && g_CountdownTimer <= COUNTDOWN_DURATION / 4.0f)
        {
            PlayAudio(g_AudioID_LevelStart);
            g_GoSoundPlayed = true;
        }

        if (g_CountdownTimer <= 0.0f)
        {
            g_ShowCountdown = false;
        }
        return;
    }

    if (Upgrade_IsChoiceActive())
    {
        Upgrade_Update(delta_time);
        if (!Upgrade_IsChoiceActive())
        {
            // player just confirmed a choice -- finish the advance we deferred
            Level_Load((LevelType)(g_CurrentLevel + 1));
            Level_SetCheckpoint();
        }
        return;
    }

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
                if (!Upgrade_TryBeginChoice(g_CurrentLevel))
                {
                    Level_Load((LevelType)(g_CurrentLevel + 1)); // advance — money & inventory carry over
                    Level_SetCheckpoint(); // new checkpoint = whatever you carried into this level
                }
                // else: the branch above finishes the advance once the player picks
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
    if (Level_GetCurrent() == LevelTutorial) return;
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

static void DrawCountdownPanel()
{
    float panelX = (SCREEN_WIDTH - COUNTDOWN_PANEL_WIDTH) * 0.5f;
    float panelY = (SCREEN_HEIGHT - COUNTDOWN_PANEL_HEIGHT) * 0.5f;

    Draw3Slice(g_PanelCapLeftID, g_PanelCapMidID, g_PanelCapRightID, g_PanelCapWidth,
        panelX, panelY, COUNTDOWN_PANEL_WIDTH, COUNTDOWN_PANEL_HEIGHT);

    const LevelLayout& layout = g_Levels[g_CurrentLevel];

    char titleStr[16];
    snprintf(titleStr, sizeof(titleStr), "LEVEL %d", g_CurrentLevel);

    char goalStr[32];
    snprintf(goalStr, sizeof(goalStr), "GOAL: %d", layout.moneyQuota);

    int totalSeconds = (int)(layout.timeLimit + 0.5f);
    char timeStr[32];
    snprintf(timeStr, sizeof(timeStr), "TIME LIMIT: %02d:%02d", totalSeconds / 60, totalSeconds % 60);

    float elapsed = COUNTDOWN_DURATION - g_CountdownTimer;
    int phase = (int)(elapsed / (COUNTDOWN_DURATION / 4.0f));
    if (phase < 0) phase = 0;
    if (phase > 3) phase = 3;
    const char* countdownLabels[4] = { "3", "2", "1", "GO!" };
    const char* countdownStr = countdownLabels[phase];

    constexpr float TITLE_SCALE = 3.0f;
    constexpr float INFO_SCALE = 3.0f;
    constexpr float COUNTDOWN_SCALE = 5.0f;

    DirectX::XMFLOAT2 titleSize = Font_MeasureText(titleStr, TITLE_SCALE);
    Font_Print(titleStr, panelX + (COUNTDOWN_PANEL_WIDTH - titleSize.x) * 0.5f, panelY + 30.0f, TITLE_SCALE);

    DirectX::XMFLOAT2 goalSize = Font_MeasureText(goalStr, INFO_SCALE);
    Font_Print(goalStr, panelX + (COUNTDOWN_PANEL_WIDTH - goalSize.x) * 0.5f, panelY + 80.0f, INFO_SCALE);

    DirectX::XMFLOAT2 timeSize = Font_MeasureText(timeStr, INFO_SCALE);
    Font_Print(timeStr, panelX + (COUNTDOWN_PANEL_WIDTH - timeSize.x) * 0.5f, panelY + 80.0f + goalSize.y + 8.0f, INFO_SCALE);

    DirectX::XMFLOAT2 countdownSize = Font_MeasureText(countdownStr, COUNTDOWN_SCALE);
    Font_Print(countdownStr, panelX + (COUNTDOWN_PANEL_WIDTH - countdownSize.x) * 0.5f,
        panelY + COUNTDOWN_PANEL_HEIGHT - countdownSize.y - 50.0f, COUNTDOWN_SCALE);
}

void Level_DrawResult()
{
    if (g_CurrentLevel == LevelTutorial)
    {
        Tutorial_Draw();
        return;
    }

    if (g_ShowCountdown)
    {
        DrawCountdownPanel();
        return;
    }

    if (Upgrade_IsChoiceActive())
    {
        Upgrade_Draw();
        return;
    }

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
    return g_ShowResult || g_ShowCountdown;
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
