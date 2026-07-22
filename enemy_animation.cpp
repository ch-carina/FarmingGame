/*----------------------------------------------------
 HEADER COMMENT

 EnemyAnimation.cpp
 controller different enemy sprite animation sates
                                   Author: Carina Chao
                                   Date: 2026/07/22
 ----------------------------------------------------*/
#pragma once
#include "enemy.h"
#include "enemy_animation.h"
#include "collision.h"
#include "texture.h"
#include "sprite.h"
#include <DirectXMath.h> 

static EnemySpriteInfo g_EnemySpriteInfo[EnemyType_MAX];

void EnemyAnimation_Initialize()
{
    //Animation Sprites 
    g_EnemySpriteInfo[EnemyType_Rabbit].animation[EnemyAnim_Run] =
    {
        Texture_Load(L"assets/Animal/Rabbit_JumpL.png", true),32,32,0,9,0.15f,5
    };

    g_EnemySpriteInfo[EnemyType_Rabbit].animation[EnemyAnim_Eating] =
    {
        Texture_Load(L"assets/Animal/Rabbit_Nibble.png", true),0,32,0,7,10.15f,4 
    };

    g_EnemySpriteInfo[EnemyType_Bear].animation[EnemyAnim_Run] =
    {
        Texture_Load(L"assets/Animal/Rabbit_Nibble.png", true),0,32,0,7,10.15f,4
    };
    
    g_EnemySpriteInfo[EnemyType_Bear].animation[EnemyAnim_Eating] =
    {
        Texture_Load(L"assets/Animal/Rabbit_JumpL.png", true),32,32,0,9,0.15f,5
    };

}

void EnemyAnimation_Update(Enemy& enemy, float deltaTime)
{

    AnimInfo anim =
        EnemyAnimation_GetInfo(
            enemy.type,
            enemy.animState);

    enemy.animTimer += deltaTime;

    if (enemy.animTimer >= anim.frameRate)
    {
        enemy.animTimer = 0.0f;

        enemy.currentFrame++;

        if (enemy.currentFrame >= anim.frameCount)
        {
            enemy.currentFrame = 0;
        }
    }

}

AnimInfo EnemyAnimation_GetInfo(EnemyType type, EnemyAnimState state) 
{

    return { g_EnemySpriteInfo[type].animation[state]};

}

const EnemySpriteInfo& EnemyAnimation_GetSpriteInfo(EnemyType type)
{
    return g_EnemySpriteInfo[type];
}


