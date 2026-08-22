/*----------------------------------------------------
 HEADER COMMENT

 blur.h

 blur effect 
                                   Author: Carina Chao
                                   Date: 2026/08/13
 ----------------------------------------------------*/
#pragma once
#ifndef BLUR_H
#define BLUR_H

void Blur_Initialize();
void Blur_Finalize();

// Call before drawing the world when you want a blurred snapshot of it.
void Blur_BeginCapture();

// Call after the world has been drawn while captured. Restores the back buffer.
void Blur_EndCapture();

// Blurs the captured scene and draws the result onto the back buffer.
void Blur_DrawBlurred();

#endif //BLUR_H