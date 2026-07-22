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
#include <DirectXMath.h>
#include <stdio.h>
using namespace DirectX;

static int g_Digit{};
static int g_Score{};
static float g_AccumulatedTime{};
static int g_TargetScore{};

static int g_TextureID_Digit{ -1 };

void drawNumber(int n, float x, float y, XMFLOAT4 color);

void GameScore_Initialize(int digit)
{
	g_Digit = digit;
	g_Score = 0;
	g_TargetScore = 0;
	g_AccumulatedTime = 0.0f;

	g_TextureID_Digit = Texture_Load(L"assets/numbers1.png", true);

	printf(
		"Score: %d x %d\n",
		Texture_GetWidth(g_TextureID_Digit),
		Texture_GetHeight(g_TextureID_Digit)
	);

}

void GameScore_Finalize()
{
	Texture_Release(g_TextureID_Digit);
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

void GameScore_Draw(float x, float y, const XMFLOAT4 color)
{
	int score = g_Score;
	for (int i = 0; i < g_Digit; i++)
	{
		int n = score % 10;
		score /= 10;// score =score /10; move to the next digit
		float dx = x+ 32.0f * (g_Digit - 1 - i);
		drawNumber(n, dx, y, color);
	}
}

//show this number at this place in this color 
void drawNumber(int n, float x, float y, const XMFLOAT4 color) 
{
	if (n < 0 || n>=10) return;

	//Height of each digit in the sprite sheet
    float digitWidth = 486.0f / 5.0f;   // 145.4
    float digitHeight = 316.0f / 2.0f;   // 239.5
	//spliting the sprite sheet into 5 columns and 2 rows to get the correct digit
    int column = n % 5;
    int row = n / 5;

	Sprite_Draw(
        g_TextureID_Digit,x,y, 
        32.0f,         // screen width
        32.0f,         // screen height
        digitWidth * column, digitHeight * row,
        digitWidth,digitHeight,color);
}
