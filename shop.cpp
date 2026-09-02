/*----------------------------------------------------
 HEADER COMMENT

 shop.cpp

 control shop commands

								   Author: Carina Chao
								   Date: 2026/08/12
 ----------------------------------------------------*/
#include "shop.h"
#include "inventory.h"
#include "texture.h"
#include "sprite.h"
#include "collision.h"
#include "crop_plot.h"    
#include "game_player.h"
#include "sellBox.h"
#include "level.h"
#include "font.h"
#include "input_keyboard.h"
#include "config.h"
#include "draw_queue.h"
#include <cstdio>

static int g_shopInGameTextureID = TEXTURE_INVALID_ID;
static constexpr float SHOP_WIDTH = 256.0f;
static constexpr float SHOP_HEIGHT = 256.0f;
static constexpr float SHOP_MARGIN = 20.0f;

static CollisionBox g_ShopCollision;

// ---- shop menu ----
static constexpr int SHOP_ROW_COUNT = 4;
static constexpr int SHOP_COLUMN_COUNT = 2;
static constexpr int SHOP_SLOT_COUNT = SHOP_ROW_COUNT * SHOP_COLUMN_COUNT;

static constexpr ItemType g_ShopSlotItems[SHOP_SLOT_COUNT] =
{
	ItemType_CarrotSeed,
	ItemType_WheatSeed,
	ItemType_LettuceSeed,
	ItemType_CornSeed,
	ItemType_BlueberrySeed,
	ItemType_Scarecrow,
	ItemType_None,
	ItemType_None, // reserved for a future item
};

static bool g_ShopOpen = false;
static bool g_JustOpened = false;
static int g_SelectedSlot = 0;
static int g_PendingQty[SHOP_SLOT_COUNT];

static int g_PanelCapLeftID = TEXTURE_INVALID_ID;
static int g_PanelCapMidID = TEXTURE_INVALID_ID;
static int g_PanelCapRightID = TEXTURE_INVALID_ID;
static float g_PanelCapWidth = 0.0f;

static int g_SlotCapLeftID = TEXTURE_INVALID_ID;
static int g_SlotCapMidID = TEXTURE_INVALID_ID;
static int g_SlotCapRightID = TEXTURE_INVALID_ID;
static float g_SlotCapWidth = 0.0f;

static int g_CoinTextureID = TEXTURE_INVALID_ID;

static constexpr float MENU_WIDTH = 800.0f;
static constexpr float MENU_HEIGHT = 600.0f;
static constexpr float SLOT_WIDTH = 300.0f;
static constexpr float SLOT_HEIGHT = 80.0f;
static constexpr float SLOT_SPACING_X = 40.0f;
static constexpr float SLOT_SPACING_Y = 20.0f;
static constexpr float GRID_TOP_OFFSET = 100.0f;
static constexpr float ICON_SIZE = 48.0f;
static constexpr float SELECT_SCALE = 1.05f;

static constexpr float SHOP_BLOCK_HEIGHT_RATIO = 0.55f; // only the back/roof blocks movement -- the front counter stays walkable

static CollisionBox g_ShopBlockingBox;

static const char* GetItemName(ItemType item)
{
	switch (item)
	{
	case ItemType_CarrotSeed:    return "Carrot Seeds";
	case ItemType_WheatSeed:     return "Wheat Seeds";
	case ItemType_LettuceSeed:   return "Lettuce Seeds";
	case ItemType_CornSeed:      return "Corn Seeds";
	case ItemType_BlueberrySeed: return "Blueberry Seeds";
	case ItemType_Scarecrow:    return "Scarecrow";
	default:                     return "???";
	}
}

static ItemType GetCropForSeed(ItemType seed)
{
	switch (seed)
	{
	case ItemType_CarrotSeed:    return ItemType_Carrot;
	case ItemType_WheatSeed:     return ItemType_Wheat;
	case ItemType_LettuceSeed:   return ItemType_Lettuce;
	case ItemType_CornSeed:      return ItemType_Corn;
	case ItemType_BlueberrySeed: return ItemType_Blueberry;
	default:                     return ItemType_None;
	}
}

static int FindLevelPrice(ItemType item)
{
	if (item == ItemType_None) return 0;

	const LevelLayout& layout = Level_GetCurrentLayout();
	for (int i = 0; i < layout.shopItemCount; i++)
	{
		if (layout.shopItems[i].item == item)
		{
			return layout.shopItems[i].price;
		}
	}
	return 0; // not unlocked at this level
}

static void ChangeQuantity(int index, int delta)
{
	if (index < 0 || index >= SHOP_SLOT_COUNT) return;

	int qty = g_PendingQty[index] + delta;
	if (qty < 1) qty = 1;
	if (qty > 99) qty = 99;
	g_PendingQty[index] = qty;
}

static void Draw3Slice(int leftID, int midID, int rightID, float capWidth,
	float x, float y, float width, float height, const DirectX::XMFLOAT4& tint = { 1.0f, 1.0f, 1.0f, 1.0f })
{
	float midWidth = width - capWidth * 2.0f;
	if (midWidth < 0.0f) midWidth = 0.0f;

	Sprite_Draw(leftID, x, y, capWidth, height, tint);
	Sprite_Draw(midID, x + capWidth, y, midWidth, height, tint);
	Sprite_Draw(rightID, x + capWidth + midWidth, y, capWidth, height, tint);
}

void Shop_Initialize()
{
	g_shopInGameTextureID = Texture_Load(L"assets/UI/shop.png", true);
	g_PanelCapLeftID = Texture_Load(L"assets/UI/UI_L.PNG", true);
	g_PanelCapMidID = Texture_Load(L"assets/UI/UI_M.PNG", true);
	g_PanelCapRightID = Texture_Load(L"assets/UI/UI_R.PNG", true);
	g_PanelCapWidth = (float)Texture_GetWidth(g_PanelCapLeftID);

	g_SlotCapLeftID = Texture_Load(L"assets/UI/UIL_L.PNG", true);
	g_SlotCapMidID = Texture_Load(L"assets/UI/UIL_M.PNG", true);
	g_SlotCapRightID = Texture_Load(L"assets/UI/UIL_R.PNG", true);
	g_SlotCapWidth = (float)Texture_GetWidth(g_SlotCapLeftID);

	g_CoinTextureID = Texture_Load(L"assets/UI/coin.PNG", false);

	g_ShopCollision =
	{
		SHOP_WIDTH,
		SHOP_HEIGHT,
		SHOP_MARGIN,
		SCREEN_HEIGHT - SHOP_HEIGHT - SHOP_MARGIN
	};

	g_ShopBlockingBox =
	{
		SHOP_WIDTH,
		SHOP_HEIGHT * SHOP_BLOCK_HEIGHT_RATIO,
		g_ShopCollision.x,
		g_ShopCollision.y
	};

	g_ShopOpen = false;
	g_SelectedSlot = 0;
	for (int i = 0; i < SHOP_SLOT_COUNT; i++)
	{
		g_PendingQty[i] = 1;
	}
}

void Shop_Finalize()
{
	Texture_Release(g_shopInGameTextureID);
	Texture_Release(g_PanelCapLeftID);
	Texture_Release(g_PanelCapMidID);
	Texture_Release(g_PanelCapRightID);
	Texture_Release(g_SlotCapLeftID);
	Texture_Release(g_SlotCapMidID);
	Texture_Release(g_SlotCapRightID);
	Texture_Release(g_CoinTextureID);
}

void Shop_Update(float delta_time)
{
	if (!g_ShopOpen) return;
	
	if (g_JustOpened)
	{
		g_JustOpened = false;
		return; // swallow the E press that opened the menu this frame
	}

	if (InputKeyboard_IsTrigger(KK_ESCAPE))
	{
		Shop_Close();
		return;
	}

	int row = g_SelectedSlot / SHOP_COLUMN_COUNT;
	int col = g_SelectedSlot % SHOP_COLUMN_COUNT;

	if (InputKeyboard_IsTrigger(KK_S)) row = (row + 1) % SHOP_ROW_COUNT;
	else if (InputKeyboard_IsTrigger(KK_W)) row = (row - 1 + SHOP_ROW_COUNT) % SHOP_ROW_COUNT;
	else if (InputKeyboard_IsTrigger(KK_D)) col = (col + 1) % SHOP_COLUMN_COUNT;
	else if (InputKeyboard_IsTrigger(KK_A)) col = (col - 1 + SHOP_COLUMN_COUNT) % SHOP_COLUMN_COUNT;

	g_SelectedSlot = row * SHOP_COLUMN_COUNT + col;

	if (InputKeyboard_IsTrigger(KK_RIGHT)) ChangeQuantity(g_SelectedSlot, 1);
	else if (InputKeyboard_IsTrigger(KK_LEFT)) ChangeQuantity(g_SelectedSlot, -1);

	if (InputKeyboard_IsTrigger(KK_E))
	{
		Shop_TryBuy(g_SelectedSlot);
	}
}

void Shop_Draw()
{
	DrawQueue_Push(g_shopInGameTextureID, g_ShopCollision.x, g_ShopCollision.y,
		g_ShopCollision.width, g_ShopCollision.height, 0, 0, 96, 96,
		g_ShopBlockingBox.y + g_ShopBlockingBox.height);
}

void Shop_DrawMenu()
{
	if (!g_ShopOpen) return;

	float menuX = (SCREEN_WIDTH - MENU_WIDTH) * 0.5f;
	float menuY = (SCREEN_HEIGHT - MENU_HEIGHT) * 0.5f;

	Draw3Slice(g_PanelCapLeftID, g_PanelCapMidID, g_PanelCapRightID, g_PanelCapWidth,
		menuX, menuY, MENU_WIDTH, MENU_HEIGHT);

	// money
	char moneyStr[16];
	snprintf(moneyStr, sizeof(moneyStr), "%d", SellBox_GetMoney());
	Sprite_Draw(g_CoinTextureID, menuX + 25.0f, menuY + 60.0f, 32.0f, 32.0f, 0, 0, 96, 96, 0.0f);
	Font_Print(moneyStr, menuX + 65.0f, menuY + 65.0f, 3.5f);

	Font_Print("SHOP", menuX + MENU_WIDTH * 0.5f - 40.0f, menuY + 65.0f, 4.25f);

	float gridWidth = SHOP_COLUMN_COUNT * SLOT_WIDTH + (SHOP_COLUMN_COUNT - 1) * SLOT_SPACING_X;
	float gridX = menuX + (MENU_WIDTH - gridWidth) * 0.5f;
	float gridY = menuY + GRID_TOP_OFFSET;

	for (int i = 0; i < SHOP_SLOT_COUNT; i++)
	{
		int row = i / SHOP_COLUMN_COUNT;
		int col = i % SHOP_COLUMN_COUNT;

		bool unlocked = Shop_IsSlotUnlocked(i);
		bool selected = (i == g_SelectedSlot);
		float scale = selected ? SELECT_SCALE : 1.0f;

		float slotW = SLOT_WIDTH * scale;
		float slotH = SLOT_HEIGHT * scale;
		float slotX = gridX + col * (SLOT_WIDTH + SLOT_SPACING_X) - (slotW - SLOT_WIDTH) * 0.5f;
		float slotY = gridY + row * (SLOT_HEIGHT + SLOT_SPACING_Y) - (slotH - SLOT_HEIGHT) * 0.5f;

		DirectX::XMFLOAT4 tint = unlocked
			? DirectX::XMFLOAT4{ 1.0f, 1.0f, 1.0f, 1.0f }
		: DirectX::XMFLOAT4{ 0.4f, 0.4f, 0.4f, 1.0f };

		Draw3Slice(g_SlotCapLeftID, g_SlotCapMidID, g_SlotCapRightID, g_SlotCapWidth,
			slotX, slotY, slotW, slotH, tint);

		if (!unlocked) continue;

		ItemType item = g_ShopSlotItems[i];
		int iconID = Inventory_GetIconTexture(item);
		float iconY = slotY + (slotH - ICON_SIZE) * 0.5f;
		Sprite_Draw(iconID, slotX + 10.0f, iconY, ICON_SIZE, ICON_SIZE, 0, 0, 96, 96, 0.0f);

		// top row: name, vertically centered against the icon
		constexpr float NAME_SCALE = 2.75f;
		float nameX = slotX + ICON_SIZE + 20.0f;
		DirectX::XMFLOAT2 nameSize = Font_MeasureText(GetItemName(item), NAME_SCALE);
		float nameY = iconY + (ICON_SIZE - nameSize.y) * 0.5f-3.0f;
		Font_Print(GetItemName(item), nameX, nameY, NAME_SCALE);

		// bottom row: price bottom-left, sell price bottom middle, quantity bottom-right
		constexpr float BOTTOM_ROW_SCALE = 2.0f;
		float bottomRowY = slotY + slotH - Font_MeasureText("0", BOTTOM_ROW_SCALE).y - 13.0f;

		char priceStr[16];
		snprintf(priceStr, sizeof(priceStr), "Buy:%dC", Shop_GetItemPrice(i));
		Font_Print(priceStr, nameX, bottomRowY, BOTTOM_ROW_SCALE);
		
		char sellPriceStr[16];
		SellBox_GetSellPrice(GetCropForSeed(item));
		snprintf(sellPriceStr, sizeof(sellPriceStr), "Sell:%dC", SellBox_GetSellPrice(GetCropForSeed(item)));
		DirectX::XMFLOAT2 sellPriceSize = Font_MeasureText(sellPriceStr, BOTTOM_ROW_SCALE);
		Font_Print( sellPriceStr, nameX + sellPriceSize.x + 7.5f, bottomRowY, BOTTOM_ROW_SCALE);

		char qtyStr[8];
		snprintf(qtyStr, sizeof(qtyStr), "< %d >", g_PendingQty[i]);
		DirectX::XMFLOAT2 qtySize = Font_MeasureText(qtyStr, BOTTOM_ROW_SCALE);
		Font_Print(qtyStr, slotX + slotW - qtySize.x - 12.0f, bottomRowY, BOTTOM_ROW_SCALE);
	}

	Font_Print("WASD Move   Arrows Qty   E Buy   ESC Close",
		menuX + 40.0f, menuY + MENU_HEIGHT - 50.0f, 3.0f);
}

bool Shop_IsPlayerNear()
{
	return CircleVsBox(GamePlayer_GetCollisionCircle(), g_ShopCollision);
}

void Shop_Open()
{
	g_ShopOpen = true;
	g_JustOpened = true;
	g_SelectedSlot = 0;
}

void Shop_Close()
{
	g_ShopOpen = false;
}

bool Shop_IsOpen()
{
	return g_ShopOpen;
}

int Shop_GetItemCount()
{
	return SHOP_SLOT_COUNT;
}

ItemType Shop_GetItemType(int index)
{
	if (index < 0 || index >= SHOP_SLOT_COUNT) return ItemType_None;
	return g_ShopSlotItems[index];
}

int Shop_GetItemPrice(int index)
{
	if (index < 0 || index >= SHOP_SLOT_COUNT) return 0;
	return FindLevelPrice(g_ShopSlotItems[index]);
}

bool Shop_IsSlotUnlocked(int index)
{
	if (index < 0 || index >= SHOP_SLOT_COUNT) return false;
	return g_ShopSlotItems[index] != ItemType_None && FindLevelPrice(g_ShopSlotItems[index]) > 0;
}

int Shop_GetSelectedSlot()
{
	return g_SelectedSlot;
}

bool Shop_CanBuy(int index)
{
	if (!Shop_IsSlotUnlocked(index)) return false;

	ItemType item = g_ShopSlotItems[index];
	int qty = g_PendingQty[index];
	int totalCost = Shop_GetItemPrice(index) * qty;

	if (totalCost <= 0 || totalCost > SellBox_GetMoney()) return false;
	if (!Inventory_CanAddItem(item, qty)) return false;

	return true;
}

bool Shop_TryBuy(int index)
{
	if (!Shop_CanBuy(index)) return false;

	ItemType item = g_ShopSlotItems[index];
	int qty = g_PendingQty[index];
	int cost = Shop_GetItemPrice(index) * qty;

	if (!SellBox_SpendMoney(cost)) return false;

	Inventory_AddItem(item, qty);
	g_PendingQty[index] = 1;
	return true;
}

bool Shop_IsBlocking(const CollisionCircle& circle)
{
	return CircleVsBox(circle, g_ShopBlockingBox);
}
