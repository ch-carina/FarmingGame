#include "spotlight.h"
#include "shader.h"
#include "direct3d.h"
#include "game_player.h"
#include "level.h"
#include "config.h"
#include <DirectXMath.h>
using namespace DirectX;

struct SpotlightVertex
{
	XMFLOAT3 position;
	XMFLOAT4 color; // unused by the spotlight shader, kept only to match the shared input layout
	XMFLOAT2 texcoord;
};

static ID3D11Buffer* g_pVertexBuffer = nullptr;
static ID3D11BlendState* g_pBlendState = nullptr;
static ID3D11DepthStencilState* g_pDepthStencilState = nullptr;

static constexpr float INNER_RADIUS = 90.0f;
static constexpr float OUTER_RADIUS = 250.0f;
static constexpr float DARK_ALPHA = 0.65f;
static constexpr float SPOTLIGHT_START_PROGRESS = 0.5f;
static constexpr float SPOTLIGHT_FADE_START = 0.4f; // level2 progress where it starts fading in
static constexpr float SPOTLIGHT_FADE_END = 0.7f;   // level2 progress where it reaches full strength


void Spotlight_Initialize()
{
	SpotlightVertex v[4]{};
	v[0].position = { -0.5f, -0.5f, 0.0f };
	v[1].position = { 0.5f, -0.5f, 0.0f };
	v[2].position = { -0.5f,  0.5f, 0.0f };
	v[3].position = { 0.5f,  0.5f, 0.0f };
	v[0].texcoord = { 0.0f, 0.0f };
	v[1].texcoord = { 1.0f, 0.0f };
	v[2].texcoord = { 0.0f, 1.0f };
	v[3].texcoord = { 1.0f, 1.0f };

	D3D11_BUFFER_DESC bd{};
	bd.ByteWidth = sizeof(SpotlightVertex) * 4;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA sd{};
	sd.pSysMem = v;

	Direct3D_GetDevice()->CreateBuffer(&bd, &sd, &g_pVertexBuffer);

	D3D11_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	Direct3D_GetDevice()->CreateBlendState(&blendDesc, &g_pBlendState);

	D3D11_DEPTH_STENCIL_DESC dsd{};
	dsd.DepthEnable = FALSE;
	Direct3D_GetDevice()->CreateDepthStencilState(&dsd, &g_pDepthStencilState);
}

void Spotlight_Finalize()
{
	SAFE_RELEASE(g_pDepthStencilState);
	SAFE_RELEASE(g_pBlendState);
	SAFE_RELEASE(g_pVertexBuffer);
}

void Spotlight_Draw()
{
	LevelType level = Level_GetCurrent();
	if (level != Level2 && level != Level3) return;

	float strength = 1.0f; // Level3: unchanged, always full strength

	if (level == Level2)
	{
		float progress = 0.0f;
		const LevelLayout& layout = Level_GetCurrentLayout();
		if (layout.timeLimit > 0.0f)
		{
			progress = 1.0f - (Level_GetTimeRemaining() / layout.timeLimit);
		}

		if (progress <= SPOTLIGHT_FADE_START) return; // nothing to draw yet

		strength = (progress - SPOTLIGHT_FADE_START) / (SPOTLIGHT_FADE_END - SPOTLIGHT_FADE_START);
		if (strength > 1.0f) strength = 1.0f;
	}

	UINT stride = sizeof(SpotlightVertex);
	UINT offset = 0;
	Direct3D_GetContext()->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
	Direct3D_GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	Shader_BeginSpotlight();

	Direct3D_GetContext()->OMSetBlendState(g_pBlendState, nullptr, 0xffffffff);
	Direct3D_GetContext()->OMSetDepthStencilState(g_pDepthStencilState, 0);

	XMMATRIX mtxS = XMMatrixScaling((float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, 1.0f);
	XMMATRIX mtxT = XMMatrixTranslation(SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f, 0.0f);
	XMMATRIX mtxP = XMMatrixOrthographicOffCenterLH(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f);
	Shader_SetMatrix(mtxS * mtxT * mtxP);

	CollisionCircle playerCircle = GamePlayer_GetCollisionCircle();
	Shader_SetSpotlightParams(playerCircle.center, INNER_RADIUS, OUTER_RADIUS, DARK_ALPHA * strength, { (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT });

	Direct3D_GetContext()->Draw(4, 0);
}