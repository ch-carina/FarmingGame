/*----------------------------------------------------
 HEADER COMMENT

 explosion.h
 manages explosion effects in game 
								   Author: Carina Chao
								   Date: 2026/07/06
 ----------------------------------------------------*/
#ifndef EXPLOSION_H
#define EXPLOSION_H

void Explosion_Initialize();
void Explosion_Finalize();

enum ExplosionType
{
	enemySmallExplosion,
	enemyBigExplosion,
	bulletExplosion,
	ExplosionType_MAX //senseis is called kExplosionMax
};

void Explosion_Create(ExplosionType type, float x, float y, bool loop);
void Explosion_Update(float delta_time);
void Explosion_Draw();

#endif // EXPLOSION_H
