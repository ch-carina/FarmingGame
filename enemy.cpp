/*----------------------------------------------------
 HEADER COMMENT

 Enemy.cpp

 controls Enemy Functions
								   Author: Carina Chao
								   Date: 2026/07/01
 ----------------------------------------------------*/

#include "enemy.h"

#include <DirectXMath.h>
#include <cstdlib>
#include <ctime>
#include <stdio.h>

#include "game_player.h"
#include "enemy_animation.h"
#include "sprite.h"
#include "flipbook_animation.h"
#include "config.h"
#include "texture.h"
#include "collision.h"
#include "game.h"
#include "explosion.h"

static int g_TextureID_Enemy = TEXTURE_INVALID_ID;
static constexpr int ENEMY_MAX{ 128 };
static Enemy g_Enemies[ENEMY_MAX]{ };
static int g_EnemyCount = 0;
static constexpr float ENEMY_SPEED{ 100.0f };
static constexpr float ENEMY_WIDTH{ 128.0f };
static constexpr float ENEMY_HEIGHT{ 128.0f };
static constexpr float ENEMY_MOVE_LIMIT_X{ SCREEN_WIDTH - ENEMY_WIDTH };

static int g_patternCount = 0;

static float g_EnemySpawnTimer = 0.0f;
static constexpr float ENEMY_SPAWN_INTERVAL = 3.0f; //space between each enemy spawn by seconds


void EnemyInitialize()
{
	g_TextureID_Enemy = Texture_Load(L"assets/runningman001.png", true);

	
	srand((unsigned int)time(nullptr));
	
	g_EnemyCount = 0;
	g_EnemySpawnTimer = 0.0f;
}

void EnemyFinalize()
{
	Texture_Release(g_TextureID_Enemy);
}

void EnemyCreate(EnemyType type, float x, float y)
{
	if (g_EnemyCount >= ENEMY_MAX)
	{
		return;
	}

	Enemy& e = g_Enemies[g_EnemyCount];
	e.animState = EnemyAnim_Run; 
	e.animTimer = 0.0f; 
	e.currentFrame = 0; 
	e.type = type;
	e.x = x;
	e.y = y;
	e.state = EnemyState_Spawn;
	e.isDestroyed = false;
	g_EnemyCount++;
}

int EnemyGetCount()
{
	return g_EnemyCount;
}

void Enemy_Destroy(int index)
{
	if (index < 0 || index >= g_EnemyCount)
	{
		return;
	}

	Explosion_Create(enemySmallExplosion,g_Enemies[index].x,g_Enemies[index].y,false);

	g_Enemies[index].isDestroyed = true;
}

void Enemy_Cleanup()
{
	for (int i = g_EnemyCount - 1; i >= 0; --i)
	{
		if (g_Enemies[i].isDestroyed)
		{
			g_Enemies[i] = g_Enemies[--g_EnemyCount]; // changes with the last enemy in the array and decrements the count
		}
	}
}

CollisionCircle Enemy_GetCollisionCircle(int index)
{
	const EnemySpriteInfo& info =
		EnemyAnimation_GetSpriteInfo(g_Enemies[index].type);

	CollisionCircle cc = info.collision;

	cc.center.x += g_Enemies[index].x; // I need to check this because sensei used position not center for the collision circle 
	cc.center.y += g_Enemies[index].y;
	return cc;
}

void EnemyUpdate(float delta_time)
{
	g_EnemySpawnTimer += delta_time;

	if (g_EnemySpawnTimer > ENEMY_SPAWN_INTERVAL)
	{
		EnemyType randomType =static_cast<EnemyType>(rand() % EnemyType_MAX);
		float randomY = (float)(rand() % ((int)SCREEN_HEIGHT - (int)ENEMY_HEIGHT));

		EnemyCreate(randomType, SCREEN_WIDTH, randomY);
		g_EnemySpawnTimer = 0.0f;
	}
	
	
	//managing enemies
	for (int i = 0; i < g_EnemyCount; ++i)
	{
		Enemy& e = g_Enemies[i];

		if (e.state == EnemyState_Spawn)
		{
			e.x -= ENEMY_SPEED * delta_time;

			if (e.x + ENEMY_WIDTH < 0.0f)
			{
				e.isDestroyed = true;
			}

		}

		switch (e.type)
		{
		case EnemyType_Rabbit:
			e.x -= 100.0f * delta_time;
			break;

		case EnemyType_Bear:
			e.x -= 50.0f * delta_time;
			break;
		}

		// Destroy when completely off screen
		if (e.x + ENEMY_WIDTH < 0.0f)
		{
			e.isDestroyed = true;
		}
	}
	g_patternCount++;

}

void EnemyDraw()
{
	int current_pattern = (g_patternCount / 5) % 10;


	int column = current_pattern % 5;  // 0-4
	int row = current_pattern / 5;     // 0-1

	int frameWidth = 140;
	int frameHeight = 200;


	for (int i = 0;i < g_EnemyCount;i++)
	{
		if (g_Enemies[i].state != EnemyState_Spawn)
		{
			continue;
		}
		
		switch (g_Enemies[i].type)
		{
		case EnemyType_Rabbit:
			// Draw normal enemy
			Sprite_Draw(g_TextureID_Enemy, g_Enemies[i].x, g_Enemies[i].y,
				64, 150,
				column * frameWidth, row * frameHeight,
				frameWidth, frameHeight, 0.0f);
			break;
		case EnemyType_Bear:
			// Draw big enemy
			Sprite_Draw(g_TextureID_Enemy, g_Enemies[i].x, g_Enemies[i].y,
				128, 200,
				column * frameWidth, row * frameHeight,
				frameWidth, frameHeight, 0.0f);
			break;
		}
	}
}

