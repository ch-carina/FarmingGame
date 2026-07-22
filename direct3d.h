/*==============================================================================

    Direct3D関連 [direct3d.h]
                                                         Author : Youhei Sato
                                                         Date   : 2026/06/01
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef DIRECT3D_H
#define DIRECT3D_H

#include <Windows.h>
#include <d3d11.h> //for Direct 3D 11 

// セーフリリースマクロ
#define SAFE_RELEASE(o) if (o) { (o)->Release(); o = NULL; }


// Direct3Dの初期化 : 失敗したらfalseを戻す
bool Direct3D_Initialize(HWND window_handle);

// Direct3Dのあとかたづけ
void Direct3D_Finalize();

// バックバッファのクリアなどそのフレームの描画開始時に呼ぶ関数
void Direct3D_Begin();

// バッファのフリップ
void Direct3D_Present();

//Get Device 
ID3D11Device* Direct3D_GetDevice();

//Get Device Context 
ID3D11DeviceContext* Direct3D_GetContext();

unsigned int Direct3D_GetBackBufferWidth();

unsigned int Direct3D_GetBackBufferHeight();

#endif // DIRECT3D_H
