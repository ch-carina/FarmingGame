#include "damage_flash.h"
#include "texture.h"
#include "sprite.h"
#include "config.h"
#include <DirectXMath.h>
#include <cmath>
using namespace DirectX;

static int g_TextureID_White{ -1 };
static float g_Timer{ -1.0f }; // -1 = not active
static constexpr float FLASH_DURATION{ 0.5f };
static constexpr float FLASH_BLINK_SPEED{ 18.0f }; // higher = faster blink

void DamageFlash_Initialize()
{
    g_TextureID_White = Texture_Load(L"assets/white.png");
    g_Timer = -1.0f;
}

void DamageFlash_Finalize()
{
    Texture_Release(g_TextureID_White);
}

void DamageFlash_Start()
{
    g_Timer = 0.0f;
}

void DamageFlash_Update(float delta_time)
{
    if (g_Timer < 0.0f) return;
    g_Timer += delta_time;
    if (g_Timer >= FLASH_DURATION) g_Timer = -1.0f;
}

void DamageFlash_Draw()
{
    if (g_Timer < 0.0f) return;

    float fadeOut = 1.0f - (g_Timer / FLASH_DURATION);
    float blink = (sinf(g_Timer * FLASH_BLINK_SPEED) + 1.0f) * 0.5f; // 0..1 pulse

    XMFLOAT4 color{ 1.0f, 0.0f, 0.0f, blink * fadeOut * 0.6f }; // 0.6 caps max opacity

    Sprite_Draw(g_TextureID_White, 0.0f, 0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, color);
}