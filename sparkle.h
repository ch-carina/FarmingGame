#pragma once
#ifndef SPARKLE_H
#define SPARKLE_H

enum SparkleType
{
	SparkleType_Gold, // gold-rank harvest pickup
	SparkleType_MAX
};

void Sparkle_Initialize();
void Sparkle_Finalize();

void Sparkle_Emit(SparkleType type, float x, float y); 
void Sparkle_Update(float delta_time);
void Sparkle_Draw();

#endif //SPARKLE_H