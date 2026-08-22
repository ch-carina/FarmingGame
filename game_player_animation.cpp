#include "game_player_animation.h"
#include "texture.h"
#include "sprite.h"

enum PlayerAnimID
{
	g_AnimID_PlayerIdle,
	g_AnimID_PlayerWalkF,
	g_AnimID_PlayerWalkB,
	g_AnimID_PlayerWalkL,
	g_AnimID_PlayerWalkR,
	g_AnimID_PlayerPlant,
	g_AnimID_PlayerShoot,
	g_AnimID_PlayerDamage,
	g_AnimID_PlayerTada,
	g_AnimID_PlayerWater,
	g_AnimID_MAX
};

struct AnimInfo
{
	PlayerAnimID animID;

	float playerWidth;
	float playerHeight;

	int startFrame;
	int frameCount;

	int columns;

	float speed;
};

static AnimInfo g_PlayerAnims[PlayerAnim_MAX] =
{
	{g_AnimID_PlayerIdle, 48.0f,72.0f,0,3,3,0.5f},//Idle Animation 
	{g_AnimID_PlayerWalkF, 48.0f,72.0f,0,8,4,0.15f },//Walking front Animation 
	{g_AnimID_PlayerWalkB, 48.0f,72.0f,0,8,4,0.15f },//Walking back Animation 
	{g_AnimID_PlayerWalkL, 48.0f,72.0f,0,8,4,0.15f },//Walking Left Animation 
	{g_AnimID_PlayerWalkR, 48.0f,72.0f,0,8,4,0.15f },//Walking right Animation 
	{g_AnimID_PlayerPlant, 48.0f,72.0f,0,6,3,0.15f },//Planting Animation 
	{g_AnimID_PlayerShoot, 48.0f,72.0f,0,8,4,0.15f },//Shooting Animation 
	{g_AnimID_PlayerDamage, 48.0f,72.0f,0,8,4,0.15f },//Damaged Animation 
	{g_AnimID_PlayerTada, 48.0f,72.0f,0,8,4,0.15f },//Tada Animation
	{g_AnimID_PlayerWater, 48.0f,72.0f,0,8,4,0.15f } //Watering Animation
};

static int g_AnimID[g_AnimID_MAX];

void PlayerAnimation_Initialize()
{
	g_AnimID[g_AnimID_PlayerIdle] = Texture_Load(L"assets/MC/MC_Idle.png", true);
	g_AnimID[g_AnimID_PlayerWalkF] = Texture_Load(L"assets/MC/MC_Walk.png", true);
	g_AnimID[g_AnimID_PlayerWalkB] = Texture_Load(L"assets/MC/MC_WalkB.png", true);
	g_AnimID[g_AnimID_PlayerWalkL] = Texture_Load(L"assets/MC/MC_WalkL.png", true);
	g_AnimID[g_AnimID_PlayerWalkR] = Texture_Load(L"assets/MC/MC_WalkR.png", true);
	g_AnimID[g_AnimID_PlayerPlant] = Texture_Load(L"assets/MC/MC_Plant.png", true);
	g_AnimID[g_AnimID_PlayerShoot] = Texture_Load(L"assets/MC/MC_Shoot.png", true);
	g_AnimID[g_AnimID_PlayerDamage] = Texture_Load(L"assets/MC/MC_Damage.png", true);
	g_AnimID[g_AnimID_PlayerTada] = Texture_Load(L"assets/MC/MC_Tada.png", true);
	g_AnimID[g_AnimID_PlayerWater] = Texture_Load(L"assets/MC/MC_Water.png", true);
}

void PlayerAnimation_Finalize()
{
	for (int i = 0; i < g_AnimID_MAX; i++)
	{
		Texture_Release(g_AnimID[i]);
	}
}

void PlayerAnimation_Advance(PLAYER& player, float delta_time)
{
	AnimInfo& anim = g_PlayerAnims[player.animState];

	player.animTimer += delta_time;

	if (player.animTimer >= anim.speed)
	{
		player.animTimer = 0.0f;
		player.animFrame++;

		if (player.animFrame >= anim.frameCount)
		{
			player.animFrame = 0;
		}
	}
}

void PlayerAnimation_Draw(const PLAYER& player, float posX, float posY, float width, float height)
{
	AnimInfo& anim = g_PlayerAnims[player.animState];

	int frame = anim.startFrame + player.animFrame;

	int column = frame % anim.columns;
	int row = frame / anim.columns;

	float srcX = column * anim.playerWidth;
	float srcY = row * anim.playerHeight;

	Sprite_Draw(g_AnimID[anim.animID], posX, posY, width, height,
		srcX, srcY, anim.playerWidth, anim.playerHeight, 0.0f);
}