/*---------------------------------s-------------------
 HEADER COMMENT

 inventory.cpp

 Tracks how many items slots and draws inventory bar

                                   Author: Carina Chao
                                   Date: 2026/08/04
 ----------------------------------------------------*/

#include "inventory.h"
#include "texture.h"
#include "sprite.h"
#include "config.h"
#include "input_keyboard.h"
#include "font.h"
#include <cstdio>

static inventorySlot g_Slots[INVENTORY_SLOT_COUNT];
static int g_ItemIconTextureID[ItemType_MAX];
static constexpr float ICON_DISPLAY_SIZE = 64.0f; //item size
static int g_SelectedSlot = 0; //currently selected slot

static int g_SlotTextureID = TEXTURE_INVALID_ID;
static float g_SlotWidth = 0.0f;
static float g_SlotHeight = 0.0f;

static constexpr float COUNT_FONT_SCALE = 4.0f;
static constexpr float COUNT_MARGIN = 4.0f;
static constexpr float SLOT_SPACING = 8.0f;
static constexpr float BOTTOM_MARGIN = 20.0f;
static constexpr float SLOT_SELECT_SCALE = 1.2f;

static int GetMaxStack(ItemType item)
{
	switch (item)
	{
	case ItemType_WaterPail:
		return 1;
	default:
		return 10;
	}
}

InventorySnapshot Inventory_GetSnapshot()
{
	InventorySnapshot snapshot;
	for (int i = 0; i < INVENTORY_SLOT_COUNT; i++)
	{
		snapshot.slot[i] = g_Slots[i];
	}
	return snapshot;
}


void Inventory_SetSnapshot(const InventorySnapshot& snapshot)
{
	for (int i = 0; i < INVENTORY_SLOT_COUNT; i++)
	{
		g_Slots[i] = snapshot.slot[i];
	}
}

void Inventory_Initialize()
{
    for (int i = 0; i < INVENTORY_SLOT_COUNT; i++)
    {
        g_Slots[i].item = ItemType_None;
        g_Slots[i].count = 0;
    }

	g_SlotTextureID = Texture_Load(L"assets/UI/inventoryBox.PNG", true);
	g_SlotWidth = (float)Texture_GetWidth(g_SlotTextureID);
	g_SlotHeight = (float)Texture_GetHeight(g_SlotTextureID);

	g_ItemIconTextureID[ItemType_None] = TEXTURE_INVALID_ID;
	g_ItemIconTextureID[ItemType_CarrotSeed] = Texture_Load(L"assets/UI/carrotSeeds.PNG", false);
	g_ItemIconTextureID[ItemType_Carrot] = Texture_Load(L"assets/Crops/Carrot/Carrot_Crop.PNG", false);
	g_ItemIconTextureID[ItemType_WaterPail] = Texture_Load(L"assets/UI/waterPail.PNG", false);
	g_ItemIconTextureID[ItemType_WheatSeed] = Texture_Load(L"assets/UI/Wheat_Seeds.PNG", false); 
	g_ItemIconTextureID[ItemType_Wheat] = Texture_Load(L"assets/Crops/Wheat/Wheat_Crop.PNG", false);
	g_ItemIconTextureID[ItemType_LettuceSeed] = Texture_Load(L"assets/UI/Lettuce_Seeds.PNG", false);
	g_ItemIconTextureID[ItemType_Lettuce] = Texture_Load(L"assets/Crops/Wheat_Crop.PNG", false); // need to update with right image
	g_ItemIconTextureID[ItemType_CornSeed] = Texture_Load(L"assets/UI/Corn_Seeds.PNG", false);
	g_ItemIconTextureID[ItemType_Corn] = Texture_Load(L"assets/Crops/Corn/Corn.PNG", false);
	g_ItemIconTextureID[ItemType_BlueberrySeed] = Texture_Load(L"assets/UI/Blueberry_Seeds.PNG", false);
	g_ItemIconTextureID[ItemType_Carrot] = Texture_Load(L"assets/Crops/Carrot/Carrot_Crop.PNG", false);
	g_ItemIconTextureID[ItemType_CarrotGold] = Texture_Load(L"assets/Crops/Carrot/Carrot_Crop.PNG", false);
	g_ItemIconTextureID[ItemType_Blueberry] = Texture_Load(L"assets/Crops/Blueberry.PNG", false);
}

void Inventory_Finalize()
{
	Texture_Release(g_SlotTextureID);
	
	for (int i = 0; i < ItemType_MAX; i++)
	{
		Texture_Release(g_ItemIconTextureID[i]);
	}
}

void Inventory_Draw()
{
	float totalWidth = (INVENTORY_SLOT_COUNT - 1) * g_SlotWidth + g_SlotWidth *SLOT_SELECT_SCALE + (INVENTORY_SLOT_COUNT - 1) * SLOT_SPACING;
	float startX = (SCREEN_WIDTH - totalWidth) * 0.5f;
	float baseY = SCREEN_HEIGHT - g_SlotHeight - BOTTOM_MARGIN;

	float cursorX = startX;

	for (int i = 0; i < INVENTORY_SLOT_COUNT; i++)
	{
		float scale = (i == g_SelectedSlot) ? SLOT_SELECT_SCALE : 1.0f;
		float drawW = g_SlotWidth * scale;
		float drawH = g_SlotHeight * scale;
		float drawX = cursorX;
		float drawY = baseY - (drawH - g_SlotHeight) * 0.5f;

		Sprite_Draw(g_SlotTextureID, drawX, drawY, drawW, drawH, 0, 0, (int)g_SlotWidth, (int)g_SlotHeight, 0.0f);

		if (g_Slots[i].item != ItemType_None)
		{
			int iconID = g_ItemIconTextureID[g_Slots[i].item];
			float iconSize = ICON_DISPLAY_SIZE * scale;
			float iconOffset = (drawW - iconSize) * 0.5f;

			Sprite_Draw(iconID, drawX + iconOffset, drawY + iconOffset, iconSize, iconSize, 0, 0, (int)g_SlotWidth, (int)g_SlotHeight, 0.0f);

			char countStr[8];
			snprintf(countStr, sizeof(countStr), "%d", g_Slots[i].count);

			DirectX::XMFLOAT2 textSize = Font_MeasureText(countStr, COUNT_FONT_SCALE);
			float textX = drawX + drawW - textSize.x - COUNT_MARGIN;
			float textY = drawY + drawH - textSize.y - COUNT_MARGIN;

			Font_Print(countStr, textX, textY, COUNT_FONT_SCALE);
		}

		cursorX += drawW + SLOT_SPACING;
	}
}

void Inventory_Update(float delta_time)
{
	//Keyboard input for selecting inventory slots
	if (InputKeyboard_IsTrigger(KK_D1)) g_SelectedSlot = 0;
	else if (InputKeyboard_IsTrigger(KK_D2)) g_SelectedSlot = 1;
	else if (InputKeyboard_IsTrigger(KK_D3)) g_SelectedSlot = 2;
	else if (InputKeyboard_IsTrigger(KK_D4)) g_SelectedSlot = 3;
	else if (InputKeyboard_IsTrigger(KK_D5)) g_SelectedSlot = 4;
	else if (InputKeyboard_IsTrigger(KK_D6)) g_SelectedSlot = 5;
}

bool Inventory_AddItem(ItemType item, int amount)
{
    if (item < 0 || item >= ItemType_MAX || amount <= 0)
    {
        return false;
    }

	int maxStack = GetMaxStack(item);

	//fill any existing stacks first
	for (int i = 0; i < INVENTORY_SLOT_COUNT; i++)
	{
		if (g_Slots[i].item != item)
		{
			continue;
		}

		int space = maxStack - g_Slots[i].count;
		int add = (amount < space) ? amount : space;

		g_Slots[i].count += add;
		amount -= add;
	}
    
	//spill remiander into empty slots 
	for (int i = 0; i < INVENTORY_SLOT_COUNT && amount>0; i++)
	{
		if (g_Slots[i].item != ItemType_None)
		{
			continue;
		}
		int add = (amount < maxStack) ? amount : maxStack;
		g_Slots[i].item = item;
		g_Slots[i].count = add;
		amount -= add;
	}

	return amount <= 0; //false means inventory was full and some amount didnt fit
}

bool Inventory_RemoveItem(ItemType item, int amount)
{
	if (item == ItemType_None || amount <= 0)
	{
		return false;
	}

	if (Inventory_GetItemCount(item) < amount)
	{
		return false;
	}

	for (int i = 0; i < INVENTORY_SLOT_COUNT && amount > 0; i++)
	{
		if (g_Slots[i].item != item)
		{
			continue;
		}

		int take = (amount < g_Slots[i].count) ? amount : g_Slots[i].count;

		g_Slots[i].count -= take;
		amount -= take;

		if (g_Slots[i].count <= 0)
		{
			g_Slots[i].item = ItemType_None;
			g_Slots[i].count = 0;
		}
	}

	return true;
}

int Inventory_GetItemTexture(ItemType item)
{
	return 0;
}


int Inventory_GetItemCount(ItemType item)
{
	int total = 0;
    
	for (int i = 0; i < INVENTORY_SLOT_COUNT; i++)
	{
		if (g_Slots[i].item == item)
		{
			total += g_Slots[i].count;
		}
	}

	return total;
}

int Inventory_GetSelectedSlot()
{
	return g_SelectedSlot;
}

ItemType Inventory_GetSlotItem(int index)
{
	if (index < 0 || index >= INVENTORY_SLOT_COUNT)
	{
		return ItemType_None;
	}
	return g_Slots[index].item;
}

int Inventory_GetSlotCount(int index)
{
	if (index < 0 || index >= INVENTORY_SLOT_COUNT)
	{
		return 0;
	}
	return g_Slots[index].count;
}

bool Inventory_CanAddItem(ItemType item, int amount)
{
	if (item < 0 || item >= ItemType_MAX || amount <= 0)
	{
		return false;
	}

	int maxStack = GetMaxStack(item);
	int remaining = amount;

	for (int i = 0; i < INVENTORY_SLOT_COUNT && remaining > 0; i++)
	{
		if (g_Slots[i].item == item)
		{
			remaining -= (maxStack - g_Slots[i].count);
		}
		else if (g_Slots[i].item == ItemType_None)
		{
			remaining -= maxStack;
		}
	}
	return remaining <= 0;
}

bool Inventory_HasItem(ItemType item)
{
    return Inventory_GetItemCount(item) > 0;
}

int Inventory_GetIconTexture(ItemType item)
{
	if (item < 0 || item >= ItemType_MAX)
	{
		return -1; // Invalid item type
	}
	return g_ItemIconTextureID[item];
}