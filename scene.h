/*----------------------------------------------------
 HEADER COMMENT

 scene.h
 Scene header - managing scenes (changing in between)
								   Author: Carina Chao
								   Date: 2026/07/09
 ----------------------------------------------------*/
#ifndef SCENE_H
#define SCENE_H

void Scene_Initialize();
void Scene_Finalize();
void Scene_Update(float delta_time);
void Scene_Draw();

enum Scene //can add more scenes in here 
{
	kTitle,
	kGame,
	kResult
};

void Scene_SetNextScene(Scene scene);
void Scene_Change();

#endif // SCENE_H
