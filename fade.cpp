/*----------------------------------------------------
 HEADER COMMENT

 fade.cpp
 fade header - loading (changing in between)
								   Author: Carina Chao
								   Date: 2026/07/10
 ----------------------------------------------------*/
#include "fade.h"
#include "texture.h"
#include "sprite.h"
#include "config.h"
#include "font.h"
#include <DirectXMath.h> // #include <cmath> Direcmath includes this already
using namespace DirectX;
#include <utility>

static float g_AccumulatedTime{}; 
static int g_TextureID_White{ -1 };
static float g_FadeTime{};
static XMFLOAT4 g_FadeColor{0.0f,0.0f,0.0f,1.0f};

static int g_TextureID_WalkR{ -1 };
static float g_WalkAnimTimer{ 0.0f };
static int g_WalkFrame{ 0 };
static constexpr float WALK_FRAME_WIDTH = 48.0f;
static constexpr float WALK_FRAME_HEIGHT = 72.0f;
static constexpr int WALK_COLUMNS = 4;
static constexpr int WALK_FRAME_COUNT = 8;
static constexpr float WALK_FRAME_RATE = 0.15f;
static constexpr float WALK_DRAW_SCALE = 1.75f;
static constexpr float WALK_MARGIN = 40.0f;
static constexpr float BOB_AMPLITUDE = 8.0f;
static constexpr float BOB_SPEED = 6.0f;
static constexpr float LETTER_BOB_PHASE = 0.6f;

static int g_LoopsCompleted = 0;
static constexpr int LOOPS_BEFORE_FINISH = 2;

static FadeType g_FadeType{};

void Fade_Initialize()
{
	g_TextureID_White = Texture_Load(L"assets/white.png");
	g_TextureID_WalkR = Texture_Load(L"assets/MC/MC_WalkR.png", true);
	g_AccumulatedTime = 0.0f;
	g_FadeTime = 0.0f; 
	g_FadeColor.w = 0.0f; 

}

void Fade_Finalize()
{
	Texture_Release(g_TextureID_White);
	Texture_Release(g_TextureID_WalkR);
}

void Fade_Update(float delta_time)
{
	if (g_FadeTime <= 0.0f) return; //dont start fade 

	bool loopingDone = (g_LoopsCompleted >= LOOPS_BEFORE_FINISH);

	if (g_FadeType == kIn && !loopingDone)
	{
		g_FadeColor.w = 1.0f; // stay fully hidden until the loading loop has played out
	}
	else if (g_FadeType == kOut && loopingDone)
	{
		g_FadeColor.w = 1.0f; // already fully opaque -- hold instead of accumulating further
	}
	else
	{
		g_AccumulatedTime += delta_time;
		float alpha{ std::min(1.0f, g_AccumulatedTime / g_FadeTime) };
		g_FadeColor.w = g_FadeType == kOut ? alpha : 1.0f - alpha;
	}

	// g_LoopsCompleted only gates Fade_IsFinished() below -- the walk cycle itself
	// never stops, so the character keeps moving for as long as the fade is on screen
	g_WalkAnimTimer += delta_time;
	if (g_WalkAnimTimer >= WALK_FRAME_RATE)
	{
		g_WalkAnimTimer = 0.0f;
		g_WalkFrame++;
		if (g_WalkFrame >= WALK_FRAME_COUNT)
		{
			g_WalkFrame = 0;
			g_LoopsCompleted++;
		}
	}
}

void Fade_Draw()
{
	if (g_FadeTime <= 0.0f || g_FadeColor.w <= 0.0f) { return; }
	Sprite_Draw(g_TextureID_White, 0.0f, 0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, g_FadeColor);

	int column = g_WalkFrame % WALK_COLUMNS;
	int row = g_WalkFrame / WALK_COLUMNS;
	float drawW = WALK_FRAME_WIDTH * WALK_DRAW_SCALE;
	float drawH = WALK_FRAME_HEIGHT * WALK_DRAW_SCALE;

	float bobOffset = sinf(g_AccumulatedTime * BOB_SPEED) * BOB_AMPLITUDE;
	float drawX = SCREEN_WIDTH - drawW - WALK_MARGIN;
	float drawY = SCREEN_HEIGHT - drawH - WALK_MARGIN + bobOffset;

	DirectX::XMFLOAT4 walkColor{ 1.0f, 1.0f, 1.0f, g_FadeColor.w };
	Sprite_Draw(g_TextureID_WalkR,
		drawX, drawY,
		drawW, drawH,
		(int)(column * WALK_FRAME_WIDTH), (int)(row * WALK_FRAME_HEIGHT),
		(int)WALK_FRAME_WIDTH, (int)WALK_FRAME_HEIGHT,
		walkColor);

	constexpr float LOADING_TEXT_SCALE = 2.0f;
	constexpr float LOADING_TEXT_GAP = 10.0f;
	const char* loadingText = "LOADING...";
	float baseY = SCREEN_HEIGHT - drawH - WALK_MARGIN; // character's un-bobbed anchor

	DirectX::XMFLOAT2 textSize = Font_MeasureText(loadingText, LOADING_TEXT_SCALE);
	float letterWidth = Font_MeasureText("A", LOADING_TEXT_SCALE).x; // the font is fixed-width per glyph
	float textStartX = drawX - LOADING_TEXT_GAP - textSize.x; // sits to the left of the character
	float textBaseY = baseY + drawH - textSize.y * 0.5f;             // just below its feet

	char letter[2] = { '\0', '\0' };
	for (int i = 0; loadingText[i] != '\0'; i++)
	{
		letter[0] = loadingText[i];

		float letterBob = sinf(g_AccumulatedTime * BOB_SPEED - i * LETTER_BOB_PHASE) * BOB_AMPLITUDE;
		Font_Print(letter, textStartX + i * letterWidth, textBaseY + letterBob, LOADING_TEXT_SCALE, walkColor);
	}
}

void Fade_Start(FadeType type, float fadeTime, const DirectX::XMFLOAT4& color)
{
	g_FadeType = type;
	g_FadeTime = fadeTime;
	g_FadeColor = color;

	g_AccumulatedTime = 0.0f;
	g_LoopsCompleted = 0;
	g_WalkFrame = 0;
	g_WalkAnimTimer = 0.0f;
}
bool Fade_IsFinished()
{
	if (g_LoopsCompleted < LOOPS_BEFORE_FINISH) return false;
	return g_AccumulatedTime / g_FadeTime >= 1.0f;
}
