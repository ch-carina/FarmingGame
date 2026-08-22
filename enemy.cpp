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
static constexpr float ENEMY_SPAWN_COOLDOWN = 2.0f; //space between spawns 


void EnemyInitialize()
{	
	EnemyAnimation_Initialize();
	g_EnemyCount = 0;
	g_EnemySpawnTimer = 0.0f;
}

void EnemyFinalize()
{
	EnemyAnimation_Finalize();
}

void EnemyCreate(EnemyType type, EnemySpawnSide side, float targetX, float targetY, int plotIndex)
{
	if (g_EnemyCount >= ENEMY_MAX)
	{
		return;
	}

	Enemy& e = g_Enemies[g_EnemyCount];
	e.type = type;
	e.targetX = targetX;
	e.targetY = targetY;
	e.targetPlotIndex = plotIndex;
	e.isDestroyed = false;
	e.eatingTimer = 0.0f;
	e.burrowTimer = 0.0f;

	if (type == EnemyType_Mole)
	{
		// Mole pops up in place — no travel-in
		e.x = targetX;
		e.y = targetY;
		e.dirX = 0.0f;
		e.dirY = 0.0f;
		e.speed = 0.0f;

		e.state = EnemyState_Burrowing;
		e.animState = EnemyAnim_Burrow;
	}
	else
	{
		float spawnX, spawnY;

		switch (side)
		{
		case EnemySpawn_TopLeft:
			spawnX = -ENEMY_WIDTH;
			spawnY = -ENEMY_HEIGHT;
			break;
		case EnemySpawn_TopRight:
			spawnX = SCREEN_WIDTH + ENEMY_WIDTH;
			spawnY = -ENEMY_HEIGHT;
			break;
		case EnemySpawn_Bottom:
			spawnX = targetX;
			spawnY = SCREEN_HEIGHT + ENEMY_HEIGHT;
			break;
		}

		e.x = spawnX;
		e.y = spawnY;

		float dx = targetX - spawnX, dy = targetY - spawnY;
		float len = sqrtf(dx * dx + dy * dy);
		e.dirX = dx / len;
		e.dirY = dy / len;

		switch (type)
		{
		case EnemyType_Rabbit: e.speed = 220.0f; break;
		case EnemyType_Crow:   e.speed = 260.0f; break;
		case EnemyType_Deer:   e.speed = 180.0f; break;
		case EnemyType_Bear:   e.speed = 120.0f; break;
		}

		e.state = EnemyState_Spawn;
		e.animState = EnemyAnim_Run;
	}

	g_EnemyCount++;
}

int EnemyGetCount()
{
	return g_EnemyCount;
}

void Enemy_CheckCropSpawns(float delta_time)
{
	for (int i = 0; i <CropPlot_GetCount(); i++)
	{
		CropPlot* plot = CropPlot_Get(i);

		if (!plot->occupied) continue; 
		if (plot->hasActiveEnemy) continue; // one enemy per crop at a time

		if (plot->spawnCooldownTimer > 0.0f)
		{
			plot->spawnCooldownTimer -= delta_time; 
			continue; 
		}

		const Crop& crop = CropGet(plot->cropIndex);
		if (crop.growthStage == CropGrowth_Planted || crop.growthStage == CropGrowth_Ready) continue;

		CollisionBox box = CropPlot_GetCollision(i);
		float targetX = box.x + box.width * 0.5f;
		float targetY = box.y + box.height * 0.5f;

		EnemySpawnSide side = static_cast<EnemySpawnSide>(rand() % EnemySpawn_MAX);
		EnemyCreate(Enemy_GetTypeForCrop(crop.type), side, targetX, targetY, i);
		plot->hasActiveEnemy = true;
	}
}

void Enemy_Destroy(int index)
{
	if (index < 0 || index >= g_EnemyCount) return;

	Explosion_Create(enemySmallExplosion, g_Enemies[index].x, g_Enemies[index].y, false);

	CropPlot* plot = CropPlot_Get(g_Enemies[index].targetPlotIndex);
	if (plot != nullptr)
	{
		plot->hasActiveEnemy = false;
		plot->spawnCooldownTimer = ENEMY_SPAWN_COOLDOWN;
	}

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
	if (g_Enemies[index].state == EnemyState_Burrowing)
	{
		return { {0.0f, 0.0f}, 0.0f }; // still a dirt mound — no bullet collision yet
	}

	const EnemySpriteInfo& info =
		EnemyAnimation_GetSpriteInfo(g_Enemies[index].type);

	CollisionCircle cc = info.collision;

	cc.center.x += g_Enemies[index].x;
	cc.center.y += g_Enemies[index].y;
	return cc;
}

EnemyType Enemy_GetTypeForCrop(CropType cropType) //This is to assign enemy to crop type 
{
	//add switch cases when more enemies are created 
	//Can use option[rand()%_countof(options) when i make a specific array for the specific crop type to randomize enemy when I want to. 
	switch (cropType)
	{
	case CropType_Carrot:
	{
		static constexpr EnemyType options[] = { EnemyType_Rabbit, EnemyType_Mole };
		return options[rand() % (sizeof(options) / sizeof(options[0]))];
	}
	}
	return EnemyType_Rabbit; // Default case
}

void EnemyUpdate(float delta_time)
{
	Enemy_CheckCropSpawns(delta_time);

	for (int i = 0; i < g_EnemyCount; ++i)
	{
		Enemy& e = g_Enemies[i];
		EnemyAnimation_Update(e, delta_time); 
		switch (e.state)
		{
		case EnemyState_Spawn: // approach
		{
			e.x += e.dirX * e.speed * delta_time;
			e.y += e.dirY * e.speed * delta_time;

			float dx = e.targetX - e.x;
			float dy = e.targetY - e.y;
			float distSq = dx * dx + dy * dy;

			constexpr float ARRIVAL_DIST = 8.0f;

			CropPlot* targetPlot = CropPlot_Get(e.targetPlotIndex);

			if (distSq <= ARRIVAL_DIST * ARRIVAL_DIST)
			{
				if (targetPlot != nullptr && targetPlot->occupied)
				{
					e.state = EnemyState_Eating;
					e.animState = EnemyAnim_Eating;
					e.eatingTimer = 0.0f;
				}
				else
				{
					// nothing left to eat — turn around immediately
					e.state = EnemyState_Return;
					e.animState = EnemyAnim_Escape;
				}
				e.currentFrame = 0;
				e.animTimer = 0.0f;
			}
			break;
		}
		case EnemyState_Alive:
		{
			CropPlot* targetPlot = CropPlot_Get(e.targetPlotIndex);

			// Player touched the mole itself (not a bullet) — crop destroyed instantly, mole flees unharmed
			if (CollisionCircle_IsOverlap(GamePlayer_GetCollisionCircle(), Enemy_GetCollisionCircle(i)))
			{
				if (targetPlot != nullptr && targetPlot->occupied)
				{
					CropPlot_Harvest(e.targetPlotIndex);
				}
				e.state = EnemyState_Return;
				e.animState = EnemyAnim_Burrow;
				e.currentFrame = 0;
				e.animTimer = 0.0f;
				e.burrowTimer = 0.0f;
				break;
			}

			// Left alone — mole finishes the job itself, same pacing as the other animals
			e.eatingTimer += delta_time;
			if (e.eatingTimer >= 2.0f)
			{
				if (targetPlot != nullptr && targetPlot->occupied)
				{
					CropPlot_Harvest(e.targetPlotIndex);
				}
				e.state = EnemyState_Return;
				e.animState = EnemyAnim_Burrow;
				e.currentFrame = 0;
				e.animTimer = 0.0f;
				e.burrowTimer = 0.0f;
			}
			break;
		}
		case EnemyState_Eating:
		{
			e.eatingTimer += delta_time;
			if (e.eatingTimer >= 2.0f)
			{
				CropPlot_Harvest(e.targetPlotIndex); // crop disappears
				e.state = EnemyState_Return;
				e.animState = EnemyAnim_Escape;
				e.currentFrame = 0;
				e.animTimer = 0.0f;
			}
			break;
		}
		case EnemyState_Return:
		{
			if (e.type == EnemyType_Mole)
			{
				e.burrowTimer += delta_time;

				constexpr float MOLE_RETREAT_TIME = 0.5f;
				if (e.burrowTimer >= MOLE_RETREAT_TIME)
				{
					CropPlot* plot = CropPlot_Get(e.targetPlotIndex);
					if (plot != nullptr)
					{
						plot->hasActiveEnemy = false;
						plot->spawnCooldownTimer = ENEMY_SPAWN_COOLDOWN;
					}
					e.isDestroyed = true;
				}
				break;
			}

			e.x -= e.dirX * e.speed * delta_time;
			e.y -= e.dirY * e.speed * delta_time;

			if (e.x < -ENEMY_WIDTH || e.x > SCREEN_WIDTH + ENEMY_WIDTH ||
				e.y < -ENEMY_HEIGHT || e.y > SCREEN_HEIGHT + ENEMY_HEIGHT)
			{
				CropPlot* plot = CropPlot_Get(e.targetPlotIndex);
				if (plot != nullptr)
				{
					plot->hasActiveEnemy = false;
					plot->spawnCooldownTimer = ENEMY_SPAWN_COOLDOWN;
				}
				e.isDestroyed = true;
			}
			break;
		}
		case EnemyState_Burrowing:
		{
			e.burrowTimer += delta_time;

			CropPlot* targetPlot = CropPlot_Get(e.targetPlotIndex);

			// Player touched the plot before the mole popped up — crop is safe, mole bails
			if (targetPlot != nullptr &&
				CircleVsBox(GamePlayer_GetCollisionCircle(), targetPlot->cropCollision))
			{
				targetPlot->hasActiveEnemy = false;
				targetPlot->spawnCooldownTimer = ENEMY_SPAWN_COOLDOWN;
				e.isDestroyed = true;
				break;
			}

			constexpr float MOLE_BURROW_TIME = 1.5f;
			if (e.burrowTimer >= MOLE_BURROW_TIME)
			{
				e.state = EnemyState_Alive;
				e.animState = EnemyAnim_Appear;
				e.currentFrame = 0;
				e.animTimer = 0.0f;
			}
			break;
		}
		}
	}
}

void EnemyDraw()
{
	for (int i = 0; i < g_EnemyCount; i++)
	{
		Enemy& e = g_Enemies[i];

		AnimInfo anim = EnemyAnimation_GetInfo(e.type, e.animState);

		int frame = anim.startFrame + e.currentFrame;
		int column = frame % anim.columns;
		int row = frame / anim.columns;

		int sourceX = column * anim.frameWidth;
		int sourceY = row * anim.frameHeight;

		Sprite_Draw(anim.textureID, e.x, e.y,
			ENEMY_DRAW_SIZE, ENEMY_DRAW_SIZE,
			sourceX, sourceY,
			anim.frameWidth, anim.frameHeight, 0.0f);
	}
}

