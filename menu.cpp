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
#include "Audio.h"
#include "level.h"
#include <Windows.h>
#include <cstdlib>

static int g_TextureID_MenuTitle{ -1 };
static float g_AccumalatedTime{ 0.0f };
static bool g_IsChangeScene{};

static int g_AudioID_MenuMove{ -1 };
static int g_AudioID_MenuSelect{ -1 };

enum MenuState
{
	kMenuMain,
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
static constexpr float SELECT_SCALE = 1.1f;

enum MenuRabbitState
{
	MenuRabbitState_Inactive,
	MenuRabbitState_Crossing, // used for both entering and leaving -- same animation, same direction
	MenuRabbitState_Nibbling,
};

struct MenuRabbit
{
	MenuRabbitState state;
	float x, y;
	float targetX;
	int direction; // +1 = left-to-right, -1 = right-to-left
	float nibbleTimer;
	int animFrame;
	float animTimer;
	bool hasNibbled;
};

static constexpr int MAX_MENU_RABBITS = 2;
static MenuRabbit g_MenuRabbits[MAX_MENU_RABBITS];
static float g_RabbitSpawnTimer = 0.0f;

static int g_RabbitRunTextureID = TEXTURE_INVALID_ID;
static int g_RabbitNibbleTextureID = TEXTURE_INVALID_ID;

static constexpr float RABBIT_SIZE = 64.0f;
static constexpr float RABBIT_SPEED = 90.0f;
static constexpr float RABBIT_SPAWN_MIN = 4.0f;
static constexpr float RABBIT_SPAWN_MAX = 9.0f;
static constexpr float RABBIT_NIBBLE_MIN = 1.5f;
static constexpr float RABBIT_NIBBLE_MAX = 3.0f;
static constexpr float RABBIT_FRAME_RATE = 0.15f;
static constexpr int RABBIT_FRAME_SIZE = 96;

// matches the run/nibble sheets already defined in enemy_animation.cpp
static constexpr int RABBIT_RUN_START = 0, RABBIT_RUN_COUNT = 4, RABBIT_RUN_COLUMNS = 5;
static constexpr int RABBIT_NIBBLE_START = 0, RABBIT_NIBBLE_COUNT = 7, RABBIT_NIBBLE_COLUMNS = 4;

static float RandomRange(float minV, float maxV)
{
	return minV + (maxV - minV) * (rand() / (float)RAND_MAX);
}

static void SpawnMenuRabbit()
{
	for (int i = 0; i < MAX_MENU_RABBITS; i++)
	{
		if (g_MenuRabbits[i].state != MenuRabbitState_Inactive) continue;

		MenuRabbit& r = g_MenuRabbits[i];
		bool fromLeft = (rand() % 2) == 0;

		r.direction = fromLeft ? 1 : -1;
		r.x = fromLeft ? -RABBIT_SIZE : (float)SCREEN_WIDTH;
		r.y = RandomRange(SCREEN_HEIGHT * 0.55f, SCREEN_HEIGHT - RABBIT_SIZE - 30.0f);
		r.targetX = RandomRange(SCREEN_WIDTH * 0.25f, SCREEN_WIDTH * 0.75f);
		r.state = MenuRabbitState_Crossing;
		r.nibbleTimer = 0.0f;
		r.animFrame = 0;
		r.animTimer = 0.0f;
		r.hasNibbled = false;
		return; // only ever fill one open slot per spawn tick
	}
}

static void UpdateMenuRabbits(float delta_time)
{
	g_RabbitSpawnTimer -= delta_time;
	if (g_RabbitSpawnTimer <= 0.0f)
	{
		SpawnMenuRabbit();
		g_RabbitSpawnTimer = RandomRange(RABBIT_SPAWN_MIN, RABBIT_SPAWN_MAX);
	}

	for (int i = 0; i < MAX_MENU_RABBITS; i++)
	{
		MenuRabbit& r = g_MenuRabbits[i];
		if (r.state == MenuRabbitState_Inactive) continue;

		int frameCount = (r.state == MenuRabbitState_Nibbling) ? RABBIT_NIBBLE_COUNT : RABBIT_RUN_COUNT;
		r.animTimer += delta_time;
		if (r.animTimer >= RABBIT_FRAME_RATE)
		{
			r.animTimer = 0.0f;
			r.animFrame = (r.animFrame + 1) % frameCount;
		}

		if (r.state == MenuRabbitState_Crossing)
		{
			r.x += r.direction * RABBIT_SPEED * delta_time;

			bool reachedPauseSpot = (r.direction > 0 && r.x >= r.targetX) || (r.direction < 0 && r.x <= r.targetX);
			bool leftScreen = (r.x < -RABBIT_SIZE - 10.0f || r.x > SCREEN_WIDTH + 10.0f);

			if (leftScreen)
			{
				r.state = MenuRabbitState_Inactive;
			}
			else if (!r.hasNibbled && reachedPauseSpot)
			{
				r.hasNibbled = true;
				r.state = MenuRabbitState_Nibbling;
				r.nibbleTimer = RandomRange(RABBIT_NIBBLE_MIN, RABBIT_NIBBLE_MAX);
				r.animFrame = 0;
				r.animTimer = 0.0f;
			}
		}
		else // Nibbling
		{
			r.nibbleTimer -= delta_time;
			if (r.nibbleTimer <= 0.0f)
			{
				r.state = MenuRabbitState_Crossing;
				r.animFrame = 0;
				r.animTimer = 0.0f;
			}
		}
	}
}

static void DrawMenuRabbits()
{
	for (int i = 0; i < MAX_MENU_RABBITS; i++)
	{
		const MenuRabbit& r = g_MenuRabbits[i];
		if (r.state == MenuRabbitState_Inactive) continue;

		bool nibbling = (r.state == MenuRabbitState_Nibbling);
		int textureID = nibbling ? g_RabbitNibbleTextureID : g_RabbitRunTextureID;
		int startFrame = nibbling ? RABBIT_NIBBLE_START : RABBIT_RUN_START;
		int columns = nibbling ? RABBIT_NIBBLE_COLUMNS : RABBIT_RUN_COLUMNS;

		int frame = startFrame + r.animFrame;
		int column = frame % columns;
		int row = frame / columns;

		int srcX = column * RABBIT_FRAME_SIZE;
		int srcY = row * RABBIT_FRAME_SIZE;
		int srcW = RABBIT_FRAME_SIZE;

		// the art faces left; mirror the UVs (not the quad) to face right --
		// a negative scale flips the geometry's winding and gets backface-culled instead
		if (r.direction > 0)
		{
			srcX += srcW;
			srcW = -srcW;
		}

		Sprite_Draw(textureID,
			r.x, r.y,
			RABBIT_SIZE, RABBIT_SIZE,
			srcX, srcY,
			srcW, RABBIT_FRAME_SIZE);
	}
}

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
		if (!g_IsChangeScene)
		{
			Level_RequestTutorialFromMenu();
			Fade_Start(FadeType::kOut, 1.0f, { 0.0f,0.0f,0.0f,0.0f });
			g_IsChangeScene = true;
		}
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
	g_AudioID_MenuMove = LoadAudio("assets/SFX/menu_movement.wav");
	g_AudioID_MenuSelect = LoadAudio("assets/SFX/menu_select.wav");
	g_AccumalatedTime = 0.0f;
	g_IsChangeScene = false;
	g_MenuState = kMenuMain;
	g_SelectedButton = 0;

	g_RabbitRunTextureID = Texture_Load(L"assets/Animal/Rabbit_JumpL.png", true);
	g_RabbitNibbleTextureID = Texture_Load(L"assets/Animal/Rabbit_Nibble.png", true);
	for (int i = 0; i < MAX_MENU_RABBITS; i++) g_MenuRabbits[i].state = MenuRabbitState_Inactive;
	g_RabbitSpawnTimer = RandomRange(1.0f, 3.0f); // first rabbit shows up soon after the menu loads

	Fade_Start(FadeType::kIn, 0.5f, { 0.0f,0.0f,0.0f,1.0f });
}

void Menu_Finalize()
{
	Texture_Release(g_TextureID_MenuTitle);
	Texture_Release(g_SlotCapLeftID);
	Texture_Release(g_SlotCapMidID);
	Texture_Release(g_SlotCapRightID);
	Texture_Release(g_RabbitRunTextureID);
	Texture_Release(g_RabbitNibbleTextureID);
	Font_Finalize();
	UnloadAudio(g_AudioID_MenuMove);
	UnloadAudio(g_AudioID_MenuSelect);
	Ground_Finalize();
}

void Menu_Update(float delta_time)
{
	g_AccumalatedTime += delta_time;
	UpdateMenuRabbits(delta_time);

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
		PlayAudio(g_AudioID_MenuMove);
	}
	if (InputKeyboard_IsTrigger(KK_W))
	{
		g_SelectedButton = (g_SelectedButton - 1 + kButtonCount) % kButtonCount;
		PlayAudio(g_AudioID_MenuMove);
	}

	if (InputKeyboard_IsTrigger(KK_ENTER))
	{
		PlayAudio(g_AudioID_MenuSelect);
		ActivateButton(g_SelectedButton);
	}
}

void Menu_Draw()
{
	Ground_Draw();
	DrawMenuRabbits();
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
	else if (g_MenuState == kMenuCredits)
	{
		const char* text = "Made by Carina Chao\n\nPress ENTER to return.";
		DirectX::XMFLOAT2 textSize = Font_MeasureText(text, 2.5f);
		Font_Print(text, SCREEN_WIDTH * 0.5f - textSize.x * 0.5f, SCREEN_HEIGHT * 0.5f - textSize.y * 0.5f, 2.5f);
	}
}