#pragma once
#ifndef UPGRADE_H
#define UPGRADE_H

#include "level.h"

void Upgrade_Initialize();
void Upgrade_Finalize();

// Call when advancing away from a cleared level, before loading the next one.
// Returns true if this level has an upgrade choice tied to it (and starts showing it) --
// caller should hold off loading the next level until Upgrade_IsChoiceActive() goes false.
bool Upgrade_TryBeginChoice(LevelType clearedLevel);

bool Upgrade_IsChoiceActive();
void Upgrade_Update(float delta_time);
void Upgrade_Draw();

bool Upgrade_IsWaterAreaUnlocked();

#endif //UPGRADE_H