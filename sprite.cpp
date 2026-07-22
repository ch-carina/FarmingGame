//#include "WICTextureLoader11.h"
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
static ID3D11SamplerState* g_pSamplerState{ nullptr };
static ID3D11BlendState* g_pBlendState{ nullptr };
static ID3D11DepthStencilState* g_pDepthStencilState = nullptr;
static ID3D11Buffer* g_pVSConstantBuffer1{ nullptr };
static ID3D11Buffer* g_pPSConstantBuffer0{ nullptr };

//static ID3D11ShaderResourceView* g_pShaderResourceView{ nullptr }; //load texture
//static ID3D11Resource* g_pTexture{ nullptr }; //put texture 
//static int g_TextureID{ TEXTURE_INVALID_ID };

static constexpr int NUM_VERTEX{ 4 };

// 頂点構造体
struct Vertex
{
    XMFLOAT3 position; // 頂点座標
    XMFLOAT4 color; // color (RGBA) 
    XMFLOAT2 texcoord; // テクスチャ座標
};


bool Sprite_Initialize()
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

    // 頂点情報を書き込み
    v[0].position = { -0.5f, -0.5f, 0.0f }; // 左上
    v[1].position = { 0.5f, -0.5f, 0.0f }; // 右上
    v[2].position = { -0.5f,  0.5f, 0.0f }; // 左下
    v[3].position = { 0.5f,  0.5f, 0.0f }; // 右下

    v[0].texcoord = { 0.0f, 0.0f };
    v[1].texcoord = { 1.0f, 0.0f };
    v[2].texcoord = { 0.0f, 1.0f };
    v[3].texcoord = { 1.0f, 1.0f };

    D3D11_SUBRESOURCE_DATA sd{};
    sd.pSysMem = v;

    HRESULT hr = Direct3D_GetDevice()->CreateBuffer(&bd, &sd, &g_pVertexBuffer);
    if (FAILED(hr)) {
        hal::dout << "Sprite.cpp:頂点バッファの生成に失敗しました。\n";
        return false;
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
        return false;
    }

    // 定数バッファカラーバッファの作成
    cb_desc.ByteWidth = sizeof(XMFLOAT4);
    hr = Direct3D_GetDevice()->CreateBuffer(&cb_desc, nullptr, &g_pPSConstantBuffer0);

    if (FAILED(hr)) {
        hal::dout << "Sprite.cpp:カラー用定数バッファの生成に失敗しました。\n";
        return false;
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
        .BorderColor = { 1.0f, 1.0f, 0.0f, 1.0f },
        .MinLOD = 0,
        .MaxLOD = D3D11_FLOAT32_MAX
    };

    hr = Direct3D_GetDevice()->CreateSamplerState(&sampler_desc, &g_pSamplerState);


    //Blend state creation
    // ブレンドステートの設定例（半透明合成）
    D3D11_BLEND_DESC blend_desc{};
    blend_desc.RenderTarget[0].BlendEnable = TRUE;

    blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA; // 描画する色の係数
    blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA; // すでにある色の係数
    blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

    blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

    blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    ID3D11BlendState* pBlendState = nullptr;

    //color (RGB) = srcRGB * srcAlpha + dest  * (1-srcAlpha)

    hr = Direct3D_GetDevice()->CreateBlendState(&blend_desc, &g_pBlendState);


    // デプスステンシルステートの設定
    D3D11_DEPTH_STENCIL_DESC dsd{};
    dsd.DepthEnable = FALSE; // デプステストを無効化

    hr = Direct3D_GetDevice()->CreateDepthStencilState(&dsd, &g_pDepthStencilState);
    
    return true;
}

void Sprite_Finalize()
{
    SAFE_RELEASE(g_pDepthStencilState);
    SAFE_RELEASE(g_pBlendState);
    SAFE_RELEASE(g_pVertexBuffer);
    SAFE_RELEASE(g_pSamplerState);
    SAFE_RELEASE(g_pPSConstantBuffer0);
    SAFE_RELEASE(g_pVSConstantBuffer1);
}

void Sprite_Draw(int texture_id, float position_x, float position_y, const DirectX::XMFLOAT4& color)
{
    Sprite_Draw(texture_id, position_x, position_y,
        (float)Texture_GetWidth(texture_id), (float)Texture_GetHeight(texture_id), color);
}

void Sprite_Draw(int texture_id, float position_x, float position_y, float width, float height, const DirectX::XMFLOAT4& color)
{
    Direct3D_GetContext()->UpdateSubresource(g_pPSConstantBuffer0, 0, nullptr, &color, 0, 0);
    Direct3D_GetContext()->PSSetConstantBuffers(0, 1, &g_pPSConstantBuffer0);

    // シェーダーを設定する
    Shader_Begin();

    // 変換行列を作る
    XMMATRIX mtxS = XMMatrixScaling(width, height, 1.0f);
    XMMATRIX mtxT = XMMatrixTranslation(position_x + width * 0.5f, position_y + height * 0.5f, 0.0f);
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
    Direct3D_GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);


    // ピクセルシェーダーにサンプラーを設定
    Direct3D_GetContext()->PSSetSamplers(0, 1, &g_pSamplerState);


    // ピクセルシェーダーにテクスチャを設定
    Texture_SetTexture(texture_id);


    // ブレンドステートの設定
    Direct3D_GetContext()->OMSetBlendState(g_pBlendState, nullptr, 0xffffffff);

    // デブスステンシルステートの設定
    Direct3D_GetContext()->OMSetDepthStencilState(g_pDepthStencilState, 0);


    // ポリゴン描画命令発行
    Direct3D_GetContext()->Draw(NUM_VERTEX, 0);
}

void Sprite_Draw(int texture_id, float position_x, float position_y, int texture_x, int texture_y, int texture_width, int texture_height, const DirectX::XMFLOAT4& color)
{
    Sprite_Draw(texture_id, position_x, position_y, (float)texture_width, (float)texture_height, texture_x, texture_y, texture_width, texture_height, color);
}

void Sprite_Draw(int texture_id, float position_x, float position_y, float width, float height, int texture_x, int texture_y, int texture_width, int texture_height, const DirectX::XMFLOAT4& color)
{
    Direct3D_GetContext()->UpdateSubresource(g_pPSConstantBuffer0, 0, nullptr, &color, 0, 0);
    Direct3D_GetContext()->PSSetConstantBuffers(0, 1, &g_pPSConstantBuffer0);

    // シェーダーを設定する
    Shader_Begin();

    // 変換行列を作る
    XMMATRIX mtxS = XMMatrixScaling(width, height, 1.0f);
    XMMATRIX mtxT = XMMatrixTranslation(position_x + width * 0.5f, position_y + height * 0.5f, 0.0f);
    XMMATRIX mtxP = XMMatrixOrthographicOffCenterLH(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f);

    XMMATRIX mtx = mtxS * mtxT * mtxP; // 行列を合成

    // 座標変換用行列を頂点シェーダーに設定する
    Shader_SetMatrix(mtx);

    // UV用行列を定数バッファへ送る
    float tx = texture_x / (float)Texture_GetWidth(texture_id);
    float ty = texture_y / (float)Texture_GetHeight(texture_id);
    float tw = texture_width / (float)Texture_GetWidth(texture_id);
    float th = texture_height / (float)Texture_GetHeight(texture_id);

    mtxS = XMMatrixScaling(tw, th, 1.0f);
    mtxT = XMMatrixTranslation(tx, ty, 0.0f);
    XMFLOAT4X4 mtxUV;
    XMStoreFloat4x4(&mtxUV, XMMatrixTranspose(mtxS * mtxT));

    Direct3D_GetContext()->UpdateSubresource(g_pVSConstantBuffer1, 0, nullptr, &mtxUV, 0, 0);
    Direct3D_GetContext()->VSSetConstantBuffers(1, 1, &g_pVSConstantBuffer1);

    // 頂点バッファを描画パイプラインに設定
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    Direct3D_GetContext()->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);


    // プリミティブトポロジーの設定
    Direct3D_GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);


    // ピクセルシェーダーにサンプラーを設定
    Direct3D_GetContext()->PSSetSamplers(0, 1, &g_pSamplerState);


    // ピクセルシェーダーにテクスチャを設定
    Texture_SetTexture(texture_id);


    // ブレンドステートの設定
    Direct3D_GetContext()->OMSetBlendState(g_pBlendState, nullptr, 0xffffffff);

    // デブスステンシルステートの設定
    Direct3D_GetContext()->OMSetDepthStencilState(g_pDepthStencilState, 0);


    // ポリゴン描画命令発行
    Direct3D_GetContext()->Draw(NUM_VERTEX, 0);
}

void Sprite_Draw(int texture_id, float position_x, float position_y, float width, float height, int texture_x, int texture_y, int texture_width, int texture_height, float angle, const DirectX::XMFLOAT2& scale, const DirectX::XMFLOAT4& color)
{
    Direct3D_GetContext()->UpdateSubresource(g_pPSConstantBuffer0, 0, nullptr, &color, 0, 0);
    Direct3D_GetContext()->PSSetConstantBuffers(0, 1, &g_pPSConstantBuffer0);

    // シェーダーを設定する
    Shader_Begin();

    // 変換行列を作る
    XMMATRIX mtxS = XMMatrixScaling(width * scale.x, height * scale.y, 1.0f);
    XMMATRIX mtxR = XMMatrixRotationZ(angle);
    XMMATRIX mtxT = XMMatrixTranslation(position_x + width * 0.5f, position_y + height * 0.5f, 0.0f);
    XMMATRIX mtxP = XMMatrixOrthographicOffCenterLH(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f);

    XMMATRIX mtx = mtxS * mtxR * mtxT * mtxP; // 行列を合成

    // 座標変換用行列を頂点シェーダーに設定する
    Shader_SetMatrix(mtx);

    // UV用行列を定数バッファへ送る
    float tx = texture_x / (float)Texture_GetWidth(texture_id);
    float ty = texture_y / (float)Texture_GetHeight(texture_id);
    float tw = texture_width / (float)Texture_GetWidth(texture_id);
    float th = texture_height / (float)Texture_GetHeight(texture_id);

    mtxS = XMMatrixScaling(tw, th, 1.0f);
    mtxT = XMMatrixTranslation(tx, ty, 0.0f);
    XMFLOAT4X4 mtxUV;
    XMStoreFloat4x4(&mtxUV, XMMatrixTranspose(mtxS * mtxT));

    Direct3D_GetContext()->UpdateSubresource(g_pVSConstantBuffer1, 0, nullptr, &mtxUV, 0, 0);
    Direct3D_GetContext()->VSSetConstantBuffers(1, 1, &g_pVSConstantBuffer1);

    // 頂点バッファを描画パイプラインに設定
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    Direct3D_GetContext()->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);


    // プリミティブトポロジーの設定
    Direct3D_GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);


    // ピクセルシェーダーにサンプラーを設定
    Direct3D_GetContext()->PSSetSamplers(0, 1, &g_pSamplerState);


    // ピクセルシェーダーにテクスチャを設定
    Texture_SetTexture(texture_id);


    // ブレンドステートの設定
    Direct3D_GetContext()->OMSetBlendState(g_pBlendState, nullptr, 0xffffffff);

    // デブスステンシルステートの設定
    Direct3D_GetContext()->OMSetDepthStencilState(g_pDepthStencilState, 0);


    // ポリゴン描画命令発行
    Direct3D_GetContext()->Draw(NUM_VERTEX, 0);
}
