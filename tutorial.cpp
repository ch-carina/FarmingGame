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
	"Walk onto the glowing plot and hold E to plant seed",
	"An animal is after your crop! Hold SPACE to shoot, arrow keys to aim",
	"Select your Water Pail (number key 1), then press E on the plot",
	"Wait for the crop to finish growing, then press E to harvest it",
	"Carry your crop to the Sell Box (bottom right), select correct inventory slot and then press E to sell",
	"Visit the Shop, press E to open it, and buy something",
	"Tutorial Complete! Press ENTER to officially start farming!",
};

static constexpr int TUTORIAL_PLOT_INDEX = -1; 

static TutorialStep g_Step = TutorialStep_Move;
static XMFLOAT2 g_StartPos{ 0.0f, 0.0f };
static int g_MoneyBaseline = 0;
static bool g_EnemySpawned = false;

static void AdvanceStep(TutorialStep next)
{
	g_Step = next;
	g_MoneyBaseline = SellBox_GetMoney();
	g_EnemySpawned = false;
}

void Tutorial_Begin()
{
	CollisionCircle feet = GamePlayer_GetCollisionCircle();
	g_StartPos = feet.center;
	AdvanceStep(TutorialStep_Move);
}

void Tutorial_Update(float delta_time) 
{
	CropPlot* plot = CropPlot_Get(TUTORIAL_PLOT_INDEX);

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
        if (plot && plot->occupied)
        {
            AdvanceStep(TutorialStep_Combat);
        }
        break;

    case TutorialStep_Combat:
        if (!g_EnemySpawned && plot && plot->occupied)
        {
            float targetX = plot->x + PLOT_SIZE * 0.5f;
            float targetY = plot->y + PLOT_SIZE * 0.5f;
            EnemyCreate(EnemyType_Rabbit, EnemySpawn_Bottom, targetX, targetY, TUTORIAL_PLOT_INDEX);
            plot->hasActiveEnemy = true;
            g_EnemySpawned = true;
        }
        else if (g_EnemySpawned && EnemyGetCount() == 0)
        {
            AdvanceStep(TutorialStep_Water);
        }
        break;

    case TutorialStep_Water:
        if (plot && plot->occupied && Crop_GetRank(plot->cropIndex) == CropRank_Watered)
        {
            AdvanceStep(TutorialStep_Harvest);
        }
        break;

    case TutorialStep_Harvest:
        if (plot && !plot->occupied) // CropPlot_Harvest just cleared it
        {
            AdvanceStep(TutorialStep_Sell);
        }
        break;

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

void Tutorial_Draw()
{
    const char* text = g_Prompts[g_Step];
    constexpr float SCALE = 2.0f;
    XMFLOAT2 size = Font_MeasureText(text, SCALE);
    Font_Print(text, (SCREEN_WIDTH - size.x) * 0.5f, SCREEN_HEIGHT - size.y - 30.0f, SCALE);
}

bool Tutorial_IsComplete()
{
    return g_Step == TutorialStep_Done && InputKeyboard_IsTrigger(KK_ENTER);
}

