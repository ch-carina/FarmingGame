/*----------------------------------------------------
 HEADER COMMENT

 cloud.cpp
 Ambient cloud shadows drifting diagonally across the ground and looping,
 soft edges baked into the texture's alpha channel -- no shader needed
								   Author: Carina Chao
								   Date: 2026/08/28
 ----------------------------------------------------*/
#include "cloud.h"
#include "texture.h"
#include "sprite.h"
#include "config.h"
#include <cstdlib>

static constexpr int CLOUD_COUNT = 5;
static constexpr float CLOUD_BASE_WIDTH = 340.0f;
static constexpr float CLOUD_BASE_HEIGHT = 195.0f; // matches Cloud.PNG's ~384x220 aspect ratio
static constexpr float CLOUD_MIN_SPEED = 15.0f;
static constexpr float CLOUD_MAX_SPEED = 30.0f;
static constexpr float CLOUD_MIN_SCALE = 0.8f;
static constexpr float CLOUD_MAX_SCALE = 1.5f;

// direct screen-position translation: top-right to bottom-left = x decreases, y increases
static constexpr float CLOUD_DIR_X = -0.87f;
static constexpr float CLOUD_DIR_Y = 0.5f;

// dark, low-alpha tint -- reads as a shadow passing over the ground rather than a visible cloud
static const DirectX::XMFLOAT4 CLOUD_SHADOW_TINT{ 0.05f, 0.05f, 0.1f, 0.4f };

struct CloudInstance
{
	float x, y; // center, in screen pixels
	float speed;
	float scale;
};

static int g_CloudTextureID = TEXTURE_INVALID_ID;
static CloudInstance g_Clouds[CLOUD_COUNT];

static float RandomRange(float lo, float hi)
{
	return lo + (rand() / (float)RAND_MAX) * (hi - lo);
}

// re-spawns a cloud off the top-right corner once it exits off the bottom-left
static void RespawnCloud(CloudInstance& cloud)
{
	cloud.x = SCREEN_WIDTH + RandomRange(0.0f, 300.0f);
	cloud.y = RandomRange(-150.0f, SCREEN_HEIGHT * 0.4f);
	cloud.speed = RandomRange(CLOUD_MIN_SPEED, CLOUD_MAX_SPEED);
	cloud.scale = RandomRange(CLOUD_MIN_SCALE, CLOUD_MAX_SCALE);
}

void Cloud_Initialize()
{
	g_CloudTextureID = Texture_Load(L"assets/Effects/Cloud.PNG", true);

	// scatter across the whole screen initially so they don't all enter from one corner at once
	for (int i = 0; i < CLOUD_COUNT; i++)
	{
		g_Clouds[i].x = RandomRange(0.0f, SCREEN_WIDTH);
		g_Clouds[i].y = RandomRange(0.0f, SCREEN_HEIGHT);
		g_Clouds[i].speed = RandomRange(CLOUD_MIN_SPEED, CLOUD_MAX_SPEED);
		g_Clouds[i].scale = RandomRange(CLOUD_MIN_SCALE, CLOUD_MAX_SCALE);
	}
}

void Cloud_Finalize()
{
	Texture_Release(g_CloudTextureID);
}

void Cloud_Update(float delta_time)
{
	for (int i = 0; i < CLOUD_COUNT; i++)
	{
		g_Clouds[i].x += CLOUD_DIR_X * g_Clouds[i].speed * delta_time;
		g_Clouds[i].y += CLOUD_DIR_Y * g_Clouds[i].speed * delta_time;

		float halfWidth = (CLOUD_BASE_WIDTH * g_Clouds[i].scale) * 0.5f;
		bool offLeft = g_Clouds[i].x + halfWidth < 0.0f;
		bool offBottom = g_Clouds[i].y - halfWidth > SCREEN_HEIGHT;

		if (offLeft || offBottom)
		{
			RespawnCloud(g_Clouds[i]);
		}
	}
}

void Cloud_Draw()
{
	for (int i = 0; i < CLOUD_COUNT; i++)
	{
		float width = CLOUD_BASE_WIDTH * g_Clouds[i].scale;
		float height = CLOUD_BASE_HEIGHT * g_Clouds[i].scale;

		Sprite_Draw(g_CloudTextureID,
			g_Clouds[i].x - width * 0.5f, g_Clouds[i].y - height * 0.5f,
			width, height,
			CLOUD_SHADOW_TINT);
	}
}