#ifndef FENCE_H
#define FENCE_H	
#include "crop_plot.h"
#include "collision.h"

void Fence_Initialize();
void Fence_Finalize();
void Fence_Draw();

void Fence_LoadRegions(const PlotRegion regions[], int regionCount);

int Fence_GetPlayerSlot(); // -1 if the player isn't standing on an empty perimeter slot
bool Fence_IsBuilt(int index);
void Fence_Build(int index);

bool Fence_IsBlocked(const CollisionCircle& circle);
void Fence_RegisterHit(int index); // breaks the segment after enough hits

int Fence_GetBlockingSlot(const CollisionCircle& circle, int ignoreIndex = -1);

bool Fence_GetSlotPosition(int index, float& outX, float& outY);

#endif FENCE_H