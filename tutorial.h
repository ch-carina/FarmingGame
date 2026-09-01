#pragma once
#ifndef TUTORIAL_H
#define TUTORIAL_H

void Tutorial_Initialize();
void Tutorial_Finalize();
void Tutorial_Begin();
void Tutorial_Update(float delta_time);
void Tutorial_Draw();
bool Tutorial_IsComplete();

#endif //TUTORIAL_H