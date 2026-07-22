#include "game.h"
#include "game_player.h"
#include "config.h"
#include"game_player_bullet.h"
#include "enemy.h"
#include "sprite.h"
#include "collision.h"
#include "explosion.h"
#include "game_score.h"
#include "Audio.h"

#ifdef _DEBUG
#include "collision_debug.h"
#endif // _DEBUG

static int g_Score{};

static int g_AudioID_BGM{ -1 };
static int g_AudioID_Explosion{ -1 };

void checkPlayerBulletsvsEnemies();

void Game_Initialize()
{
	constexpr float player_start_y = (SCREEN_HEIGHT - 128)*0.5f;
	GamePlayer_Initialize(0.0f,player_start_y);
	GamePlayer_BulletInitialize();
	EnemyInitialize();
	Explosion_Initialize();
	GameScore_Initialize(7); // Initialize game score with 3 digits

	g_Score = 0; 

#ifdef _DEBUG
	CollisionDebug_Initialize();
#endif // _DEBUG
	
	g_AudioID_BGM = LoadAudio("assets/bgm.wav");
		PlayAudio(g_AudioID_BGM,true);
}

void Game_Finalize()
{
	GamePlayer_Finalize();
	GamePlayer_BulletFinalize();
	EnemyFinalize();
	Explosion_Finalize();
	GameScore_Finalize();

#ifdef _DEBUG
	CollisionDebug_Finalize();
#endif // _DEBUG
	UnloadAudio(g_AudioID_BGM);
}

void Game_Update(float delta_time)
{
	GamePlayer_Update(delta_time);
	GamePlayer_BulletUpdate(delta_time);
	EnemyUpdate(delta_time);
	Explosion_Update(delta_time);

	// Collision detection between player bullets and enemies
	checkPlayerBulletsvsEnemies();
	GamePlayer_Cleanup(); // Clean up destroyed bullets
	Enemy_Cleanup(); // Clean up destroyed enemies

	GameScore_Update(delta_time);
}

void Game_Draw()
{
	//AddSetFilter here if you want to 
	GamePlayer_Draw();
	GamePlayer_BulletDraw();
	EnemyDraw();
	GameScore_Draw(1300.0f, 25.0f, { 1.0f, 1.0f, 1.0f, 1.0f });
	Explosion_Draw();

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
				g_Score += 100;
				GameScore_SetScore(g_Score);
				break; // Exit the inner loop since the bullet can only hit one enemy
			}
		}
	}
}
