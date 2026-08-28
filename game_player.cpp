#include "game_player.h"
#include "game_player_animation.h"
#include "game_player_interaction.h"
#include "texture.h"
#include "sprite.h"
#include "flipbook_animation.h"
#include "input_keyboard.h"
#include "game_player_bullet.h"
#include "direct3d.h"
#include "game.h"
#include "config.h"
#include "collision.h"
#include "collision_debug.h"
#include "crop_plot.h"
#include "inventory.h"
#include "shop.h"
#include "sellBox.h"
#include "explosion.h"
#include "level.h"

#include <DirectXMath.h>
using namespace DirectX;

//player Texture ID 
static PLAYER g_Player{};

//player coordinates (x,y) 
static XMFLOAT2 g_Position{};
static constexpr float PLAYER_WIDTH = 60.0f; // base is 48 
static constexpr float PLAYER_HEIGHT = 92.5f; // base is 74 
static constexpr float PLAYER_MOVE_LIMIT_X{ SCREEN_WIDTH - PLAYER_WIDTH };
static constexpr float PLAYER_MOVE_LIMIT_Y{ SCREEN_HEIGHT - PLAYER_HEIGHT };
static constexpr float PLAYER_DEFAULT_SPEED{ 400.0f };

static float g_Speed{ PLAYER_DEFAULT_SPEED }; //how many pixels does it move per second 

static XMFLOAT2 FacingToDirection(PlayerFacing facing)
{
	switch (facing)
	{
	case Up:    return { 0.0f, -1.0f };
	case Down:  return { 0.0f,  1.0f };
	case Left:  return { -1.0f, 0.0f };
	case Right: return { 1.0f, 0.0f };
	}
	return { 1.0f, 0.0f };
}

static bool isDamaged = false;
static float damageTimer = 0.0f;
constexpr float DAMAGE_DISPLAY_TIME = 0.5f;

static bool isInvincible = false;
static float invincibleTimer = 0.0f;
constexpr float INVINCIBLE_TIME = 1.0f; // grace period so one touch isn't 10 hits/sec

bool GamePlayer_TakeDamage()
{
	if (isInvincible) return false;
	isDamaged = true;
	damageTimer = 0.0f;
	isInvincible = true;
	invincibleTimer = 0.0f;
	return true;
}

void GamePlayer_Initialize(float start_x, float start_y)
{
	PlayerAnimation_Initialize();

	g_Position.x = start_x;
	g_Position.y = start_y;

	g_Player.x = start_x;
	g_Player.y = start_y;

	g_Player.facing = Down;

	g_Player.animFrame = 0;
	g_Player.animTimer = 0.0f;

	//Intializing Input for Keyboard
	InputKeyboard_Initialize();
}

void GamePlayer_Finalize()
{
	PlayerAnimation_Finalize();
}

//Changing Player Animation State 
void Player_ChangeState(PlayerAnimState newState)
{
	if (g_Player.animState != newState)
	{
		g_Player.animState = newState;
		g_Player.animFrame = 0;
		g_Player.animTimer = 0.0f;
	}
}

void GamePlayer_AddSpeedBonus(float amount)
{
	g_Speed += amount; 
}

CollisionCircle GamePlayer_GetCollisionCircle()
{
	return
	{
		{
			g_Player.x + PLAYER_WIDTH * 0.5f,
			g_Player.y + PLAYER_HEIGHT - PLAYER_WIDTH * 0.5f
		},
		PLAYER_WIDTH * 0.5f
	};
}

void GamePlayer_Update(float delta_time)
{
	XMFLOAT2 direction{ 0.0f,0.0f };

	if (Shop_IsOpen() || Level_IsShowingResult())
	{
		Player_ChangeState(Idle);
		return;
	}

	if (isInvincible)
	{
		invincibleTimer += delta_time;
		if (invincibleTimer >= INVINCIBLE_TIME) isInvincible = false;
	}

	if (isDamaged)
	{
		damageTimer += delta_time;
		if (damageTimer >= DAMAGE_DISPLAY_TIME)
		{
			isDamaged = false;
			Player_ChangeState(Idle);
		}
	}

	PlayerInteraction_UpdateHarvestTimer(delta_time);

	if (InputKeyboard_IsPress(KK_W)) direction.y = -1.0f;
	if (InputKeyboard_IsPress(KK_A)) direction.x = -1.0f;
	if (InputKeyboard_IsPress(KK_S)) direction.y = 1.0f;
	if (InputKeyboard_IsPress(KK_D)) direction.x = 1.0f;

	if (direction.y < 0.0f)
	{
		g_Player.facing = Up;
	}
	else if (direction.y > 0.0f)
	{
		g_Player.facing = Down;
	}
	else if (direction.x < 0.0f)
	{
		g_Player.facing = Left;
	}
	else if (direction.x > 0.0f)
	{
		g_Player.facing = Right;
	}

	//Changing PlayerAnim State according to keys 
	if (isDamaged)
	{
		Player_ChangeState(Damaged);
	}
	else if (PlayerInteraction_IsHarvesting())
	{
		Player_ChangeState(Harvesting);
	}
	else if (InputKeyboard_IsPress(KK_SPACE)) // as long as you hold down the space key, shoot a bullet
	{
		Player_ChangeState(Shooting);

		XMFLOAT2 shootDir = FacingToDirection(g_Player.facing);
		float spawnX = g_Position.x + (PLAYER_WIDTH * 0.5f) + shootDir.x * PLAYER_WIDTH * 0.5f;
		float spawnY = g_Position.y + (PLAYER_HEIGHT * 0.5f) + shootDir.y * PLAYER_HEIGHT * 0.5f;

		GamePlayer_BulletCreate(spawnX, spawnY, shootDir);
	}
	else if (InputKeyboard_IsPress(KK_E)) // E to plant -> probably will need to change when figuring out keyboard usage 
	{
		PlayerInteraction_HandleUse(delta_time);
	}
	else if (direction.y < 0.0f)
	{
		Player_ChangeState(WalkingB);
	}
	else if (direction.y > 0.0f)
	{
		Player_ChangeState(WalkingF);
	}
	else if (direction.x < 0.0f)
	{
		Player_ChangeState(WalkingL);
	}
	else if (direction.x > 0.0)
	{
		Player_ChangeState(WalkingR);
	}
	else
	{
		Player_ChangeState(Idle);
	}

	//check to make sure Vector is not 0 
	XMVECTOR velocity = XMLoadFloat2(&direction);

	//Check when the Directional Vectors are not 0
	//Make the Directional Vector equal to 1
	//get the lenght of the vector to the power of 2 
	if (XMVectorGetX(XMVector2LengthSq(velocity)) != 0.0f)
	{
		//if there is player input 
		// normalize vector length 1 
		velocity = XMVector2Normalize(velocity);
		//x/length,y/length -> length is 0 and will become an error 

		velocity *= g_Speed * delta_time;

		//change current coordinates with the vector speed to get new coordinates 
		XMStoreFloat2(&g_Position, XMLoadFloat2(&g_Position) + velocity);
	}


	// Keep player inside screen
	if (g_Position.x < 0.0f)
	{
		g_Position.x = 0.0f;
	}

	if (g_Position.y < 0.0f)
	{
		g_Position.y = 0.0f;
	}

	if (g_Position.x >= PLAYER_MOVE_LIMIT_X)
	{
		g_Position.x = PLAYER_MOVE_LIMIT_X;
	}

	if (g_Position.y >= PLAYER_MOVE_LIMIT_Y)
	{
		g_Position.y = PLAYER_MOVE_LIMIT_Y;
	}

	PlayerAnimation_Advance(g_Player, delta_time);

	g_Player.x = g_Position.x;
	g_Player.y = g_Position.y;
}

void GamePlayer_Draw()
{
	PlayerAnimation_Draw(g_Player, g_Position.x, g_Position.y, PLAYER_WIDTH, PLAYER_HEIGHT);

	if (PlayerInteraction_IsHarvesting())
	{
		constexpr float POPUP_SIZE = 48.0f;
		float popupX = g_Position.x + (PLAYER_WIDTH - POPUP_SIZE) * 0.5f;
		float popupY = g_Position.y - POPUP_SIZE;

		Sprite_Draw(Inventory_GetIconTexture(PlayerInteraction_GetHarvestPopupItem()), popupX, popupY, POPUP_SIZE, POPUP_SIZE,
			0, 0, 96, 96, 0.0f);
	}
}