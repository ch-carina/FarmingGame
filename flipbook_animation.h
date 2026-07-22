/*----------------------------------------------------
 HEADER COMMENT

 Flipbook_animation header

 turns sprite animations into ones that can be reused
								   
								   Author: Carina Chao
								   Date: 2026/06/28
 ----------------------------------------------------*/
#pragma once

#ifndef FLIPBOOK_ANIMATION_H
#define FLIPBOOK_ANIMATION_H	

void FlipBookAnimation_Initialize();

int FlipBookAnimation_Create(int texture_id,
	int pattern_width, int pattern_height,
	int pattern_count_max, int pattern_col_count_max,
	float pattern_update_time, bool loop = true);

bool FlipBookAnimation_IsFinished(int animation_id); 

void FlipBookAnimation_Draw(int animation_id, float x, float y);

void FlipBookAnimation_DrawEx(int animation_id, float x,float y,float width,float height);

void FlipBookAnimation_Update(float delta_time);

void FlipBookAnimation_Finalize();

#endif // FLIPBOOK_ANIMATION_H