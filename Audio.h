#pragma once



void InitAudio();
void UninitAudio();


int LoadAudio(const char* FileName);
void UnloadAudio(int Index);
void PlayAudio(int Index, bool Loop = false);
void  StopAudio(int Index);

//learn how to add volume in here 