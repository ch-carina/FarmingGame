/*----------------------------------------------------
 HEADER COMMENT

 tutorial.cpp
								   Author: Carina Chao
								   Date: 2026/09/01
 ----------------------------------------------------*/
#include "tutorial.h"
#include "game_player.h"
#include "crop_plot.h"
#include "crop.h"
#include "inventory.h"
#include "sellBox.h"
#include "enemy.h"
#include "font.h"
#include "input_keyboard.h"
#include "config.h"
#include "sprite.h"
#include "texture.h"
#include <cmath>

using namespace DirectX;

enum TutorialStep
{
	TutorialStep_Move,
	TutorialStep_Plant,
	TutorialStep_Combat,
	TutorialStep_Water,
	TutorialStep_Harvest,
	TutorialStep_Sell,
	TutorialStep_Shop,
	TutorialStep_Done,
	TutorialStep_MAX
};

static const char* g_Prompts[TutorialStep_MAX] =
{
	"Use W A S D to move around",
	"Walk onto the glowing plot, \npress 2 to select your Carrot Seed, \nthen walk onto the plot and hold E to plant",
	"An animal is after your crop! Hold SPACE to shoot, arrow keys to aim",
	"Select your Water Pail (number key 1), then press E on the plot",
	"Wait for the crop to finish growing, then press E to harvest it",
	"Carry your crop to the Sell Box (bottom right), \nselect correct inventory slot \nand then press E to sell",
	"Visit the Shop, press E to open it, and buy something",
	"Tutorial Complete! Press ENTER to officially start farming!",
};

static TutorialStep g_Step = TutorialStep_Move;
static XMFLOAT2 g_StartPos{ 0.0f, 0.0f };
static int g_MoneyBaseline = 0;
static bool g_EnemySpawned = false;
static int g_TrackedPlot = -1;
static int g_TextureID_White = TEXTURE_INVALID_ID;
static float g_GlowTimer = 0.0f;

static void AdvanceStep(TutorialStep next)
{
	g_Step = next;
	g_MoneyBaseline = SellBox_GetMoney();
	g_EnemySpawned = false;
}

void Tutorial_Initialize()
{
    g_TextureID_White = Texture_Load(L"assets/white.png");
}

void Tutorial_Finalize()
{
    Texture_Release(g_TextureID_White);
}

void Tutorial_Begin()
{
	CollisionCircle feet = GamePlayer_GetCollisionCircle();
	g_StartPos = feet.center;
	AdvanceStep(TutorialStep_Move);
}

static void UpdateSproutPause()
{
    if (g_TrackedPlot == -1)return;

    CropPlot* plot = CropPlot_Get(g_TrackedPlot);
    if (!plot || !plot->occupied)return;

    const Crop& crop = CropGet(plot->cropIndex);
    bool watered = Crop_GetRank(plot->cropIndex) == CropRank_Watered;

    Crop_SetGrowthPaused(plot->cropIndex, crop.growthStage == CropGrowth_Sprouted && !watered);
}

void Tutorial_Update(float delta_time)
{
    g_GlowTimer += delta_time;
    UpdateSproutPause();

    switch (g_Step)
    {
    case TutorialStep_Move:
    {
        CollisionCircle feet = GamePlayer_GetCollisionCircle();
        float dx = feet.center.x - g_StartPos.x;
        float dy = feet.center.y - g_StartPos.y;
        if (dx * dx + dy * dy > 80.0f * 80.0f)
        {
            AdvanceStep(TutorialStep_Plant);
        }
        break;
    }

    case TutorialStep_Plant:
        for (int i = 0; i < CropPlot_GetCount(); i++)
        {
            CropPlot* p = CropPlot_Get(i);
            if (p && p->occupied)
            {
                g_TrackedPlot = i;
                AdvanceStep(TutorialStep_Combat);
                break;
            }
        }
        break;

    case TutorialStep_Combat:
    {
        CropPlot* plot = CropPlot_Get(g_TrackedPlot);
        if (!g_EnemySpawned && plot && plot->occupied)
        {
            float targetX = plot->x + PLOT_SIZE * 0.5f;
            float targetY = plot->y + PLOT_SIZE * 0.5f;
            EnemyCreate(EnemyType_Rabbit, EnemySpawn_Bottom, targetX, targetY, g_TrackedPlot);
            plot->hasActiveEnemy = true;
            g_EnemySpawned = true;
        }
        else if (g_EnemySpawned && EnemyGetCount() == 0)
        {
            AdvanceStep(TutorialStep_Water);
        }
        break;
    }

    case TutorialStep_Water:
    {
        CropPlot* plot = CropPlot_Get(g_TrackedPlot);
        if (plot && plot->occupied && Crop_GetRank(plot->cropIndex) == CropRank_Watered)
        {
            AdvanceStep(TutorialStep_Harvest);
        }
        break;
    }

    case TutorialStep_Harvest:
    {
        CropPlot* plot = CropPlot_Get(g_TrackedPlot);
        if (plot && !plot->occupied)
        {
            AdvanceStep(TutorialStep_Sell);
        }
        break;
    }

    case TutorialStep_Sell:
        if (SellBox_GetMoney() > g_MoneyBaseline)
        {
            AdvanceStep(TutorialStep_Shop);
        }
        break;

    case TutorialStep_Shop:
        if (SellBox_GetMoney() < g_MoneyBaseline)
        {
            g_Step = TutorialStep_Done;
        }
        break;

    case TutorialStep_Done:
        break;
    }
}

static void DrawPlantGlow()
{
    if (g_Step != TutorialStep_Plant) return;

    float pulse = (sinf(g_GlowTimer * 4.0f) + 1.0f) * 0.5f; // 0..1
    XMFLOAT4 glowColor{ 1.0f, 0.9f, 0.2f, 0.25f + pulse * 0.35f }; // gold, alpha 0.25-0.6

    for (int i = 0; i < CropPlot_GetCount(); i++)
    {
        CropPlot* p = CropPlot_Get(i);
        if (!p || p->occupied) continue;

        Sprite_Draw(g_TextureID_White, p->x, p->y, PLOT_SIZE, PLOT_SIZE, glowColor);
    }
}

void Tutorial_Draw()
{
	DrawPlantGlow();

    const char* text = g_Prompts[g_Step];
    constexpr float SCALE = 3.0f;
    XMFLOAT2 size = Font_MeasureText(text, SCALE);
    Font_Print(text, (SCREEN_WIDTH - size.x) * 0.5f, SCREEN_HEIGHT - size.y - 150.0f, SCALE);
}

bool Tutorial_IsComplete()
{
    return g_Step == TutorialStep_Done && InputKeyboard_IsTrigger(KK_ENTER);
}

