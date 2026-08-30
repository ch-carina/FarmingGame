/*----------------------------------------------------
 HEADER COMMENT

 shop.h

 control shop commands 

                                   Author: Carina Chao
                                   Date: 2026/08/12 
 ----------------------------------------------------*/
#pragma once
#ifndef SHOP_H
#define SHOP_H

#include "inventory.h"
#include "collision.h"

void Shop_Initialize(); 
void Shop_Finalize(); 
void Shop_Update(float delta_time);
void Shop_Draw();
void Shop_DrawMenu();

bool Shop_IsPlayerNear();

void Shop_Open();
void Shop_Close(); 
bool Shop_IsOpen(); 

int Shop_GetItemCount();          // how many items are for sale at the CURRENT level
ItemType Shop_GetItemType(int index);
int Shop_GetItemPrice(int index);

bool Shop_IsSlotUnlocked(int index);
int Shop_GetSelectedSlot();

bool Shop_CanBuy(int index);      // affordable AND fits in inventory
bool Shop_TryBuy(int index);      // false if Shop_CanBuy would be false; otherwise spends money + adds item

bool Shop_IsBlocking(const CollisionCircle& circle);
#endif //SHOP_H 