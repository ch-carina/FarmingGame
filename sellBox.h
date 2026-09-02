#pragma once
#ifndef SELL_BOX_H
#define SELL_BOX_H
#include "collision.h"
#include "inventory.h"

void SellBox_Initialize();
void SellBox_Finalize();
void SellBox_Update(float delta_time);
void SellBox_Draw();

bool SellBox_IsPlayerNear();
bool SellBox_TrySell();   // sells whatever's in the selected inventory slot

int SellBox_GetMoney();
int SellBox_GetSellPrice(ItemType item);
bool SellBox_SpendMoney(int amount);
void SellBox_SetMoney(int amount);

void SellBox_DrawPopup();

bool SellBox_IsBlocking(const CollisionCircle& circle);
#endif //SELL_BOX_H
