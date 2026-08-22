/*----------------------------------------------------
 HEADER COMMENT

 blur.cpp

 blur effect
								   Author: Carina Chao
								   Date: 2026/08/13
 ----------------------------------------------------*/

#include "blur.h"
#include "direct3d.h"
#include "shader.h"
#include "config.h"
#include <DirectXMath.h>
using namespace DirectX;

struct RenderTargetTex
{
	ID3D11Texture2D* texture = nullptr;
	ID3D11RenderTargetView* rtv = nullptr;
	ID3D11ShaderResourceView* srv = nullptr;
};

struct BlurVertex
{
	XMFLOAT3 position;
	XMFLOAT4 color;
	XMFLOAT2 texcoord;
};

static RenderTargetTex g_SceneTarget;
static RenderTargetTex g_PingTarget;

static ID3D11Buffer* g_pVertexBuffer = nullptr;
static ID3D11Buffer* g_pUVConstantBuffer = nullptr;
static ID3D11SamplerState* g_pSamplerState = nullptr;

static bool CreateRenderTarget(RenderTargetTex& target, UINT width, UINT height)
{
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	if (FAILED(Direct3D_GetDevice()->CreateTexture2D(&desc, nullptr, &target.texture))) return false;
	if (FAILED(Direct3D_GetDevice()->CreateRenderTargetView(target.texture, nullptr, &target.rtv))) return false;
	if (FAILED(Direct3D_GetDevice()->CreateShaderResourceView(target.texture, nullptr, &target.srv))) return false;

	return true;
}

static void ReleaseRenderTarget(RenderTargetTex& target)
{
	SAFE_RELEASE(target.srv);
	SAFE_RELEASE(target.rtv);
	SAFE_RELEASE(target.texture);
}

void Blur_Initialize()
{
	CreateRenderTarget(g_SceneTarget, SCREEN_WIDTH, SCREEN_HEIGHT);
	CreateRenderTarget(g_PingTarget, SCREEN_WIDTH, SCREEN_HEIGHT);

	BlurVertex v[4]{};
	v[0].position = { -0.5f, -0.5f, 0.0f };
	v[1].position = { 0.5f, -0.5f, 0.0f };
	v[2].position = { -0.5f,  0.5f, 0.0f };
	v[3].position = { 0.5f,  0.5f, 0.0f };
	v[0].texcoord = { 0.0f, 0.0f };
	v[1].texcoord = { 1.0f, 0.0f };
	v[2].texcoord = { 0.0f, 1.0f };
	v[3].texcoord = { 1.0f, 1.0f };

	D3D11_BUFFER_DESC bd{};
	bd.ByteWidth = sizeof(BlurVertex) * 4;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA sd{};
	sd.pSysMem = v;

	Direct3D_GetDevice()->CreateBuffer(&bd, &sd, &g_pVertexBuffer);

	D3D11_BUFFER_DESC uvBufferDesc{};
	uvBufferDesc.ByteWidth = sizeof(XMFLOAT4X4);
	uvBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	Direct3D_GetDevice()->CreateBuffer(&uvBufferDesc, nullptr, &g_pUVConstantBuffer);

	XMFLOAT4X4 identity;
	XMStoreFloat4x4(&identity, XMMatrixIdentity());
	Direct3D_GetContext()->UpdateSubresource(g_pUVConstantBuffer, 0, nullptr, &identity, 0, 0);

	D3D11_SAMPLER_DESC sampler_desc{};
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampler_desc.MinLOD = 0;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

	Direct3D_GetDevice()->CreateSamplerState(&sampler_desc, &g_pSamplerState);
}

void Blur_Finalize()
{
	SAFE_RELEASE(g_pSamplerState);
	SAFE_RELEASE(g_pUVConstantBuffer);
	SAFE_RELEASE(g_pVertexBuffer);
	ReleaseRenderTarget(g_PingTarget);
	ReleaseRenderTarget(g_SceneTarget);
}

void Blur_BeginCapture()
{
	float clear[4]{ 0.0f, 0.0f, 0.0f, 1.0f };
	Direct3D_GetContext()->ClearRenderTargetView(g_SceneTarget.rtv, clear);
	Direct3D_GetContext()->OMSetRenderTargets(1, &g_SceneTarget.rtv, nullptr);
}

void Blur_EndCapture()
{
	ID3D11RenderTargetView* backBuffer = Direct3D_GetBackBufferRTV();
	ID3D11DepthStencilView* depth = Direct3D_GetDepthStencilView();
	Direct3D_GetContext()->OMSetRenderTargets(1, &backBuffer, depth);
}

static void DrawFullscreenQuad(ID3D11ShaderResourceView* srv, const XMFLOAT2& direction)
{
	UINT stride = sizeof(BlurVertex);
	UINT offset = 0;
	Direct3D_GetContext()->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
	Direct3D_GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	Shader_BeginBlur();
	Direct3D_GetContext()->VSSetConstantBuffers(1, 1, &g_pUVConstantBuffer);

	XMMATRIX mtxS = XMMatrixScaling((float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, 1.0f);
	XMMATRIX mtxT = XMMatrixTranslation(SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f, 0.0f);
	XMMATRIX mtxP = XMMatrixOrthographicOffCenterLH(0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f);
	Shader_SetMatrix(mtxS * mtxT * mtxP);

	Shader_SetBlurParams({ 1.0f / SCREEN_WIDTH, 1.0f / SCREEN_HEIGHT }, direction);

	Direct3D_GetContext()->PSSetShaderResources(0, 1, &srv);
	Direct3D_GetContext()->PSSetSamplers(0, 1, &g_pSamplerState);
	Direct3D_GetContext()->OMSetBlendState(nullptr, nullptr, 0xffffffff);

	Direct3D_GetContext()->Draw(4, 0);

	ID3D11ShaderResourceView* nullSRV = nullptr;
	Direct3D_GetContext()->PSSetShaderResources(0, 1, &nullSRV);
}

void Blur_DrawBlurred()
{
	// Pass 1: horizontal blur, Scene -> Ping
	Direct3D_GetContext()->OMSetRenderTargets(1, &g_PingTarget.rtv, nullptr);
	DrawFullscreenQuad(g_SceneTarget.srv, { 1.0f, 0.0f });

	// Pass 2: vertical blur, Ping -> back buffer
	ID3D11RenderTargetView* backBuffer = Direct3D_GetBackBufferRTV();
	ID3D11DepthStencilView* depth = Direct3D_GetDepthStencilView();
	Direct3D_GetContext()->OMSetRenderTargets(1, &backBuffer, depth);
	DrawFullscreenQuad(g_PingTarget.srv, { 0.0f, 1.0f });
}