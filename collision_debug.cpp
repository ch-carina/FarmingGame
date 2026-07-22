/*----------------------------------------------------
 HEADER COMMENT

 Collision_debug.cpp
								   Author: Carina Chao
								   Date: 2026/07/06
 ----------------------------------------------------*/

#include "collision_debug.h"
#include "texture.h"
#include "direct3d.h"
#include "debug_ostream.h"
#include "shader.h"
#include "config.h"
#include "sprite.h"

#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;

static ID3D11Buffer* g_pVertexBuffer = nullptr; // 頂点バッファ

static ID3D11SamplerState* g_pSamplerStatePoint{ nullptr };

static ID3D11BlendState* g_pBlendState{ nullptr };

static ID3D11DepthStencilState* g_pDepthStencilState = nullptr;

static ID3D11Buffer* g_pVSConstantBuffer1{ nullptr };
static ID3D11Buffer* g_pPSConstantBuffer0{ nullptr };

static constexpr int NUM_VERTEX{9};

// 頂点構造体
struct Vertex
{
    XMFLOAT3 position; // 頂点座標
    XMFLOAT2 texcoord; // テクスチャ座標
};

static int g_White_TextureID{ -1};

void CollisionDebug_Initialize()
{
    // 頂点バッファ生成
    D3D11_BUFFER_DESC bd
    {
        .ByteWidth = sizeof(Vertex) * NUM_VERTEX,
        .Usage = D3D11_USAGE_DYNAMIC,
        .BindFlags = D3D11_BIND_VERTEX_BUFFER,
        .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
    };

    // 頂点バッファへ送るデータの作成
    Vertex v[NUM_VERTEX]{};

    constexpr float angle = XM_2PI / 8.0f; 
    for(int i = 0; i < 9; i++)
    {
        const float x = cosf(angle * i);
        const float y = sinf(angle * i);
        v[i].position = { x, y, 0.0f };
        v[i].texcoord = { 0.0f, 0.0f };
    }

    D3D11_SUBRESOURCE_DATA sd{};
    sd.pSysMem = v;

    HRESULT hr = Direct3D_GetDevice()->CreateBuffer(&bd, &sd, &g_pVertexBuffer);
    if (FAILED(hr)) {
        hal::dout << "collision_debug.cpp:頂点バッファの生成に失敗しました。\n";
    }

    // 定数バッファUV用マトリクスバッファの作成
    D3D11_BUFFER_DESC cb_desc{};
    cb_desc.ByteWidth = sizeof(XMFLOAT4X4); // バッファのサイズ (16の倍数であること)
    cb_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    // 定数バッファとして使用する
    // 頂点シェーダー用定数バッファの作成
    hr = Direct3D_GetDevice()->CreateBuffer(&cb_desc, nullptr, &g_pVSConstantBuffer1);

    if (FAILED(hr)) {
        hal::dout << "Sprite.cpp:UV用行列の定数バッファの生成に失敗しました。\n";
    }

    // 定数バッファカラーバッファの作成
    cb_desc.ByteWidth = sizeof(XMFLOAT4);
    hr = Direct3D_GetDevice()->CreateBuffer(&cb_desc, nullptr, &g_pPSConstantBuffer0);

    if (FAILED(hr)) {
        hal::dout << "Sprite.cpp:カラー用定数バッファの生成に失敗しました。\n";
    }

    // サンプラーステートの作成
    D3D11_SAMPLER_DESC sampler_desc{
        .Filter = D3D11_FILTER_MIN_MAG_MIP_POINT,
        // .Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
        // .Filter = D3D11_FILTER_ANISOTROPIC,
        .AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
        .AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
        .AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
        // .MaxAnisotropy = 16,
        .ComparisonFunc = D3D11_COMPARISON_NEVER,
        .BorderColor = { 1.0f, 1.0f, 1.0f, 1.0f },
        .MinLOD = 0,
        .MaxLOD = D3D11_FLOAT32_MAX
    };

    hr = Direct3D_GetDevice()->CreateSamplerState(&sampler_desc, &g_pSamplerStatePoint);


    //Blend state creation
    // ブレンドステートの設定例（半透明合成）
    D3D11_BLEND_DESC blend_desc{};
    blend_desc.RenderTarget[0].BlendEnable = FALSE;

    blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    hr = Direct3D_GetDevice()->CreateBlendState(&blend_desc, &g_pBlendState);


    // デプスステンシルステートの設定
    D3D11_DEPTH_STENCIL_DESC dsd{};
    dsd.DepthEnable = FALSE; // デプステストを無効化

    hr = Direct3D_GetDevice()->CreateDepthStencilState(&dsd, &g_pDepthStencilState);

	g_White_TextureID = Texture_Load(L"assets/white.png", false);

}

void CollisionDebug_Finalize()
{
	Texture_Release(g_White_TextureID);

	SAFE_RELEASE(g_pDepthStencilState);
	SAFE_RELEASE(g_pBlendState);
	SAFE_RELEASE(g_pVertexBuffer);
	SAFE_RELEASE(g_pSamplerStatePoint);
	SAFE_RELEASE(g_pPSConstantBuffer0);
	SAFE_RELEASE(g_pVSConstantBuffer1);
}

void CollisionDebug_Draw(const CollisionCircle& collision)
{
	XMFLOAT4 color{ 1.0f,1.0f,0.0f,1.0f }; // 黄色
    Direct3D_GetContext()->UpdateSubresource(g_pPSConstantBuffer0, 0, nullptr, &color, 0, 0);
    Direct3D_GetContext()->PSSetConstantBuffers(0, 1, &g_pPSConstantBuffer0);

    // シェーダーを設定する
    Shader_Begin();

    // 変換行列を作る
    XMMATRIX mtxS = XMMatrixScaling(collision.radius, collision.radius, 1.0f);
    XMMATRIX mtxT = XMMatrixTranslation(collision.center.x , collision.center.y, 0.0f);
    XMMATRIX mtxP = XMMatrixOrthographicOffCenterLH(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f);

    XMMATRIX mtx = mtxS * mtxT * mtxP; // 行列を合成

    // 座標変換用行列を頂点シェーダーに設定する
    Shader_SetMatrix(mtx);

    // UV用行列を定数バッファへ送る
    XMMATRIX mtxI = XMMatrixIdentity();
    XMFLOAT4X4 mtxUV;
    XMStoreFloat4x4(&mtxUV, mtxI);

    Direct3D_GetContext()->UpdateSubresource(g_pVSConstantBuffer1, 0, nullptr, &mtxUV, 0, 0);
    Direct3D_GetContext()->VSSetConstantBuffers(1, 1, &g_pVSConstantBuffer1);

    // 頂点バッファを描画パイプラインに設定
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    Direct3D_GetContext()->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);


    // プリミティブトポロジーの設定
    Direct3D_GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);


    // ピクセルシェーダーにサンプラーを設定
    Direct3D_GetContext()->PSSetSamplers(0, 1, &g_pSamplerStatePoint);


    // ピクセルシェーダーにテクスチャを設定
    Texture_SetTexture(g_White_TextureID);


    // ブレンドステートの設定
    Direct3D_GetContext()->OMSetBlendState(g_pBlendState, nullptr, 0xffffffff);

    // デブスステンシルステートの設定
    Direct3D_GetContext()->OMSetDepthStencilState(g_pDepthStencilState, 0);


    // ポリゴン描画命令発行
    Direct3D_GetContext()->Draw(NUM_VERTEX, 0);
}

