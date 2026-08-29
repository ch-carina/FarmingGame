/*----------------------------------------------------
 HEADER COMMENT

 water.cpp
 Decorative, player-blocking water pools placed per level,
 separate from crop plots. Border tiles are static; the open
 center tiles get an animated ripple shader.
								   Author: Carina Chao
								   Date: 2026/08/29
 ----------------------------------------------------*/
#include "water.h"
#include "texture.h"
#include "sprite.h"
#include "shader.h"
#include "direct3d.h"
#include "config.h"
#include <DirectXMath.h>
using namespace DirectX;

enum WaterTileType
{
	WaterTile_TL, WaterTile_TC, WaterTile_TR,
	WaterTile_ML, WaterTile_C, WaterTile_MR,
	WaterTile_BL, WaterTile_BC, WaterTile_BR,
	WaterTile_MAX
};

struct WaterTile
{
	float x, y;
	WaterTileType type;
};

static constexpr float WATER_TILE_SIZE = 96.0f;
static constexpr int MAX_WATER_TILES = 64;

static WaterTile g_WaterTiles[MAX_WATER_TILES];
static int g_WaterTileCount = 0;

static int g_WaterTextures[WaterTile_MAX];

static const wchar_t* waterFiles[WaterTile_MAX] =
{
	L"assets/GroundTiles/Water/Water_TL.PNG",
	L"assets/GroundTiles/Water/Water_TM.PNG",
	L"assets/GroundTiles/Water/Water_TR.PNG",
	L"assets/GroundTiles/Water/Water_LM.PNG",
	L"assets/GroundTiles/Water/Water.PNG",
	L"assets/GroundTiles/Water/Water_RM.PNG",
	L"assets/GroundTiles/Water/Water_BL.PNG",
	L"assets/GroundTiles/Water/Water_BM.PNG",
	L"assets/GroundTiles/Water/Water_BR.PNG",
};

struct WaterVertex
{
	XMFLOAT3 position;
	XMFLOAT4 color; // unused by the water shader, kept only to match the shared input layout
	XMFLOAT2 texcoord;
};

static ID3D11Buffer* g_pVertexBuffer = nullptr;
static ID3D11SamplerState* g_pWaterSampler = nullptr;
static ID3D11BlendState* g_pBlendState = nullptr;
static ID3D11DepthStencilState* g_pDepthStencilState = nullptr;

static float g_TotalTime = 0.0f;

bool Water_IsBlocked(const CollisionCircle& circle)
{
	for (int i = 0; i < g_WaterTileCount; i++)
	{
		CollisionBox box{ WATER_TILE_SIZE, WATER_TILE_SIZE, g_WaterTiles[i].x, g_WaterTiles[i].y };
		if (CircleVsBox(circle, box)) return true;
	}
	return false;
}

void Water_Initialize()
{
	for (int i = 0; i < WaterTile_MAX; i++)
	{
		g_WaterTextures[i] = Texture_Load(waterFiles[i], true);
	}
	g_WaterTileCount = 0;
	g_TotalTime = 0.0f;

	WaterVertex v[4]{};
	v[0].position = { -0.5f, -0.5f, 0.0f };
	v[1].position = { 0.5f, -0.5f, 0.0f };
	v[2].position = { -0.5f,  0.5f, 0.0f };
	v[3].position = { 0.5f,  0.5f, 0.0f };
	v[0].texcoord = { 0.0f, 0.0f };
	v[1].texcoord = { 1.0f, 0.0f };
	v[2].texcoord = { 0.0f, 1.0f };
	v[3].texcoord = { 1.0f, 1.0f };

	D3D11_BUFFER_DESC bd{};
	bd.ByteWidth = sizeof(WaterVertex) * 4;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA sd{};
	sd.pSysMem = v;

	Direct3D_GetDevice()->CreateBuffer(&bd, &sd, &g_pVertexBuffer);

	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	Direct3D_GetDevice()->CreateSamplerState(&samplerDesc, &g_pWaterSampler);

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

void Water_Finalize()
{
	SAFE_RELEASE(g_pDepthStencilState);
	SAFE_RELEASE(g_pBlendState);
	SAFE_RELEASE(g_pWaterSampler);
	SAFE_RELEASE(g_pVertexBuffer);

	for (int i = 0; i < WaterTile_MAX; i++)
	{
		Texture_Release(g_WaterTextures[i]);
	}
}

void Water_LoadRegions(const PlotRegion regions[], int regionCount)
{
	g_WaterTileCount = 0;

	for (int r = 0; r < regionCount; r++)
	{
		const PlotRegion& region = regions[r];

		for (int row = 0; row < region.height; row++)
		{
			for (int col = 0; col < region.width; col++)
			{
				if (g_WaterTileCount >= MAX_WATER_TILES) return;

				WaterTile& tile = g_WaterTiles[g_WaterTileCount];
				tile.x = (region.gridX + col) * WATER_TILE_SIZE;
				tile.y = (region.gridY + row) * WATER_TILE_SIZE;

				if (row == 0 && col == 0)                                     tile.type = WaterTile_TL;
				else if (row == 0 && col == region.width - 1)                 tile.type = WaterTile_TR;
				else if (row == region.height - 1 && col == 0)                tile.type = WaterTile_BL;
				else if (row == region.height - 1 && col == region.width - 1) tile.type = WaterTile_BR;
				else if (row == 0)                                            tile.type = WaterTile_TC;
				else if (row == region.height - 1)                           tile.type = WaterTile_BC;
				else if (col == 0)                                           tile.type = WaterTile_ML;
				else if (col == region.width - 1)                            tile.type = WaterTile_MR;
				else                                                         tile.type = WaterTile_C;

				g_WaterTileCount++;
			}
		}
	}
}

bool Water_IsBlocked(float x, float y)
{
	int col = (int)(x / WATER_TILE_SIZE);
	int row = (int)(y / WATER_TILE_SIZE);

	for (int i = 0; i < g_WaterTileCount; i++)
	{
		int tileCol = (int)(g_WaterTiles[i].x / WATER_TILE_SIZE);
		int tileRow = (int)(g_WaterTiles[i].y / WATER_TILE_SIZE);

		if (tileCol == col && tileRow == row) return true;
	}
	return false;
}

void Water_Update(float delta_time)
{
	g_TotalTime += delta_time;
}

void Water_Draw()
{
	// base layer: every tile gets the animated water fill via the shader
	UINT stride = sizeof(WaterVertex);
	UINT offset = 0;
	Direct3D_GetContext()->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
	Direct3D_GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	Shader_BeginWater();

	ID3D11ShaderResourceView* waterSRV = Texture_GetSRV(g_WaterTextures[WaterTile_C]);
	Direct3D_GetContext()->PSSetShaderResources(0, 1, &waterSRV);
	Direct3D_GetContext()->PSSetSamplers(0, 1, &g_pWaterSampler);

	Direct3D_GetContext()->OMSetBlendState(g_pBlendState, nullptr, 0xffffffff);
	Direct3D_GetContext()->OMSetDepthStencilState(g_pDepthStencilState, 0);

	for (int i = 0; i < g_WaterTileCount; i++)
	{
		XMMATRIX mtxS = XMMatrixScaling(WATER_TILE_SIZE, WATER_TILE_SIZE, 1.0f);
		XMMATRIX mtxT = XMMatrixTranslation(g_WaterTiles[i].x + WATER_TILE_SIZE * 0.5f, g_WaterTiles[i].y + WATER_TILE_SIZE * 0.5f, 0.0f);
		XMMATRIX mtxP = XMMatrixOrthographicOffCenterLH(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f);
		Shader_SetMatrix(mtxS * mtxT * mtxP);

		Shader_SetWaterParams(g_TotalTime, { g_WaterTiles[i].x, g_WaterTiles[i].y });

		Direct3D_GetContext()->Draw(4, 0);
	}

	// overlay layer: static shoreline art on top -- never touched by the shader
	for (int i = 0; i < g_WaterTileCount; i++)
	{
		if (g_WaterTiles[i].type == WaterTile_C) continue; // pure open water has no border overlay

		Sprite_Draw(g_WaterTextures[g_WaterTiles[i].type], g_WaterTiles[i].x, g_WaterTiles[i].y,
			WATER_TILE_SIZE, WATER_TILE_SIZE, 0, 0, 96, 96, 0.0f);
	}
}