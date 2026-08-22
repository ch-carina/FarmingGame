/*----------------------------------------------------
 HEADER COMMENT

 menu.cpp
 Menu cpp. 
 Idea is to make this base for all possible menus
								   Author: Carina Chao
								   Date: 2026/07/09
 ----------------------------------------------------*/
#include "menu.h"
#include "sprite.h"
#include "texture.h"
#include "input_keyboard.h"
#include "scene.h"//Scene_SetNextScene();
#include "config.h"
#include "fade.h"
#include "font.h"

static int g_TextureID_MenuTitle{-1};
static float g_AccumalatedTime{ 0.0f };
static bool g_IsChangeScene{};

void Menu_Initialize()
{
	g_TextureID_MenuTitle = Texture_Load(L"assets/UI/Title.png");
	Font_Initialize();
	g_AccumalatedTime = 0.0f;
	g_IsChangeScene = false; 

}

void Menu_Finalize()
{
	Texture_Release(g_TextureID_MenuTitle);
	Font_Finalize();
}

void Menu_Update(float delta_time)
{
	g_AccumalatedTime += delta_time; 

	if (!g_IsChangeScene) {
		if (InputKeyboard_IsTrigger(KK_ENTER))
		{
			Fade_Start(FadeType::kOut, 1.0f, { 0.0f,0.0f,0.0f,0.0f });
			g_IsChangeScene = true;
		}
	}
	else 
	{
		if (g_IsChangeScene && Fade_IsFinished())
		{
			Scene_SetNextScene(kGame);
		}
	}
}

void Menu_Draw()
{
	float w = (float)Texture_GetWidth(g_TextureID_MenuTitle);
	float h = (float)Texture_GetHeight(g_TextureID_MenuTitle);
	float titleY = SCREEN_HEIGHT * 0.5f - h * 0.5f;
	Sprite_Draw(g_TextureID_MenuTitle, SCREEN_WIDTH * 0.5f - w * 0.5f, titleY);
	
	float textY = titleY + h + 50.0f; // 50px below the title's bottom edge
	DirectX::XMFLOAT2 textSize = Font_MeasureText("Press ENTER to play", 3.0f);
	float textX = SCREEN_WIDTH * 0.5f - textSize.x * 0.5f;
	Font_Print("Press ENTER to play", textX, textY, 3.0f);
}
