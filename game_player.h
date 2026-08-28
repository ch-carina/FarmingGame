#ifndef GAME_PLAYER_H
#define GAME_PLAYER_H
#include <DirectXMath.h>
#include "collision.h"
#include "crop_plot.h"

enum PlayerAnimState {
	Idle,
	WalkingF,
	WalkingB,
	WalkingL,
	WalkingR,
	Planting,
	Shooting,
	Damaged,
	Harvesting,
	Watering,
	PlayerAnim_MAX
};

enum PlayerFacing {
	Up,
	Down, 
	Left,
	Right
};

struct PLAYER
{
	float x; 
	float y; 

	PlayerAnimState animState;
	int animFrame;
	float animTimer; 
	PlayerFacing facing;
};

void GamePlayer_Initialize(float start_x, float start_y);

void GamePlayer_Finalize();

void GamePlayer_Update(float delta_time);

void GamePlayer_Draw();

void Player_ChangeState(PlayerAnimState newState);

void GamePlayer_AddSpeedBonus(float amount);

CollisionCircle GamePlayer_GetCollisionCircle(); 

bool GamePlayer_TakeDamage(); // returns true if damage was actually applied

#endif GAME_PLAYER_H