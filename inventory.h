/*----------------------------------------------------
 HEADER COMMENT

 inventory.h
 header for player seed inventory
                                   Author: Carina Chao
                                   Date: 2026/08/04
 ----------------------------------------------------*/

#pragma once
#ifndef INVENTORY_H
#define INVENTORY_H
#include "crop.h"

static constexpr int INVENTORY_SLOT_COUNT = 6;

enum ItemType
{
    ItemType_None,
    ItemType_CarrotSeed,
    ItemType_Carrot,
    ItemType_CarrotGold,
    ItemType_WaterPail,
    ItemType_WheatSeed, 
    ItemType_Wheat,
    ItemType_LettuceSeed, 
    ItemType_Lettuce,
    ItemType_CornSeed, 
    ItemType_Corn,
    ItemType_BlueberrySeed, 
    ItemType_Blueberry,
    ItemType_Scarecrow,
    ItemType_MAX
};

struct inventorySlot
{
	ItemType item;
	int count;
};

struct InventorySnapshot
{
    inventorySlot slot[INVENTORY_SLOT_COUNT];
};

InventorySnapshot Inventory_GetSnapshot();
void Inventory_SetSnapshot(const InventorySnapshot& snapshot);

void Inventory_Initialize();
void Inventory_Finalize();
void Inventory_Draw();
void Inventory_Update(float delta_time);

bool Inventory_AddItem(ItemType item, int amount);
bool Inventory_RemoveItem(ItemType item, int amount = 1);
bool Inventory_HasItem(ItemType item);
int Inventory_GetIconTexture(ItemType item);
int Inventory_GetItemTexture(ItemType item);
int Inventory_GetItemCount(ItemType item);
int Inventory_GetSelectedSlot();
ItemType Inventory_GetSlotItem(int index);
int Inventory_GetSlotCount(int index);
bool Inventory_CanAddItem(ItemType item, int amount);


#endif //INVENTORY_H
