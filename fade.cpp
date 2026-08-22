/*----------------------------------------------------
 HEADER COMMENT

 fade.cpp
 fade header - loading (changing in between)
								   Author: Carina Chao
								   Date: 2026/07/10
 ----------------------------------------------------*/
#include "fade.h"
#include "texture.h"
#include "sprite.h"
#include "config.h"
#include <DirectXMath.h> // #include <cmath> Direcmath includes this already
using namespace DirectX;
#include <utility>

static float g_AccumulatedTime{}; 
static int g_TextureID_White{ -1 };
static float g_FadeTime{};
static XMFLOAT4 g_FadeColor{0.0f,0.0f,0.0f,1.0f};

static FadeType g_FadeType{};

void Fade_Initialize()
{
	g_TextureID_White = Texture_Load(L"assets/white.png");
	g_AccumulatedTime = 0.0f;
	g_FadeTime = 0.0f; 
	g_FadeColor.w = 0.0f; 

}

void Fade_Finalize()
{
	Texture_Release(g_TextureID_White);
}

void Fade_Update(float delta_time)
{
	if (g_FadeTime <= 0.0f) return; //dont start fade 

	g_AccumulatedTime += delta_time;

	float alpha{ std::min(1.0f,g_AccumulatedTime / g_FadeTime)};
	
	g_FadeColor.w = g_FadeType == kOut? alpha : 1.0f-alpha;
}

void Fade_Draw()
{
	if (g_FadeTime <= 0.0f || g_AccumulatedTime <= 0.0f || g_FadeColor.w <= 0.0f) { return; }
	Sprite_Draw(g_TextureID_White, 0.0f, 0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, g_FadeColor);
}

void Fade_Start(FadeType type, float fadeTime, const DirectX::XMFLOAT4& color)
{
	g_FadeType = type; 
	g_FadeTime = fadeTime; 
	g_FadeColor = color; 

	g_AccumulatedTime = 0.0f; 
}

bool Fade_IsFinished()
{
	return g_AccumulatedTime / g_FadeTime >=1.0f;
}
