#include "sparkle.h"
#include "texture.h"
#include "draw_queue.h"
#include <DirectXMath.h>
#include <cstdlib>
#include <cmath>

using namespace DirectX;

struct SparkleParticle
{
	SparkleType type;
	float x, y;
	float vx, vy;
	float life;
	float maxLife;
	bool isActive;
};

struct SparkleInfo
{
	const wchar_t* filename;
	int textureID;
	int srcWidth, srcHeight; // filled in at Initialize from the loaded texture

	XMFLOAT4 tint;
	int count;
	float size;
	float minSpeed, maxSpeed;
	float minLife, maxLife;
	float minAngle, maxAngle; // radians; 0 = right, PI/2 = down (screen space)
	float gravity;
};

static SparkleInfo g_SparkleInfo[SparkleType_MAX] =
{
	{ L"assets/UI/coin.PNG", -1, 0, 0, {1.0f, 0.85f, 0.2f, 1.0f}, 14, 20.0f, 90.0f, 220.0f, 0.35f, 0.6f, XM_PI, XM_PI * 2.0f, 260.0f },
};

static constexpr int SPARKLE_MAX = 64;
static SparkleParticle g_Sparkles[SPARKLE_MAX]{};

static float RandomRange(float lo, float hi)
{
	return lo + (rand() / (float)RAND_MAX) * (hi - lo);
}

void Sparkle_Initialize()
{
	for (int i = 0; i < SparkleType_MAX; i++)
	{
		SparkleInfo& info = g_SparkleInfo[i];
		info.textureID = Texture_Load(info.filename, true);
		info.srcWidth = (int)Texture_GetWidth(info.textureID);
		info.srcHeight = (int)Texture_GetHeight(info.textureID);
	}
	for (SparkleParticle& p : g_Sparkles) p.isActive = false;
}

void Sparkle_Finalize()
{
	for (int i = 0; i < SparkleType_MAX; i++)
	{
		Texture_Release(g_SparkleInfo[i].textureID);
	}
}

void Sparkle_Emit(SparkleType type, float x, float y)
{
	const SparkleInfo& info = g_SparkleInfo[type];

	for (int n = 0; n < info.count; n++)
	{
		int slot = -1;
		for (int i = 0; i < SPARKLE_MAX; i++)
		{
			if (!g_Sparkles[i].isActive) { slot = i; break; }
		}
		if (slot == -1) return; // pool full -- drop the rest of this burst

		float angle = RandomRange(info.minAngle, info.maxAngle);
		float speed = RandomRange(info.minSpeed, info.maxSpeed);

		SparkleParticle& p = g_Sparkles[slot];
		p.type = type;
		p.x = x;
		p.y = y;
		p.vx = cosf(angle) * speed;
		p.vy = sinf(angle) * speed;
		p.maxLife = RandomRange(info.minLife, info.maxLife);
		p.life = p.maxLife;
		p.isActive = true;
	}
}

void Sparkle_Update(float delta_time)
{
	for (SparkleParticle& p : g_Sparkles)
	{
		if (!p.isActive) continue;

		const SparkleInfo& info = g_SparkleInfo[p.type];
		p.vy += info.gravity * delta_time;
		p.x += p.vx * delta_time;
		p.y += p.vy * delta_time;

		p.life -= delta_time;
		if (p.life <= 0.0f) p.isActive = false;
	}
}

void Sparkle_Draw()
{
	for (SparkleParticle& p : g_Sparkles)
	{
		if (!p.isActive) continue;

		const SparkleInfo& info = g_SparkleInfo[p.type];
		float t = p.life / p.maxLife; 
		float size = info.size * (0.5f + 0.5f * t);

		XMFLOAT4 tint = info.tint;
		tint.w *= t;

		DrawQueue_Push(info.textureID, p.x - size * 0.5f, p.y - size * 0.5f, size, size,
			0, 0, info.srcWidth, info.srcHeight, p.y, 0.0f, tint);
	}
}