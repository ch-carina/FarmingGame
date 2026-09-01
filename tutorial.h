#pragma once
#ifndef TUTORIAL_H
#define TUTORIAL_H

void Tutorial_Begin();               // call from Level_Load(LevelTutorial)
void Tutorial_Update(float delta_time);
void Tutorial_Draw();
bool Tutorial_IsComplete();

#endif //TUTORIAL_H