/*----------------------------------------------------
 HEADER COMMENT

 hue_change.h
                                   Author: Carina Chao
                                   Date: 2026/08/30
 ----------------------------------------------------*/

#pragma once
#ifndef HUE_CHANGE_H
#define HUE_CHANGE_H

void HueChange_Initialize();
void HueChange_Finalize();
void HueChange_DrawOverlay(); // tints the game world drawn so far this frame toward the current level's time-of-day color

#endif //HUE_CHANGE_H