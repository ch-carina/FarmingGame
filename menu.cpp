/*----------------------------------------------------
 HEADER COMMENT

 menu.cpp
 Menu cpp.

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
#include "ground.h"
#include <Windows.h>

static int g_TextureID_MenuTitle{ -1 };
static float g_AccumalatedTime{ 0.0f };
static bool g_IsChangeScene{};

enum MenuState
{
	kMenuMain,
	kMenuInstructions,
	kMenuCredits
};
static MenuState g_MenuState{ kMenuMain };

static constexpr int kButtonCount = 4;
static const char* g_ButtonLabels[kButtonCount] =
{
	"Play Game",
	"Instructions",
	"Credits",
	"Quit",
};
static int g_SelectedButton{ 0 };

static int g_SlotCapLeftID = TEXTURE_INVALID_ID;
static int g_SlotCapMidID = TEXTURE_INVALID_ID;
static int g_SlotCapRightID = TEXTURE_INVALID_ID;
static float g_SlotCapWidth = 0.0f;

static constexpr float TITLE_TOP_MARGIN = 30.0f;
static constexpr float TITLE_BUTTON_GAP = 30.0f;

static constexpr float BUTTON_WIDTH = 260.0f;
static constexpr float BUTTON_HEIGHT = 70.0f;
static constexpr float BUTTON_SPACING_Y = 20.0f;
static constexpr float BUTTON_TEXT_SCALE = 3.0f;
static constexpr float SELECT_SCALE = 1.05f; 

static void Draw3Slice(int leftID, int midID, int rightID, float capWidth,
	float x, float y, float width, float height, const DirectX::XMFLOAT4& tint = { 1.0f, 1.0f, 1.0f, 1.0f })
{
	float midWidth = width - capWidth * 2.0f;
	if (midWidth < 0.0f) midWidth = 0.0f;

	Sprite_Draw(leftID, x, y, capWidth, height, tint);
	Sprite_Draw(midID, x + capWidth, y, midWidth, height, tint);
	Sprite_Draw(rightID, x + capWidth + midWidth, y, capWidth, height, tint);
}

static float GetButtonBlockStartY()
{
	float titleH = (float)Texture_GetHeight(g_TextureID_MenuTitle);
	return TITLE_TOP_MARGIN + titleH + TITLE_BUTTON_GAP;
}

static void ActivateButton(int index)
{
	switch (index)
	{
	case 0: // Play Game
		if (!g_IsChangeScene)
		{
			Fade_Start(FadeType::kOut, 1.0f, { 0.0f,0.0f,0.0f,0.0f });
			g_IsChangeScene = true;
		}
		break;
	case 1: // Instructions
		g_MenuState = kMenuInstructions;
		break;
	case 2: // Credits
		g_MenuState = kMenuCredits;
		break;
	case 3: // Quit
		PostMessage(GetActiveWindow(), WM_CLOSE, 0, 0);
		break;
	}
}

void Menu_Initialize()
{
	g_TextureID_MenuTitle = Texture_Load(L"assets/UI/Title.png");

	g_SlotCapLeftID = Texture_Load(L"assets/UI/UIL_L.PNG", true);
	g_SlotCapMidID = Texture_Load(L"assets/UI/UIL_M.PNG", true);
	g_SlotCapRightID = Texture_Load(L"assets/UI/UIL_R.PNG", true);
	g_SlotCapWidth = (float)Texture_GetWidth(g_SlotCapLeftID);

	Ground_Initialize();
	Ground_LoadLayout(nullptr, 0); // no plot regions in the menu, so every tile gets a random ground tile

	Font_Initialize();
	g_AccumalatedTime = 0.0f;
	g_IsChangeScene = false;
	g_MenuState = kMenuMain;
	g_SelectedButton = 0;
}

void Menu_Finalize()
{
	Texture_Release(g_TextureID_MenuTitle);
	Texture_Release(g_SlotCapLeftID);
	Texture_Release(g_SlotCapMidID);
	Texture_Release(g_SlotCapRightID);
	Font_Finalize();
	Ground_Finalize();
}

void Menu_Update(float delta_time)
{
	g_AccumalatedTime += delta_time;

	if (g_IsChangeScene)
	{
		if (Fade_IsFinished())
		{
			Scene_SetNextScene(kGame);
		}
		return;
	}

	if (g_MenuState != kMenuMain)
	{
		// Instructions / Credits screen: ENTER or ESC takes you back
		if (InputKeyboard_IsTrigger(KK_ENTER) || InputKeyboard_IsTrigger(KK_ESCAPE))
		{
			g_MenuState = kMenuMain;
		}
		return;
	}

	if (InputKeyboard_IsTrigger(KK_S))
	{
		g_SelectedButton = (g_SelectedButton + 1) % kButtonCount;
	}
	if (InputKeyboard_IsTrigger(KK_W))
	{
		g_SelectedButton = (g_SelectedButton - 1 + kButtonCount) % kButtonCount;
	}

	if (InputKeyboard_IsTrigger(KK_ENTER))
	{
		ActivateButton(g_SelectedButton);
	}
}

void Menu_Draw()
{
	Ground_Draw();
	float titleW = (float)Texture_GetWidth(g_TextureID_MenuTitle);
	float titleH = (float)Texture_GetHeight(g_TextureID_MenuTitle);
	Sprite_Draw(g_TextureID_MenuTitle, SCREEN_WIDTH * 0.5f - titleW * 0.5f, TITLE_TOP_MARGIN);

	if (g_MenuState == kMenuMain)
	{
		float startY = GetButtonBlockStartY();

		for (int i = 0; i < kButtonCount; ++i)
		{
			bool selected = (i == g_SelectedButton);
			float scale = selected ? SELECT_SCALE : 1.0f;

			float baseX = SCREEN_WIDTH * 0.5f - BUTTON_WIDTH * 0.5f;
			float baseY = startY + i * (BUTTON_HEIGHT + BUTTON_SPACING_Y);

			float panelW = BUTTON_WIDTH * scale;
			float panelH = BUTTON_HEIGHT * scale;
			float panelX = baseX - (panelW - BUTTON_WIDTH) * 0.5f;
			float panelY = baseY - (panelH - BUTTON_HEIGHT) * 0.5f;

			Draw3Slice(g_SlotCapLeftID, g_SlotCapMidID, g_SlotCapRightID, g_SlotCapWidth,
				panelX, panelY, panelW, panelH);

			float textScale = BUTTON_TEXT_SCALE * scale;
			DirectX::XMFLOAT2 textSize = Font_MeasureText(g_ButtonLabels[i], textScale);
			float textX = panelX + (panelW - textSize.x) * 0.5f;
			float textY = panelY + (panelH - textSize.y) * 0.5f;
			Font_Print(g_ButtonLabels[i], textX, textY, textScale);
		}
	}
	else if (g_MenuState == kMenuInstructions)
	{
		const char* text = "Use WASD to move, and click to plant/harvest.\n\nPress ENTER to return.";
		DirectX::XMFLOAT2 textSize = Font_MeasureText(text, 2.5f);
		Font_Print(text, SCREEN_WIDTH * 0.5f - textSize.x * 0.5f, SCREEN_HEIGHT * 0.5f - textSize.y * 0.5f, 2.5f);
	}
	else if (g_MenuState == kMenuCredits)
	{
		const char* text = "Made by Carina Chao\n\nPress ENTER to return.";
		DirectX::XMFLOAT2 textSize = Font_MeasureText(text, 2.5f);
		Font_Print(text, SCREEN_WIDTH * 0.5f - textSize.x * 0.5f, SCREEN_HEIGHT * 0.5f - textSize.y * 0.5f, 2.5f);
	}
}