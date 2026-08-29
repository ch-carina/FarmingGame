/*----------------------------------------------------
 HEADER COMMENT

 cloud.h
 Ambient cloud blobs that drift across the screen and loop,
 purely decorative -- suggests time passing during a level
                                   Author: Carina Chao
                                   Date: 2026/08/28
 ----------------------------------------------------*/
#pragma once
#ifndef CLOUD_H
#define CLOUD_H

void Cloud_Initialize();
void Cloud_Finalize();
void Cloud_Update(float delta_time);
void Cloud_Draw();

#endif //CLOUD_H
