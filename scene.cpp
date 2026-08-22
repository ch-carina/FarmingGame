/*----------------------------------------------------
 HEADER COMMENT

 scene.cpp
 Scene header - managing scenes (changing in between)
								   Author: Carina Chao
								   Date: 2026/07/09
 ----------------------------------------------------*/
#include "scene.h"
#include "menu.h"
#include "game.h"
#include "results.h"

 //you can change the enum (kTitle) to make debugging faster 
static Scene g_CurrentScene{ kTitle }; 
static Scene g_NextScene{ g_CurrentScene };

void Scene_Initialize()
{
	switch (g_CurrentScene)
	{
	case kTitle:
		Menu_Initialize();
		break;
	case kGame:
		Game_Initialize();
		break;
	case kResult:
		Results_Initialize();
		break;
	}
}

void Scene_Finalize()
{
	switch (g_CurrentScene)
	{
	case kTitle:
		Menu_Finalize();
		break;
	case kGame:
		Game_Finalize();
		break;
	case kResult:
		Results_Finalize();
		break;
	}
}

void Scene_Update(float delta_time)
{
	switch (g_CurrentScene)
	{
	case kTitle:
		Menu_Update(delta_time);
		break;
	case kGame:
		Game_Update(delta_time);
		break;
	case kResult:
		Results_Update(delta_time);
		break;
	}
}

void Scene_Draw()
{
	switch (g_CurrentScene)
	{
	case kTitle:
		Menu_Draw();
		break;
	case kGame:
		Game_Draw();
		break;
	case kResult:
		Results_Draw();
		break;
	}
}

void Scene_SetNextScene(Scene scene)
{
	g_NextScene = scene;
}

void Scene_Change()
{
	if (g_NextScene != g_CurrentScene)
	{
		//Release current scene 
		Scene_Finalize();

		//make current scene the next scene 
		g_CurrentScene = g_NextScene;

		//initalize current scene is now the next scene 
		Scene_Initialize();
		//This costs many resources in a big game. But for a game size like this, it shouldnt affect it
	}
}
