/*==============================================================================

   シェーダー [shader.h]
														 Author : Youhei Sato
														 Date   : 2025/05/15
	Note: 
	- Added thing to make blur shader work (Carina) [2026/08/13] 
	- Added thing to make water shader work (Carina) [2026/08/29]
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef SHADER_H
#define	SHADER_H

#include <d3d11.h>
#include <DirectXMath.h>

bool Shader_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Shader_Finalize();

void Shader_SetMatrix(const DirectX::XMMATRIX& matrix);

void Shader_Begin();

void Shader_LoadBlurPixelShader();
void Shader_BeginBlur();
void Shader_SetBlurParams(const DirectX::XMFLOAT2& texelSize, const DirectX::XMFLOAT2& direction);

void Shader_LoadWaterPixelShader();
void Shader_BeginWater();
void Shader_SetWaterParams(float time, const DirectX::XMFLOAT2& worldOffset);
#endif // SHADER_H
