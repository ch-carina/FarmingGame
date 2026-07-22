
/*----------------------------------------------------
 HEADER COMMENT

 Flipbook_animation .cpp

 controls animations for sprites 
                                   Author: Carina Chao
                                   Date: 2026/06/28
 ----------------------------------------------------*/
#include "flipbook_animation.h"
#include "sprite.h"

struct FlipBookAnimation
{
	int texture_id;
	int pattern_width;
	int pattern_height;

	int pattern_count_max; //how many patterns are in the texture
	int pattern_col_count_max; //how long the pattern is in a row
	float pattern_update_time; //time to update pattern

	int pattern_index; //current pattern index (can be float for smooth animation)
	float pattern_accumulator; //accumulates time for pattern switching
	float animation_elapsed_time; //time elapsed since the last update
	bool toLoop; //whether the animation should loop or not
};

static constexpr int ANIMATION_MAX{ 128 };
static FlipBookAnimation g_Animations[ANIMATION_MAX]{};


void FlipBookAnimation_Initialize()
{

	for (FlipBookAnimation& a : g_Animations)
	{
		a.pattern_count_max = 0;
	}


}

int FlipBookAnimation_Create(int texture_id,
	int pattern_width, int pattern_height,
	int pattern_count_max, int pattern_col_count_max,
	float pattern_update_time, bool loop)
{
	for (int i = 0; i < ANIMATION_MAX; i++)
	{
		if (g_Animations[i].pattern_count_max != 0) continue;//if not used
		
			g_Animations[i].texture_id = texture_id;
			g_Animations[i].pattern_width = pattern_width;
			g_Animations[i].pattern_height = pattern_height;
			g_Animations[i].pattern_count_max = pattern_count_max;
			g_Animations[i].pattern_col_count_max = pattern_col_count_max;
			g_Animations[i].pattern_update_time = pattern_update_time;
			g_Animations[i].pattern_index = 0;
			g_Animations[i].pattern_accumulator = 0.0f;
			g_Animations[i].animation_elapsed_time = 0.0f;
			g_Animations[i].toLoop = loop;
			
			return i; //return the animation ID (index)
		
	}
	return -1;
}

bool FlipBookAnimation_IsFinished(int animation_id)
{
	return g_Animations[animation_id].pattern_count_max*g_Animations[animation_id].pattern_update_time 
		<=g_Animations[animation_id].animation_elapsed_time;
}

void FlipBookAnimation_Draw(int animation_id, float x, float y) // could add scale and roation in here if you wanted "float scale, float rotation"
{

	if (animation_id < 0 || animation_id >= ANIMATION_MAX) return;
	if (g_Animations[animation_id].pattern_count_max == 0) return;

	FlipBookAnimation* p = &g_Animations[animation_id];

	int count = p->pattern_index % p->pattern_count_max;

	int pattern_x = count % p->pattern_col_count_max;
	int pattern_y = count / p->pattern_col_count_max;

	Sprite_Draw(
		p->texture_id,
		x, y,
		(float)p->pattern_width,
		(float)p->pattern_height,
		p->pattern_width * pattern_x,
		p->pattern_height * pattern_y,
		p->pattern_width,
		p->pattern_height,
		0.0f
	);

}

void FlipBookAnimation_DrawEx(int animation_id, float x, float y, float width, float height)
{
	
if (animation_id < 0 || animation_id >= ANIMATION_MAX) return;
    if (g_Animations[animation_id].pattern_count_max == 0) return;

    FlipBookAnimation* p = &g_Animations[animation_id];

    int count = p->pattern_index % p->pattern_count_max;

    int pattern_x = count % p->pattern_col_count_max;
    int pattern_y = count / p->pattern_col_count_max;

    Sprite_Draw(
        p->texture_id,
        x, y,
        width, height,
        p->pattern_width * pattern_x,
        p->pattern_height * pattern_y,
        p->pattern_width,
        p->pattern_height
    );

}

void FlipBookAnimation_Update(float delta_time)
{

	for (int i = 0; i < ANIMATION_MAX; i++)
	{
		if (g_Animations[i].pattern_count_max == 0) continue;

		g_Animations[i].animation_elapsed_time += delta_time;

		g_Animations[i].pattern_accumulator += delta_time;

		if (g_Animations[i].pattern_accumulator >= g_Animations[i].pattern_update_time)
		{
			g_Animations[i].pattern_accumulator = 0.0f;

			g_Animations[i].pattern_index++;

			if (!g_Animations[i].toLoop && g_Animations[i].pattern_index >= g_Animations[i].pattern_count_max)
			{
				g_Animations[i].pattern_index = g_Animations[i].pattern_count_max - 1; //stop at the last pattern
			}

			if (g_Animations[i].pattern_index >= g_Animations[i].pattern_count_max)
			{
				g_Animations[i].pattern_index = 0;
			}
		}
	}


}

void FlipBookAnimation_Finalize()
{
	//nothing here right now because there is nothing to clear 
}
