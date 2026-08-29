#include "game.h"
#include "level.h"
#include "game_player.h"
#include "config.h"
#include"game_player_bullet.h"
#include "enemy.h"
#include "sprite.h"
#include "collision.h"
#include "explosion.h"
#include "game_score.h"
#include "crop_plot.h"
#include "crop.h"
#include "inventory.h"
#include "Audio.h"
#include "sellBox.h"
#include "font.h"
#include "ground.h"
#include "shop.h"
#include "blur.h"
#include "fade.h"
#include "damage_flash.h"
#include "cloud.h"

#ifdef _DEBUG
#include "collision_debug.h"
#endif // _DEBUG

static int g_AudioID_BGM{ -1 };
static int g_AudioID_Explosion{ -1 };
static int g_LastMoney = -1;

void checkPlayerBulletsvsEnemies();
void checkPlayerVsEnemies();

void Game_Initialize()
{
	constexpr float player_start_x = (SCREEN_WIDTH - 128) * 0.5f;
	constexpr float player_start_y = (SCREEN_HEIGHT - 128)*0.5f;
	Ground_Initialize();
	CropPlot_Initialize();
	Shop_Initialize();
	Level_Initialize();
	GamePlayer_Initialize(player_start_x,player_start_y);
	GamePlayer_BulletInitialize();
	EnemyInitialize();
	CropInitialize();
	Cloud_Initialize();
	Inventory_Initialize();
	Inventory_AddItem(ItemType_WaterPail, 1); //WaterPail for player to water plants
	Inventory_AddItem(ItemType_CarrotSeed, 5); // starting seeds for testing
	Explosion_Initialize();
	GameScore_Initialize(3); // Initialize game score with 3 digits
	Font_Initialize();
	SellBox_Initialize();
	Level_SetCheckpoint(); // Level 1's baseline: 100 coin + starting inventory
	Blur_Initialize();
	DamageFlash_Initialize();
	Fade_Start(FadeType::kIn, 0.5f, { 0.0f,0.0f,0.0f,1.0f });

#ifdef _DEBUG
	CollisionDebug_Initialize();
#endif // _DEBUG
	
	g_AudioID_BGM = LoadAudio("assets/bgm.wav");
		PlayAudio(g_AudioID_BGM,true);
}

void Game_Finalize()
{
	GamePlayer_Finalize();
	Ground_Finalize();
	CropFinalize();
	Cloud_Finalize();
	Shop_Finalize();
	Inventory_Finalize();
	GamePlayer_BulletFinalize();
	EnemyFinalize();
	Explosion_Finalize();
	GameScore_Finalize();
	Font_Finalize();
	SellBox_Finalize();
	Blur_Finalize(); 
	DamageFlash_Finalize();
	Level_Finalize();

#ifdef _DEBUG
	CollisionDebug_Finalize();
#endif // _DEBUG
	UnloadAudio(g_AudioID_BGM);
}

void Game_Update(float delta_time)
{
	Level_Update(delta_time);
	GamePlayer_Update(delta_time);
	CropUpdate(delta_time);
	Cloud_Update(delta_time);
	GamePlayer_BulletUpdate(delta_time);
	EnemyUpdate(delta_time);
	Explosion_Update(delta_time);


	// Collision detection between player bullets and enemies
	checkPlayerBulletsvsEnemies();
	checkPlayerVsEnemies();
	DamageFlash_Update(delta_time);
	GamePlayer_Cleanup(); // Clean up destroyed bullets
	Enemy_Cleanup(); // Clean up destroyed enemies

	Inventory_Update(delta_time);
	SellBox_Update(delta_time);
	Shop_Update(delta_time);

	int currentMoney = SellBox_GetMoney();
	if (currentMoney != g_LastMoney)
	{
		GameScore_SetScore(currentMoney);
		g_LastMoney = currentMoney;
	}

	GameScore_Update(delta_time);
}

static void DrawWorld()
{
	Ground_Draw();
	CropPlot_Draw();
	CropDraw();
	Shop_Draw();
	GamePlayer_Draw();
	GamePlayer_BulletDraw();
	EnemyDraw();
	Cloud_Draw();
	GameScore_Draw();
	Explosion_Draw();
	Inventory_Draw();
	SellBox_Draw();
	DamageFlash_Draw();
	Level_DrawHUD();
}

void Game_Draw()
{
	if (Shop_IsOpen())
	{
		Blur_BeginCapture();
		DrawWorld();
		Blur_EndCapture();
		Blur_DrawBlurred();

		Shop_DrawMenu();
	}
	else
	{
		DrawWorld();
	}

	Level_DrawResult();
}

static bool g_WasTouchingEnemy = false;
void checkPlayerVsEnemies()
{
	CollisionCircle playerCircle = GamePlayer_GetCollisionCircle();
	bool touchingNow = false;

	for (int enemyIndex = 0; enemyIndex < EnemyGetCount(); enemyIndex++)
	{
		CollisionCircle enemyCircle = Enemy_GetCollisionCircle(enemyIndex);
		if (enemyCircle.radius <= 0.0f) continue;

		if (CollisionCircle_IsOverlap(playerCircle, enemyCircle))
		{
			touchingNow = true;
			if (!g_WasTouchingEnemy) // edge-triggered: only fires the moment contact starts
			{
				if (GamePlayer_TakeDamage()) DamageFlash_Start();
			}
			break;
		}
	}

	g_WasTouchingEnemy = touchingNow;
}

void checkPlayerBulletsvsEnemies()
{
	//player bullet vs enemy collision detection
	for (int playerBulletIndex = 0; playerBulletIndex < GamePlayer_BulletGetCount(); playerBulletIndex++)
	{
		CollisionCircle bulletCircle = GamePlayerBullet_GetCollisionCircle(playerBulletIndex);
		if (bulletCircle.radius <= 0.0f) // Skip if the bullet is not active
		{
			continue;
		}
		for (int enemyIndex = 0; enemyIndex < EnemyGetCount(); enemyIndex++)
		{
			CollisionCircle enemyCircle = Enemy_GetCollisionCircle(enemyIndex);
			if (enemyCircle.radius <= 0.0f) // Skip if the enemy is not active
			{
				continue;
			}
			if (CollisionCircle_IsOverlap(bulletCircle, enemyCircle))
			{

				// Handle collision: destroy both the bullet and the enemy
				GamePlayer_BulletDestroy(playerBulletIndex); // Destroy the bullet
				Enemy_Destroy(enemyIndex); // Mark the enemy for destruction
			}
		}
	}
}
