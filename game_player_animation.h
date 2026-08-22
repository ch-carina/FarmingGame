#pragma once
#ifndef GAME_PLAYER_ANIMATION_H
#define GAME_PLAYER_ANIMATION_H

#include "game_player.h"

void PlayerAnimation_Initialize();
void PlayerAnimation_Finalize();

void PlayerAnimation_Advance(PLAYER& player, float delta_time);
void PlayerAnimation_Draw(const PLAYER& player, float posX, float posY, float width, float height);

#endif