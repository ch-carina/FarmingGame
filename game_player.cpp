#include "game_player.h"
#include "texture.h"
#include "sprite.h"
#include "flipbook_animation.h"
#include "input_keyboard.h"
#include "game_player_bullet.h"
#include "direct3d.h"
#include "game.h"
#include "config.h"

#include <DirectXMath.h>
using namespace DirectX;

enum PlayerAnimID
{
	g_AnimID_PlayerIdle,
	g_AnimID_PlayerWalkF,
	g_AnimID_PlayerWalkB,
	g_AnimID_PlayerPlant,
	g_AnimID_PlayerShoot,
	g_AnimID_PlayerDamage,
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
	{g_AnimID_PlayerPlant, 48.0f,72.0f,0,6,3,0.15f },//Planting Animation 
	{g_AnimID_PlayerShoot, 48.0f,72.0f,0,8,4,0.15f },//Shooting Animation 
	{g_AnimID_PlayerDamage, 48.0f,72.0f,0,8,4,0.15f },//Damaged Animation 
};

//player Texture ID 
static PLAYER g_Player{};
static int g_AnimID[g_AnimID_MAX]; 

//player coordinates (x,y) 
static XMFLOAT2 g_Position{};
static constexpr float PLAYER_WIDTH = 48.0f;
static constexpr float PLAYER_HEIGHT = 72.0f;
static constexpr float PLAYER_MOVE_LIMIT_X{SCREEN_WIDTH-PLAYER_WIDTH};
static constexpr float PLAYER_MOVE_LIMIT_Y{ SCREEN_HEIGHT - PLAYER_HEIGHT };
static constexpr float PLAYER_DEFAULT_SPEED{ 200.0f };

static float g_Speed{PLAYER_DEFAULT_SPEED}; //how many pixels does it move per second 

void GamePlayer_Initialize(float start_x,float start_y)
{
	//Call Player Texture 
	g_AnimID[g_AnimID_PlayerIdle] = Texture_Load(L"assets/MC/MC_Idle.png", true);
	g_AnimID[g_AnimID_PlayerWalkF] = Texture_Load(L"assets/MC/MC_Walk.png", true);
	g_AnimID[g_AnimID_PlayerWalkB] = Texture_Load(L"assets/MC/MC_WalkB.png", true);
	g_AnimID[g_AnimID_PlayerPlant] = Texture_Load(L"assets/MC/MC_Plant.png", true);
	g_AnimID[g_AnimID_PlayerShoot] = Texture_Load(L"assets/MC/MC_Shoot.png", true);
	g_AnimID[g_AnimID_PlayerDamage] = Texture_Load(L"assets/MC/MC_Damage.png", true);

	g_Position.x = start_x;
	g_Position.y = start_y; 
	
	g_Player.x = start_x; 
	g_Player.y = start_y;

	g_Player.animFrame = 0; 
	g_Player.animTimer = 0.0f; 

	//Intializing Input for Keyboard
	InputKeyboard_Initialize();
}

void GamePlayer_Finalize()
{
	//Release Player Texture 
	for (int i = 0; i < g_AnimID_MAX; i++)
	{
		Texture_Release(g_AnimID[i]);
	}

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

void GamePlayer_Update(float delta_time)
{

	XMFLOAT2 direction{ 0.0f,0.0f };

	//Get key input and update coordinates 
	InputKeyboard_Update(delta_time);

	float speed = 100.0f;

	if (InputKeyboard_IsPress(KK_W)) // as long as you hold down the W key, the texture will scroll to the right
	{
		//g_PlayerY -= speed * delta_time; // Scroll speed of 100 pixels per second
		direction.y = -1.0f;
	}

	if (InputKeyboard_IsPress(KK_A)) // as long as you hold down the A key, the texture will scroll to the right
	{
		//g_PlayerX -= speed * delta_time; // Scroll speed of 100 pixels per second

		direction.x = -1.0f;
	}

	if (InputKeyboard_IsPress(KK_S)) // as long as you hold down the S key, the texture will scroll to the right
	{
		//g_PlayerY += speed * delta_time; // Scroll speed of 100 pixels per second
		direction.y = 1.0f;
	}

	if (InputKeyboard_IsPress(KK_D)) // as long as you hold down the D key, the texture will scroll to the right
	{
		//g_PlayerX += speed * delta_time; // Scroll speed of 100 pixels per second
		direction.x = 1.0f;
	}

	//Changing PlayerAnima State according to keys 
	if (InputKeyboard_IsPress(KK_SPACE)) // as long as you hold down the space key, shoot a bullet
	{
		Player_ChangeState(Shooting);
		GamePlayer_BulletCreate(g_Position.x + PLAYER_WIDTH, g_Position.y + (PLAYER_HEIGHT - 32.0f) * 0.5f);
	}
	else if (InputKeyboard_IsPress(KK_E)) // E to plant -> probably will need to change when figuring out keyboard usage 
	{
		Player_ChangeState(Planting);
	}
	else if (direction.y < 0.0f)
	{
		Player_ChangeState(WalkingB);
	}
	else if (direction.y > 0.0f)
	{
		Player_ChangeState(WalkingF);
	}
	else if (direction.x != 0.0f)
	{
		Player_ChangeState(WalkingF);
	}
	else
	{
		Player_ChangeState(Idle);
	}

	//check to make sure Vector is not 0 
	XMVECTOR velocity = XMLoadFloat2(& direction);
	
	//Check when the Directional Vectors are not 0
	//Make the Directional Vector equal to 1
	//get the lenght of the vector to the power of 2 
	if (XMVectorGetX(XMVector2LengthSq(velocity))!= 0.0f)
	{
		//if there is player input 
		// normalize vector length 1 
		velocity = XMVector2Normalize(velocity);
		//x/length,y/length -> length is 0 and will become an error 

		velocity *= g_Speed*delta_time;

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


	// Update animation frame
	AnimInfo& anim = g_PlayerAnims[g_Player.animState];

	g_Player.animTimer += delta_time;

	if (g_Player.animTimer >= anim.speed)
	{
		g_Player.animTimer = 0.0f;
		g_Player.animFrame++;

		if (g_Player.animFrame >= anim.frameCount)
		{
			g_Player.animFrame = 0;
		}
	}

	g_Player.x = g_Position.x;
	g_Player.y = g_Position.y;


}

void GamePlayer_Draw()
{
	AnimInfo& anim =
		g_PlayerAnims[g_Player.animState];

	int frame =
		anim.startFrame + g_Player.animFrame;

	int column =
		frame % anim.columns;

	int row =
		frame / anim.columns;

	float srcX =
		column * anim.playerWidth;

	float srcY =
		row * anim.playerHeight;


	Sprite_Draw(g_AnimID[anim.animID],g_Position.x,g_Position.y,PLAYER_WIDTH,PLAYER_HEIGHT,
	srcX,srcY,anim.playerWidth,	anim.playerHeight,0.0f);


	
}

