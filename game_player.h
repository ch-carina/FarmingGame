#ifndef GAME_PLAYER_H
#define GAME_PLAYER_H
#include <DirectXMath.h>

enum PlayerAnimState {
	Idle,
	WalkingF,
	WalkingB,
	Planting,
	Shooting,
	Damaged,
	PlayerAnim_MAX
};

struct PLAYER
{
	float x; 
	float y; 

	PlayerAnimState animState;
	int animFrame;
	float animTimer; 
};

void GamePlayer_Initialize(float start_x, float start_y);

void GamePlayer_Finalize();

void GamePlayer_Update(float delta_time);

void GamePlayer_Draw();

void Player_ChangeState(PlayerAnimState newState);


#endif GAME_PLAYER_H