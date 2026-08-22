/*----------------------------------------------------
 HEADER COMMENT

 GameScore.cpp
 Displaying of the score in the game
                                   Author: Carina Chao
                                   Date: 2026/07/08
 ----------------------------------------------------*/

#include "game_score.h"
#include "texture.h"
#include "sprite.h"
#include "config.h"
#include "font.h"
#include <DirectXMath.h>
#include <stdio.h>
using namespace DirectX;

static int g_Digit{};
static int g_Score{};
static float g_AccumulatedTime{};
static int g_TargetScore{};

static int g_TextureID_UI_L{ -1 };
static int g_TextureID_UI_M{ -1 };
static int g_TextureID_UI_R{ -1 };
static int g_TextureID_Coin{ -1 };

static constexpr float PANEL_HEIGHT = 48.0f;
static constexpr float PANEL_CAP_SIZE = 48.0f;   // UI_L/UI_R drawn at half their native 96x96
static constexpr float PANEL_PADDING = 12.0f;    // inner gap between caps and content
static constexpr float PANEL_MARGIN = 20.0f;     // gap from the screen edge
static constexpr float COIN_SIZE = 32.0f;
static constexpr float DIGIT_SIZE = 32.0f;
static constexpr float DIGIT_FONT_SCALE = 5.0f;
static constexpr float ICON_GAP = 4.0f;

void GameScore_Initialize(int digit)
{
	g_Digit = digit; 
	g_Score = 0;
	g_TargetScore = 0;
	g_AccumulatedTime = 0.0f;

	g_TextureID_UI_L = Texture_Load(L"assets/UI/UI_L.PNG", true);
	g_TextureID_UI_M = Texture_Load(L"assets/UI/UI_M.PNG", true);
	g_TextureID_UI_R = Texture_Load(L"assets/UI/UI_R.PNG", true);
	g_TextureID_Coin = Texture_Load(L"assets/UI/coin.PNG", true);
}

void GameScore_Finalize()
{
	Texture_Release(g_TextureID_UI_L);
	Texture_Release(g_TextureID_UI_M);
	Texture_Release(g_TextureID_UI_R);
	Texture_Release(g_TextureID_Coin);
}

void GameScore_SetScore(int score)
{
	g_Score = g_TargetScore;
	g_TargetScore = score;
}

void GameScore_Update(float delta_time)
{
	g_AccumulatedTime += delta_time;
	if (g_AccumulatedTime >= 0.01f) 
	{
		g_AccumulatedTime -= 0.01f; 
		g_Score += 1; 
		if (g_Score > g_TargetScore) 
		{
			g_Score = g_TargetScore;
		}
	}
}

void GameScore_Draw(const XMFLOAT4 color)
{
	float contentWidth =ICON_GAP + g_Digit * DIGIT_SIZE;
	float panelWidth = PANEL_CAP_SIZE * 2.0f + contentWidth + PANEL_PADDING * 2.0f;
	float middleWidth = panelWidth - PANEL_CAP_SIZE * 2.0f;

	float panelX = SCREEN_WIDTH - panelWidth - PANEL_MARGIN;
	float panelY = PANEL_MARGIN;

	// 3-slice background
	Sprite_Draw(g_TextureID_UI_L, panelX, panelY, PANEL_CAP_SIZE, PANEL_HEIGHT, 0, 0, 96, 96, { 1,1,1,1 });
	Sprite_Draw(g_TextureID_UI_M, panelX + PANEL_CAP_SIZE, panelY, middleWidth, PANEL_HEIGHT, 0, 0, 96, 96, { 1,1,1,1 });
	Sprite_Draw(g_TextureID_UI_R, panelX + PANEL_CAP_SIZE + middleWidth, panelY, PANEL_CAP_SIZE, PANEL_HEIGHT, 0, 0, 96, 96, { 1,1,1,1 });

	// Coin icon
	float coinX = panelX + (PANEL_CAP_SIZE - COIN_SIZE) * 0.5f;
	float coinY = panelY + (PANEL_HEIGHT - COIN_SIZE) * 0.5f;
	Sprite_Draw(g_TextureID_Coin, coinX, coinY, COIN_SIZE, COIN_SIZE, 0, 0, 96, 96, { 1,1,1,1 });

	float digitsX = panelX + PANEL_CAP_SIZE + PANEL_PADDING;
	XMFLOAT2 digitSize = Font_MeasureText("0", DIGIT_FONT_SCALE);
	float digitsY = panelY + (PANEL_HEIGHT - digitSize.y) * 0.5f;

	int score = g_Score;
	char digitStr[2] = { '0', '\0' };

	for (int i = 0; i < g_Digit; i++)
	{
		int n = score % 10;
		score /= 10;
		digitStr[0] = (char)('0' + n);

		float dx = digitsX + DIGIT_SIZE * (g_Digit - 1 - i) + (DIGIT_SIZE - digitSize.x) * 0.5f;
		Font_Print(digitStr, dx, digitsY, DIGIT_FONT_SCALE, color);
	}
}
