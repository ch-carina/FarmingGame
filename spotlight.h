/*----------------------------------------------------
 HEADER COMMENT

 spotlight.cpp

								   Author: Carina Chao
								   Date: 2026/08/30
 ----------------------------------------------------*/
#pragma once
#ifndef SPOTLIGHT_H
#define SPOTLIGHT_H

void Spotlight_Initialize();
void Spotlight_Finalize();
void Spotlight_Draw(); // darkens the screen except a circle around the player -- only active on Level 3

#endif //SPOTLIGHT_H