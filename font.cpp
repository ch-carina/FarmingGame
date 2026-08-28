/*----------------------------------------------------
 HEADER COMMENT

 font.cpp
 Bitmap font handler using an ASCII texture atlas
								   Author: Carina Chao
								   Date: 2026/08/12
 ----------------------------------------------------*/

#include "font.h"
#include "texture.h"
#include "sprite.h"
using namespace DirectX;

static constexpr int FONT_COLUMNS = 10;
static constexpr float FONT_SOURCE_CELL = 32.0f;  // font.png: tightly-packed 10x10 grid of 32x32 glyphs

static constexpr float FONT_CELL_WIDTH = 5.0f;
static constexpr float FONT_CELL_HEIGHT = 5.0f;

static const XMFLOAT4 SHADOW_COLOR{ 0.35f, 0.35f, 0.35f, 1.0f };
static constexpr float SHADOW_OFFSET = 0.5f; 

static int g_TextureID_Font{ -1 };

void Font_Initialize()
{
	g_TextureID_Font = Texture_Load(L"assets/UI/font.png", false);
}

void Font_Finalize()
{
	Texture_Release(g_TextureID_Font);
}

static void DrawGlyphRun(const char* text, float x, float y, float scale, const XMFLOAT4& color)
{
	float cursorX = x;
	float cursorY = y;

	for (const char* p = text; *p; ++p)
	{
		char c = *p;

		if (c == '\n')
		{
			cursorX = x;
			cursorY += FONT_CELL_HEIGHT * scale;
			continue;
		}

		if (c < ' ' || c > '~')
		{
			c = '?';
		}

		if (c != ' ')
		{
			int index = c - ' ';
			int column = index % FONT_COLUMNS;
			int row = index / FONT_COLUMNS;

			int srcX = (int)(column * FONT_SOURCE_CELL);
			int srcY = (int)(row * FONT_SOURCE_CELL);

			Sprite_Draw(
				g_TextureID_Font,
				cursorX, cursorY,
				FONT_CELL_WIDTH * scale, FONT_CELL_HEIGHT * scale,
				srcX, srcY,
				(int)FONT_SOURCE_CELL, (int)FONT_SOURCE_CELL,
				color);
		}

		cursorX += FONT_CELL_WIDTH * scale;
	}
}

void Font_Print(const char* text, float x, float y, float scale, const XMFLOAT4& color)
{
	// gray drop shadow first, white (or caller's color) text on top
	DrawGlyphRun(text, x + SHADOW_OFFSET * scale, y + SHADOW_OFFSET * scale, scale, SHADOW_COLOR);
	DrawGlyphRun(text, x, y, scale, color);
}

XMFLOAT2 Font_MeasureText(const char* text, float scale)
{
	float width = 0.0f;
	float widestLine = 0.0f;
	float height = FONT_CELL_HEIGHT * scale;

	for (const char* p = text; *p; ++p)
	{
		if (*p == '\n')
		{
			if (width > widestLine) widestLine = width;
			width = 0.0f;
			height += FONT_CELL_HEIGHT * scale;
			continue;
		}
		width += FONT_CELL_WIDTH * scale;
	}
	if (width > widestLine) widestLine = width;

	return { widestLine, height };
}