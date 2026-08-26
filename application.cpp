#include <Windows.h>
#include "application.h"
#include "direct3d.h"
#include "shader.h"
#include "texture.h"
#include "sprite.h"
#include "config.h"
#include "flipbook_animation.h"
#include "Audio.h"
#include "fade.h"

#include "input_keyboard.h"

#include "scene.h"
#include "game.h"
#include "game_player.h"


bool Application_Initialize(HWND hWnd)
{
	InitAudio();
	//Intializing Input for Keyboard and Mouse
	InputKeyboard_Initialize();
	
	//Initializing Direct3D
	if (!Direct3D_Initialize(hWnd)) return false;

	Shader_Initialize(Direct3D_GetDevice(), Direct3D_GetContext()); //Shader Initialize (Reading Shader file) 

	FlipBookAnimation_Initialize();

	Texture_Initialize();
	
	Sprite_Initialize();

	Fade_Initialize();
	Scene_Initialize();

	return true;
}
void Application_Finalize()
{
	Scene_Finalize();
	Fade_Finalize();
	FlipBookAnimation_Finalize();
	Sprite_Finalize();
	Texture_Finalize();
	Shader_Finalize();
	Direct3D_Finalize();

	UninitAudio();
}
void Application_Update(float delta_time)
{
	Scene_Change();
	InputKeyboard_Update(delta_time);

	FlipBookAnimation_Update(delta_time);

	Scene_Update(delta_time);
	Fade_Update(delta_time);
}
void Application_FixedUpdate()
{
}
void Application_Draw()
{
	Scene_Draw();
	Fade_Draw();
}
