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

static constexpr int FONT_COLUMNS = 18;
static constexpr int FONT_ROWS = 7;
static constexpr float FONT_CELL_WIDTH = 5.0f;   // column pitch AND glyph width (columns sit edge to edge)
static constexpr float FONT_CELL_HEIGHT = 7.0f;  // glyph height only — NOT the row spacing
static constexpr float FONT_ROW_PITCH = 9.0f;    // actual vertical spacing between rows in the atlas
static constexpr float FONT_COLUMN_PITCH = 7.0f;
static constexpr float FONT_ORIGIN_X = 1.0f;
static constexpr float FONT_ORIGIN_Y = 1.0f;

static int g_TextureID_Font{ -1 };

void Font_Initialize()
{
	g_TextureID_Font = Texture_Load(L"assets/UI/pixel-simplicity_grey.png", false);
}

void Font_Finalize()
{
	Texture_Release(g_TextureID_Font);
}

void Font_Print(const char* text, float x, float y, float scale, const XMFLOAT4& color)
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

			int srcX = (int)(FONT_ORIGIN_X + column * FONT_COLUMN_PITCH);
			int srcY = (int)(FONT_ORIGIN_Y + row * FONT_ROW_PITCH);

			Sprite_Draw(
				g_TextureID_Font,
				cursorX, cursorY,
				FONT_CELL_WIDTH * scale, FONT_CELL_HEIGHT * scale,
				srcX, srcY,
				(int)FONT_CELL_WIDTH, (int)FONT_CELL_HEIGHT,
				color);
		}

		cursorX += FONT_CELL_WIDTH * scale;
	}
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