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
#include "water.h"
#include "draw_queue.h"

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

static int g_ShadowTextureID = TEXTURE_INVALID_ID;
static constexpr float SHADOW_WIDTH = PLAYER_WIDTH * 1.3f;
static constexpr float SHADOW_HEIGHT = PLAYER_WIDTH * 0.6f;
static const DirectX::XMFLOAT4 SHADOW_TINT{ 0.0f, 0.0f, 0.0f, 0.75f };

static XMFLOAT2 g_LastMoveDirection{ 0.0f, 1.0f }; // defaults to facing down, matches initial g_Player.facing

static constexpr float KNOCKBACK_SPEED = 350.0f;
static constexpr float KNOCKBACK_DURATION = 0.25f;
static XMFLOAT2 g_KnockbackVelocity{ 0.0f, 0.0f };
static float g_KnockbackTimer = 0.0f;

static int g_FillBarFrameTextureID = TEXTURE_INVALID_ID;
static int g_FillBarFillTextureID = TEXTURE_INVALID_ID;
static constexpr float FILL_BAR_WIDTH = 64.0f;
static constexpr float FILL_BAR_HEIGHT = 18.0f;
static constexpr float FILL_BAR_INSET = 4.0f; // how far the fill sits inside the frame's border

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

static XMFLOAT2 FacingToAimDirection(PlayerFacing facing)
{
	constexpr float DIAG = 0.70710678f; 

	bool aimLeft = InputKeyboard_IsPress(KK_LEFT);
	bool aimRight = InputKeyboard_IsPress(KK_RIGHT);
	bool aimUp = InputKeyboard_IsPress(KK_UP);
	bool aimDown = InputKeyboard_IsPress(KK_DOWN);

	switch (facing)
	{
	case Up:
		if (aimLeft)  return { -DIAG, -DIAG };
		if (aimRight) return { DIAG, -DIAG };
		return { 0.0f, -1.0f };

	case Down:
		if (aimLeft)  return { -DIAG, DIAG };
		if (aimRight) return { DIAG, DIAG };
		return { 0.0f, 1.0f };

	case Left:
		if (aimUp)    return { -DIAG, -DIAG };
		if (aimDown)  return { -DIAG, DIAG };
		return { -1.0f, 0.0f };

	case Right:
		if (aimUp)    return { DIAG, -DIAG };
		if (aimDown)  return { DIAG, DIAG };
		return { 1.0f, 0.0f };
	}
	return FacingToDirection(facing);
}

static PlayerAnimState FacingToWalkState(PlayerFacing facing)
{
	switch (facing)
	{
	case Up:    return WalkingB;
	case Down:  return WalkingF;
	case Left:  return WalkingL;
	case Right: return WalkingR;
	}
	return WalkingF;
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

	g_KnockbackVelocity = { -g_LastMoveDirection.x * KNOCKBACK_SPEED, -g_LastMoveDirection.y * KNOCKBACK_SPEED };
	g_KnockbackTimer = KNOCKBACK_DURATION;

	return true;
}

void GamePlayer_DrawPopup()
{
	if (PlayerInteraction_IsFilling())
	{
		float barX = g_Position.x + (PLAYER_WIDTH - FILL_BAR_WIDTH) * 0.5f;
		float barY = g_Position.y - FILL_BAR_HEIGHT - 10.0f;

		Sprite_Draw(g_FillBarFrameTextureID, barX, barY, FILL_BAR_WIDTH, FILL_BAR_HEIGHT);

		float fillWidth = (FILL_BAR_WIDTH - FILL_BAR_INSET * 2.0f) * PlayerInteraction_GetFillProgress();
		if (fillWidth > 0.0f)
		{
			Sprite_Draw(g_FillBarFillTextureID,
				barX + FILL_BAR_INSET, barY + FILL_BAR_INSET,
				fillWidth, FILL_BAR_HEIGHT - FILL_BAR_INSET * 2.0f);
		}
	}
	if (PlayerInteraction_IsHarvesting())
	{
		constexpr float POPUP_SIZE = 48.0f;
		float popupX = g_Position.x + (PLAYER_WIDTH - POPUP_SIZE) * 0.5f;
		float popupY = g_Position.y - POPUP_SIZE;

		Sprite_Draw(Inventory_GetIconTexture(PlayerInteraction_GetHarvestPopupItem()), popupX, popupY, POPUP_SIZE, POPUP_SIZE,
			0, 0, 96, 96, 0.0f);
	}
}

void GamePlayer_Initialize(float start_x, float start_y)
{
	PlayerAnimation_Initialize();
	g_ShadowTextureID = Texture_Load(L"assets/MC/Shadow.PNG", true);
	g_FillBarFrameTextureID = Texture_Load(L"assets/UI/FillBar_L.PNG", true);
	g_FillBarFillTextureID = Texture_Load(L"assets/UI/Fill.PNG", true);

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
	Texture_Release(g_ShadowTextureID);
	Texture_Release(g_FillBarFrameTextureID);
	Texture_Release(g_FillBarFillTextureID);
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

	if (direction.x != 0.0f || direction.y != 0.0f)
	{
		XMVECTOR dirVec = XMVector2Normalize(XMLoadFloat2(&direction));
		XMStoreFloat2(&g_LastMoveDirection, dirVec);
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

		XMFLOAT2 shootDir = FacingToAimDirection(g_Player.facing);;
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
		Player_ChangeState(FacingToWalkState(g_Player.facing));
	}

	//check to make sure Vector is not 0 
	XMVECTOR velocity = XMLoadFloat2(&direction);

	if (g_KnockbackTimer > 0.0f)
	{
		g_KnockbackTimer -= delta_time;

		XMVECTOR knockbackVelocity = XMLoadFloat2(&g_KnockbackVelocity);
		XMStoreFloat2(&g_Position, XMLoadFloat2(&g_Position) + knockbackVelocity * delta_time);
	}
	else if (XMVectorGetX(XMVector2LengthSq(velocity)) != 0.0f)
	{
		velocity = XMVector2Normalize(velocity);
		velocity *= g_Speed * delta_time;

		XMFLOAT2 moveDelta;
		XMStoreFloat2(&moveDelta, velocity);

		constexpr float feetRadius = PLAYER_WIDTH * 0.5f;
		float feetOffsetY = PLAYER_HEIGHT - PLAYER_WIDTH * 0.5f;

		float candidateX = g_Position.x + moveDelta.x;
		CollisionCircle feetAtX{ { candidateX + feetRadius, g_Position.y + feetOffsetY }, feetRadius };
		if (!Water_IsBlocked(feetAtX) && !Shop_IsBlocking(feetAtX) && !SellBox_IsBlocking(feetAtX))
		{
			g_Position.x = candidateX;
		}

		float candidateY = g_Position.y + moveDelta.y;
		CollisionCircle feetAtY{ { g_Position.x + feetRadius, candidateY + feetOffsetY }, feetRadius };
		if (!Water_IsBlocked(feetAtY) && !Shop_IsBlocking(feetAtY) && !SellBox_IsBlocking(feetAtY))
		{
			g_Position.y = candidateY;
		}
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

	g_Player.x = g_Position.x;
	g_Player.y = g_Position.y;

	bool isMoving = (direction.x != 0.0f || direction.y != 0.0f);
	PlayerAnimation_Advance(g_Player, delta_time, isMoving);
}

void GamePlayer_Draw()
{
	float shadowCenterX = g_Position.x + PLAYER_WIDTH * 0.5f;
	float shadowCenterY = g_Position.y + PLAYER_HEIGHT - 10.0f;
	float playerSortY = g_Position.y + PLAYER_HEIGHT;

	int shadowTexW = (int)Texture_GetWidth(g_ShadowTextureID);
	int shadowTexH = (int)Texture_GetHeight(g_ShadowTextureID);

	DrawQueue_Push(g_ShadowTextureID,
		shadowCenterX - SHADOW_WIDTH * 0.5f, shadowCenterY - SHADOW_HEIGHT * 0.5f,
		SHADOW_WIDTH, SHADOW_HEIGHT,
		0, 0, shadowTexW, shadowTexH,
		playerSortY - 0.01f, 0.0f, SHADOW_TINT); // sorts just before the player, so it stays underfoot

	PlayerAnimation_Draw(g_Player, g_Position.x, g_Position.y, PLAYER_WIDTH, PLAYER_HEIGHT);
}