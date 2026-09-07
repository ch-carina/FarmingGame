#pragma once
#ifndef GAME_PLAYER_INTERACTION_H
#define GAME_PLAYER_INTERACTION_H

#include "inventory.h"

void PlayerInteraction_Initialize();
void PlayerInteraction_Finalize();

void PlayerInteraction_UpdateHarvestTimer(float delta_time);
void PlayerInteraction_HandleUse(float delta_time);

bool PlayerInteraction_IsHarvesting();
ItemType PlayerInteraction_GetHarvestPopupItem();

bool PlayerInteraction_IsFilling();
float PlayerInteraction_GetFillProgress();

bool PlayerInteraction_GetHoverTile(float& outX, float& outY);

void PlayerInteraction_ApplyInteractionSpeedBoost();

void PlayerInteraction_ResetUpgrades();

void PlayerInteraction_CheckEarlyRelease();

#endif