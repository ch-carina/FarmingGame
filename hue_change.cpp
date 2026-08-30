/*----------------------------------------------------
 HEADER COMMENT

 hue_change.cpp

 changes hue as time passes to get day affect 

								   Author: Carina Chao
								   Date: 2026/08/30
 ----------------------------------------------------*/
#include "hue_change.h"
#include "texture.h"
#include "sprite.h"
#include "level.h"
#include "config.h"
#include <DirectXMath.h>
using namespace DirectX;

static int g_WhiteTextureID = TEXTURE_INVALID_ID;

static const XMFLOAT3 SUNSET_TINT{ 1.0f, 0.55f, 0.35f };
static const XMFLOAT3 NIGHT_TINT{ 0.4f, 0.3f, 0.65f };

static float Lerp(float a, float b, float t) { return a + (b - a) * t; }

void HueChange_Initialize()
{
	g_WhiteTextureID = Texture_Load(L"assets/white.png", false);
}

void HueChange_Finalize()
{
	Texture_Release(g_WhiteTextureID);
}

void HueChange_DrawOverlay()
{
	LevelType level = Level_GetCurrent();

	float progress = 0.0f;
	const LevelLayout& layout = Level_GetCurrentLayout();
	if (layout.timeLimit > 0.0f)
	{
		progress = 1.0f - (Level_GetTimeRemaining() / layout.timeLimit);
		if (progress < 0.0f) progress = 0.0f;
		if (progress > 1.0f) progress = 1.0f;
	}

	constexpr float MAX_TINT_STRENGTH = 0.45f; // even at "full" tint, the world stays visible underneath

	XMFLOAT3 tintColor{ 1.0f, 1.0f, 1.0f };
	float blendFactor = 0.0f;

	if (level == Level2)
	{
		// starts at sunset, shifts to night across the whole level
		tintColor.x = Lerp(SUNSET_TINT.x, NIGHT_TINT.x, progress);
		tintColor.y = Lerp(SUNSET_TINT.y, NIGHT_TINT.y, progress);
		tintColor.z = Lerp(SUNSET_TINT.z, NIGHT_TINT.z, progress);
		blendFactor = MAX_TINT_STRENGTH; // fully applied from the very start, not ramped in
	}
	else if (level == Level3)
	{
		tintColor.x = Lerp(SUNSET_TINT.x, NIGHT_TINT.x, progress);
		tintColor.y = Lerp(SUNSET_TINT.y, NIGHT_TINT.y, progress);
		tintColor.z = Lerp(SUNSET_TINT.z, NIGHT_TINT.z, progress);
		blendFactor = MAX_TINT_STRENGTH;
	}

	if (blendFactor <= 0.0f) return;

	Sprite_Draw(g_WhiteTextureID, 0.0f, 0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT,
		{ tintColor.x, tintColor.y, tintColor.z, blendFactor });
}