#pragma once
#ifndef GAME_PLAYER_INTERACTION_H
#define GAME_PLAYER_INTERACTION_H

#include "inventory.h"

void PlayerInteraction_UpdateHarvestTimer(float delta_time);
void PlayerInteraction_HandleUse(float delta_time);

bool PlayerInteraction_IsHarvesting();
ItemType PlayerInteraction_GetHarvestPopupItem();

#endif