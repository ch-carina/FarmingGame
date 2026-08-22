/*----------------------------------------------------
 HEADER COMMENT

 GameScore.h
 Displaying of the score in the game
                                   Author: Carina Chao
                                   Date: 2026/07/08
 ----------------------------------------------------*/
#ifndef GAME_SCORE_H
#define GAME_SCORE_H

#include <DirectXMath.h>


void GameScore_Initialize(int digit); //you can choose weather if you you want to display 0s 
void GameScore_Finalize();
void GameScore_SetScore(int score); 
void GameScore_Update(float delta_time);
void GameScore_Draw(const DirectX::XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f });

#endif // GAME_SCORE_H
