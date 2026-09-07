#ifndef GAME_H
#define GAME_H

void Game_Initialize();
void Game_Finalize();

void Game_Update(float delta_time);
//void Game_FixedUpdate();
void Game_Draw();

void Game_StopBGM();
void Game_ResumeBGM();
#endif //GAME_H