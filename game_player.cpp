#include "game_player.h"
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

static constexpr ItemType g_CropHarvestItem[CropType_MAX] = {
	ItemType_Carrot,    // CropType_Carrot
	ItemType_Wheat,     // CropType_Wheat
	ItemType_Lettuce,   // CropType_Lettuce
	ItemType_Corn,      // CropType_Corn
	ItemType_Blueberry, // CropType_Blueberry
};

static constexpr ItemType g_CropHarvestItemGold[CropType_MAX] = {
	ItemType_CarrotGold, // CropType_Carrot
	ItemType_Wheat,      
	ItemType_Lettuce,     
	ItemType_Corn,       
	ItemType_Blueberry,   
};

static constexpr ItemType g_SeedForCrop[CropType_MAX] = {
	ItemType_CarrotSeed,     // CropType_Carrot
	ItemType_WheatSeed,      // CropType_Wheat
	ItemType_LettuceSeed,    // CropType_Lettuce
	ItemType_CornSeed,       // CropType_Corn
	ItemType_BlueberrySeed,  // CropType_Blueberry
};

//player Texture ID 
static PLAYER g_Player{};
static int g_AnimID[g_AnimID_MAX]; 

//player coordinates (x,y) 
static XMFLOAT2 g_Position{};
static constexpr float PLAYER_WIDTH = 60.0f; // base is 48 
static constexpr float PLAYER_HEIGHT = 92.5f; // base is 74 
static constexpr float PLAYER_MOVE_LIMIT_X{SCREEN_WIDTH-PLAYER_WIDTH};
static constexpr float PLAYER_MOVE_LIMIT_Y{ SCREEN_HEIGHT - PLAYER_HEIGHT };
static constexpr float PLAYER_DEFAULT_SPEED{ 200.0f };

static float g_Speed{PLAYER_DEFAULT_SPEED}; //how many pixels does it move per second 

//player planting times 
float plantingTimer = 0.0f;
const float PLANT_TIME = 2.0f; //time it takes to plant a crop
int currentPlotIndex = -1; //index of the current plot being planted

//Harvesting information 
static constexpr float HARVEST_DISPLAY_TIME = 1.0f; 
bool isHarvesting = false;
float harvestTimer = 0.0f;
CropRank pendingHarvestRank = CropRank_Normal;
CropType pendingHarvestType = CropType_Carrot;
bool harvestWasWatered = false; 

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

static ItemType SeedForCrop(CropType type)
{
	switch (type)
	{
	case CropType_Carrot:    return ItemType_CarrotSeed;
	case CropType_Wheat:     return ItemType_WheatSeed;
	case CropType_Lettuce:   return ItemType_LettuceSeed;
	case CropType_Corn:      return ItemType_CornSeed;
	case CropType_Blueberry: return ItemType_BlueberrySeed;
	}
	return ItemType_None;
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

void GamePlayer_Initialize(float start_x,float start_y)
{
	//Call Player Texture 
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

static bool CropForSeed(ItemType item, CropType& outType)
{
	for (int type = 0; type < CropType_MAX; type++)
	{
		if (g_SeedForCrop[type] == item)
		{
			outType = (CropType)type;
			return true;
		}
	}
	return false;
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

	float speed = 200.0f;
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

	if (!isDamaged)
	{
		if (InputKeyboard_IsPress(KK_W)) direction.y = -1.0f;
		if (InputKeyboard_IsPress(KK_A)) direction.x = -1.0f;
		if (InputKeyboard_IsPress(KK_S)) direction.y = 1.0f;
		if (InputKeyboard_IsPress(KK_D)) direction.x = 1.0f;
	}

	if (isHarvesting)
	{
		harvestTimer += delta_time;
		if (harvestTimer >= HARVEST_DISPLAY_TIME)
		{
			ItemType harvestedItem = (pendingHarvestRank == CropRank_Watered)
				? g_CropHarvestItemGold[pendingHarvestType]
				: g_CropHarvestItem[pendingHarvestType];
			Inventory_AddItem(harvestedItem, 1);
			isHarvesting = false;
			Player_ChangeState(Idle);
		}
	}

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
	else if (isHarvesting)
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
		if (Shop_IsPlayerNear())
		{
			if (InputKeyboard_IsTrigger(KK_E))
			{
				Shop_Open();
			}
		}
		else if (SellBox_IsPlayerNear())
		{ 
			if (InputKeyboard_IsTrigger(KK_E))
			{
				SellBox_TrySell();
			}
		}
		else 
		{
			int overlappingPlot = CropPlot_GetPlayerPlot();

			if (currentPlotIndex == -1)
			{
				currentPlotIndex = overlappingPlot;
			}
			else if (overlappingPlot != currentPlotIndex)
			{
				plantingTimer = 0.0f;
				currentPlotIndex = -1;
			}
			 
			CropPlot* plot = CropPlot_Get(currentPlotIndex);
			bool validPlot = currentPlotIndex != -1 && plot-> occupied;
			bool readyToHarvest = validPlot && CropGet(plot->cropIndex).growthStage == CropGrowth_Ready;
			bool hasPailSelected = Inventory_GetSlotItem(Inventory_GetSelectedSlot()) == ItemType_WaterPail;
			bool needsWater = validPlot && !readyToHarvest && Crop_GetRank(plot->cropIndex) != CropRank_Watered&& hasPailSelected;
			CropType selectedCropType = CropType_Carrot;

			if (readyToHarvest)
			{
				bool wasWatered = Crop_GetRank(plot->cropIndex) == CropRank_Watered;
				pendingHarvestRank = wasWatered ? CropRank_Watered : CropRank_Normal;
				pendingHarvestType = CropGet(plot->cropIndex).type;   // add this line

				CropPlot_Harvest(currentPlotIndex);
				isHarvesting = true;
				harvestTimer = 0.0f;
				currentPlotIndex = -1;
				Player_ChangeState(Harvesting);
			}
			else if (needsWater)
			{
				if (InputKeyboard_IsTrigger(KK_E))
				{
					const Crop& crop = CropGet(plot->cropIndex);
					Crop_Water(plot->cropIndex); 
				}
				Player_ChangeState(Watering);
			}
			else if (currentPlotIndex != -1 && !plot->occupied && CropForSeed(Inventory_GetSlotItem(Inventory_GetSelectedSlot()), selectedCropType))
			{
				Player_ChangeState(Planting);
				plantingTimer += delta_time;

				if (plantingTimer >= PLANT_TIME)
				{
					ItemType selectedSeed = Inventory_GetSlotItem(Inventory_GetSelectedSlot());
					if (Inventory_RemoveItem(selectedSeed, 1))
					{
						CropPlot_Plant(currentPlotIndex, selectedCropType);
					}
					plantingTimer = 0.0f;
					currentPlotIndex = -1;
					Player_ChangeState(Idle);
				}
			}
			else
			{
				plantingTimer = 0.0f;
				Player_ChangeState(Idle);
			}
		}
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

	if (isHarvesting)
	{
		constexpr float POPUP_SIZE = 48.0f;
		float popupX = g_Position.x + (PLAYER_WIDTH - POPUP_SIZE) * 0.5f;
		float popupY = g_Position.y - POPUP_SIZE;

		ItemType popupItem = (pendingHarvestRank == CropRank_Watered)
			? g_CropHarvestItemGold[pendingHarvestType]
			: g_CropHarvestItem[pendingHarvestType];

		Sprite_Draw(Inventory_GetIconTexture(popupItem), popupX, popupY, POPUP_SIZE, POPUP_SIZE,
			0, 0, 96, 96, 0.0f);
	}
}
