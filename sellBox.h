#pragma once
#ifndef SELL_BOX_H
#define SELL_BOX_H

void SellBox_Initialize();
void SellBox_Finalize();
void SellBox_Update(float delta_time);
void SellBox_Draw();

bool SellBox_IsPlayerNear();
bool SellBox_TrySell();   // sells whatever's in the selected inventory slot

int SellBox_GetMoney();
bool SellBox_SpendMoney(int amount);
void SellBox_SetMoney(int amount);

#endif //SELL_BOX_H
