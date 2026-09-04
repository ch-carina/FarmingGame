#include "sellBox.h"
#include "inventory.h"
#include "crop_plot.h"
#include "game_player.h"
#include "collision.h"
#include "config.h"
#include "texture.h"
#include "sprite.h"
#include "draw_queue.h"

static constexpr float SELL_BOX_WIDTH = 192.0f;
static constexpr float SELL_BOX_HEIGHT = 96.0f;
static constexpr float SELL_BOX_MARGIN = 20.0f; 

static constexpr float COIN_POPUP_DURATION = 0.5f;
static constexpr float COIN_RISE_DISTANCE = 40.0f;
static constexpr float COIN_SIZE = 42.0f; 

static int g_SellBoxTextureID = TEXTURE_INVALID_ID;
static int g_CoinTextureID = TEXTURE_INVALID_ID;
static CollisionBox g_SellBoxCollision;

static int g_Money = 0;
static bool g_ShowCoinPopup = false;
static float g_CoinPopupTimer = 0.0f;
static int g_PendingCoinAmount = 0;

static constexpr float SELL_BOX_BLOCK_HEIGHT_RATIO = 0.6f;

static CollisionBox g_SellBoxBlockingBox;
static constexpr float SELL_BOX_WALKABLE_FRONT = 50.0f; 
static int GetSellPrice(ItemType item)
{
	switch (item)
	{
	case ItemType_Carrot:
		return 40; 
	case ItemType_CarrotGold: 
		return 45;
	case ItemType_Wheat:
		return 20;  
	case ItemType_WheatGold:
		return 25;
	case ItemType_Lettuce:
		return 30;
	case ItemType_LettuceGold:
		return 35;
	case ItemType_Corn:
		return 30;
	case ItemType_CornGold:
		return 35;
	case ItemType_Blueberry:
		return 35;
	case ItemType_BlueberryGold:
		return 40;
	default:
		return 0;
	}
}

void SellBox_Initialize()
{
	g_SellBoxTextureID = Texture_Load(L"assets/GroundTiles/SellBox.PNG", true);
	g_CoinTextureID = Texture_Load(L"assets/UI/Coin.PNG", true);
	
	g_SellBoxCollision =
	{
		SELL_BOX_WIDTH,
		SELL_BOX_HEIGHT,
		SCREEN_WIDTH - SELL_BOX_WIDTH - SELL_BOX_MARGIN,
		SCREEN_HEIGHT - SELL_BOX_HEIGHT - SELL_BOX_MARGIN
	};

	g_SellBoxBlockingBox =
	{
		SELL_BOX_WIDTH,
		SELL_BOX_HEIGHT - SELL_BOX_WALKABLE_FRONT,
		g_SellBoxCollision.x,
		g_SellBoxCollision.y + SELL_BOX_WALKABLE_FRONT
	};

	g_Money = 100;
	g_ShowCoinPopup = false;
	g_CoinPopupTimer = 0.0f;
}

void SellBox_Finalize()
{
	Texture_Release(g_SellBoxTextureID);
	Texture_Release(g_CoinTextureID);
}

bool SellBox_IsPlayerNear()
{
	return CircleVsBox(GamePlayer_GetCollisionCircle(), g_SellBoxCollision);
}

bool SellBox_TrySell()
{
	if (g_ShowCoinPopup) return false; //Finish one sale before starting the next one 

	int slot = Inventory_GetSelectedSlot();
	ItemType item = Inventory_GetSlotItem(slot);
	int price = GetSellPrice(item);

	if (price <= 0) return false; //Nothing to sell or item not sellable 

	int count = Inventory_GetSlotCount(slot);
	Inventory_RemoveItem(item, count);

	g_PendingCoinAmount = price * count;
	g_ShowCoinPopup = true;
	g_CoinPopupTimer = 0.0f;

	return true;
}

void SellBox_Update(float delta_time)
{
	if (!g_ShowCoinPopup) return;

	g_CoinPopupTimer += delta_time;
	if (g_CoinPopupTimer >= COIN_POPUP_DURATION)
	{
		g_Money += g_PendingCoinAmount;
		g_ShowCoinPopup = false;
	}
}

void SellBox_Draw()
{
	DrawQueue_Push(g_SellBoxTextureID, g_SellBoxCollision.x, g_SellBoxCollision.y,
		g_SellBoxCollision.width, g_SellBoxCollision.height, 0, 0, (int)SELL_BOX_WIDTH, (int)SELL_BOX_HEIGHT,
		g_SellBoxBlockingBox.y + g_SellBoxBlockingBox.height);
}

int SellBox_GetMoney()
{
	return g_Money;
}

int SellBox_GetSellPrice(ItemType item)
{
	return GetSellPrice(item);
}

bool SellBox_SpendMoney(int amount)
{
	if (amount <= 0 || amount > g_Money)
	{
		return false;
	}

	g_Money -= amount;
	return true;
}

void SellBox_SetMoney(int amount)
{
	g_Money = amount;
	g_ShowCoinPopup = false;
	g_PendingCoinAmount = 0;
}

void SellBox_DrawPopup()
{
	if (g_ShowCoinPopup)
	{
		float t = g_CoinPopupTimer / COIN_POPUP_DURATION;
		float coinX = g_SellBoxCollision.x + (g_SellBoxCollision.width - COIN_SIZE) * 0.5f;
		float coinY = g_SellBoxCollision.y - COIN_RISE_DISTANCE * t;
		float alpha = 1.0f - t;

		Sprite_Draw(g_CoinTextureID, coinX, coinY, COIN_SIZE, COIN_SIZE, 0, 0, 96, 96, 0.0f, { 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, alpha });
	}
}

bool SellBox_IsBlocking(const CollisionCircle& circle)
{
	return CircleVsBox(circle, g_SellBoxBlockingBox);
}

