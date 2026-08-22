/*----------------------------------------------------
 HEADER COMMENT

 results.cpp
 results cpp.
 Will Return the results scene 
								   Author: Carina Chao
								   Date: 2026/07/09
 ----------------------------------------------------*/
#include "results.h"
#include "menu.h"
#include "sprite.h"
#include "texture.h"
#include "input_keyboard.h"
#include "scene.h"//Scene_SetNextScene();
#include "fade.h"

static int g_TextureID_ResultsBG{ -1 };
static float g_AccumalatedTime{ 0.0f };
static bool g_IsChangeScene{};

void Results_Initialize()
{
	g_TextureID_ResultsBG = Texture_Load(L"SampleGameOverScene.png");
	Fade_Start(FadeType::kIn, 1.0f, { 1.0f,1.0f,1.0f,1.0f });
}

void Results_Finalize()
{
	Texture_Release(g_TextureID_ResultsBG);
}

void Results_Update(float delta_time)
{
	if (InputKeyboard_IsTrigger(KK_ENTER))
	{
		Scene_SetNextScene(kTitle);
	}
}

void Results_Draw()
{
	Sprite_Draw(g_TextureID_ResultsBG, 0.0f, 0.0f);
}
