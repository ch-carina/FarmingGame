/*----------------------------------------------------
 HEADER COMMENT

 explosion.cpp
 manages explosion effects in game
								   Author: Carina Chao
								   Date: 2026/07/06
 ----------------------------------------------------*/

#include "explosion.h"
#include "flipbook_animation.h"
#include "texture.h"


struct Explosion
{
	ExplosionType type;
	float x, y;
	float timer;
	int flipbookAnimationID;
	bool isActive;
};

struct ExplosionInfo 
{
	int textureID;
	int patternWidth;
	int patternHeight;
	int patternCount;
	int patternColCount;
	//int patternScale; -> need to update flipbook to have this but this is an option to do later
	float patternUpdateTime;
};

static ExplosionInfo g_ExplosionInfo[ExplosionType_MAX]{
	{ -1, 192, 192, 20, 5, 0.1f }, // enemySmallExplosion
	{ -1, 192, 192, 20, 5, 0.1f }, // enemyBigExplosion
	{ -1, 192, 192, 20, 5, 0.1f },  // bulletExplosion
	{ -1, 96, 96, 6, 3, 0.1f }  // waterSplashVFX
};

constexpr int EXPLOSION_MAX{ 128 };
static Explosion g_Explosions[EXPLOSION_MAX]{};
static int g_ExplosionActiveCount{};

static const wchar_t* EXPLOSION_TEXTURE_FILENAMES[ExplosionType_MAX]{
	L"assets/explosion.png", // enemySmallExplosion
	L"assets/explosion.png", // enemyBigExplosion
	L"assets/explosion.png",  // bulletExplosion
	L"assets/Crops/waterVFX.png"  // waterSplashVFX

};

static int g_TextureIDs[ExplosionType_MAX]{ };

void Explosion_Initialize()
{
	for (int i = 0; i < ExplosionType_MAX; ++i)
	{
		g_ExplosionInfo[i].textureID = Texture_Load(EXPLOSION_TEXTURE_FILENAMES[i]);
	}

	for (Explosion& e : g_Explosions)
	{
		e.isActive = false;
	}
	g_ExplosionActiveCount = 0;
}

void Explosion_Finalize()
{
	for (int i = 0; i < ExplosionType_MAX; ++i)
	{
		Texture_Release(g_ExplosionInfo[i].textureID);
	}
}

void Explosion_Create(ExplosionType type, float x, float y, bool loop)
{
	Explosion& e = g_Explosions[g_ExplosionActiveCount];
	e.type = type;
	e.x = x;
	e.y = y;
	e.timer = 0.0f;
	e.isActive = true;
	g_ExplosionActiveCount++;
	int texID= g_ExplosionInfo[type].textureID;
	e.flipbookAnimationID = FlipBookAnimation_Create(texID, 
		g_ExplosionInfo[type].patternWidth, g_ExplosionInfo[type].patternHeight, 
		g_ExplosionInfo[type].patternCount, g_ExplosionInfo[type].patternColCount, 
		g_ExplosionInfo[type].patternUpdateTime,loop
	);

	g_ExplosionActiveCount++;
}

void Explosion_Update(float delta_time)
{
	FlipBookAnimation_Update(delta_time);
	for (int i = g_ExplosionActiveCount-1; i>=0; --i)
	{
		if (FlipBookAnimation_IsFinished(g_Explosions[i].flipbookAnimationID))
		{
			g_Explosions[i] = g_Explosions[--g_ExplosionActiveCount]; // swap with the last active explosion and decrement the count
		}
	}
}

void Explosion_Draw()
{
	for (int i = 0; i < g_ExplosionActiveCount; ++i)
	{
		Explosion& e = g_Explosions[i];
		if (e.isActive)
		{
			FlipBookAnimation_DrawEx(e.flipbookAnimationID, e.x, e.y,
				g_ExplosionInfo[e.type].patternWidth, g_ExplosionInfo[e.type].patternHeight);
		}
	}
}
