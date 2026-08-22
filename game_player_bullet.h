#ifndef GAME_PLAYER_BULLET
#define GAME_PLAYER_BULLET
#include "collision.h"

#include <DirectXMath.h>


void GamePlayer_BulletInitialize();

void GamePlayer_BulletFinalize();

void GamePlayer_BulletUpdate(float delta_time);

void GamePlayer_BulletDraw();

void GamePlayer_BulletCreate(float x, float y, DirectX::XMFLOAT2 direction);

int GamePlayer_BulletGetCount(); 

void GamePlayer_BulletDestroy(int index); 

void GamePlayer_Cleanup();

CollisionCircle GamePlayerBullet_GetCollisionCircle(int index);


#endif GAME_PLAYER_BULLET