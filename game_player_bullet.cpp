/*----------------------------------------------------
 HEADER COMMENT

 GamePlayerBullet.cpp
								   Author: Carina Chao
								   Date: 2026/07/01
 ----------------------------------------------------*/
#include "game_player_bullet.h"
#include "game_player.h"
#include "Audio.h"
#include "sprite.h"
#include "config.h"
#include "texture.h"
#include "direct3d.h"
#include "collision.h"
#include "collision_debug.h"

using namespace DirectX; 

struct PlayerBullet
{
	float x, y; //where start shooting position is 
	bool isShooting; //is it still shooting
	bool isDestroyed; //is it destroyed
	XMFLOAT2 direction; // direction of in which the bullet will travel 

};

static int g_TextureID_Bullet = TEXTURE_INVALID_ID;
static constexpr int BULLET_MAX{128};
static PlayerBullet g_Bullets[BULLET_MAX]{ };
static int g_BulletFireCount = 0;
static constexpr float BULLET_SPEED{300.0f};
static constexpr float BULLET_WIDTH{ 32.0f   };
static constexpr float BULLET_HEIGHT{ 32.0f };
static constexpr float BULLET_MOVE_LIMIT_X{ SCREEN_WIDTH };


static float g_BulletFireTimer = 0.0f;
static constexpr float BULLET_FIRE_INTERVAL = 0.15f; //space between each bullet by seconds 

//Declaring Audio for bullet shot 
static int g_AudioID_Shot{};

void GamePlayer_BulletInitialize()
{
	g_TextureID_Bullet = Texture_Load(L"assets/MC/bullet.png", false);
	g_AudioID_Shot = LoadAudio("assets/shot.wav");
	g_BulletFireCount = 0;
	g_BulletFireTimer = 0.0f;
}

void GamePlayer_BulletFinalize()
{
	Texture_Release(g_TextureID_Bullet);
	UnloadAudio(g_AudioID_Shot);
}

void GamePlayer_BulletCreate(float x, float y, XMFLOAT2 direction)
{

	if (g_BulletFireTimer > 0.0f)
	{
		return;
	}

	if (g_BulletFireCount >= BULLET_MAX)
	{
		return;
	}

	PlayerBullet& r = g_Bullets[g_BulletFireCount];
	r.x = x;
	r.y = y;
	r.isShooting = true;
	r.isDestroyed = false;
	r.direction = direction; 

	g_BulletFireCount++;

	g_BulletFireTimer = BULLET_FIRE_INTERVAL;

	PlayAudio(g_AudioID_Shot);

}

int GamePlayer_BulletGetCount()
{
	return g_BulletFireCount;
}

void GamePlayer_BulletDestroy(int index)
{
	if (index < 0 || index >= g_BulletFireCount)
	{
		return;
	}
	g_Bullets[index].isDestroyed = true;
}

//Cleaning up the bullets that are destroyed
void GamePlayer_Cleanup()
{
	for (int i = g_BulletFireCount - 1; i >= 0; --i)
	{
		if (g_Bullets[i].isDestroyed)
		{
			g_Bullets[i] = g_Bullets[--g_BulletFireCount]; // changes with the last bullet in the array and decrements the count
		}
	}
}

CollisionCircle GamePlayerBullet_GetCollisionCircle(int index)
{
	return
	{
		{
			g_Bullets[index].x + BULLET_WIDTH * 0.5f,
			g_Bullets[index].y + BULLET_HEIGHT * 0.5f
		},
		BULLET_WIDTH * 0.5f
	};
}

void GamePlayer_BulletUpdate(float delta_time)
{
	for (int i = 0; i < g_BulletFireCount; ++i)
	{
		g_Bullets[i].x += g_Bullets[i].direction.x * BULLET_SPEED * delta_time;
		g_Bullets[i].y += g_Bullets[i].direction.y * BULLET_SPEED * delta_time;

		bool offScreen =
			g_Bullets[i].x < -BULLET_WIDTH ||
			g_Bullets[i].x > SCREEN_WIDTH ||
			g_Bullets[i].y < -BULLET_HEIGHT ||
			g_Bullets[i].y > SCREEN_HEIGHT;

		if (offScreen)
		{
			GamePlayer_BulletDestroy(i);
		}
	}

	if (g_BulletFireTimer > 0.0f)
	{
		g_BulletFireTimer -= delta_time;
	}

}

void GamePlayer_BulletDraw()
{
	int texW = (int)Texture_GetWidth(g_TextureID_Bullet);
	int texH = (int)Texture_GetHeight(g_TextureID_Bullet);

	for (int i = 0;i < g_BulletFireCount;i++)
	{
		float angle = atan2f(g_Bullets[i].direction.y, g_Bullets[i].direction.x);

		Sprite_Draw(g_TextureID_Bullet, g_Bullets[i].x, g_Bullets[i].y,
			BULLET_WIDTH, BULLET_HEIGHT,
			0, 0, texW, texH,
			angle);
	}

#ifdef _DEBUG
	for (int i = 0;i < g_BulletFireCount;i++)
	{
		CollisionDebug_Draw(GamePlayerBullet_GetCollisionCircle(i));
	}
#endif // _DEBUG
}

