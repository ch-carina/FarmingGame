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
        Texture_Load(L"assets/Animal/Rabbit_JumpL.png", true),96,96,4,3,0.15f,5
    };

    g_EnemySpriteInfo[EnemyType_Rabbit].animation[EnemyAnim_Eating] =
    {
        Texture_Load(L"assets/Animal/Rabbit_Nibble.png", true),96,96,0,7,0.15f,4
    };

    g_EnemySpriteInfo[EnemyType_Rabbit].animation[EnemyAnim_Escape] =
    {
        Texture_Load(L"assets/Animal/Rabbit_JumpL.png", true),96,96,4,3,0.15f,5
    };

    g_EnemySpriteInfo[EnemyType_Rabbit].collision =
    {
        {ENEMY_DRAW_SIZE * 0.5f, ENEMY_DRAW_SIZE * 0.5f },
        ENEMY_DRAW_SIZE * 0.4f
    };

    g_EnemySpriteInfo[EnemyType_Bear].animation[EnemyAnim_Run] =
    {
        Texture_Load(L"assets/Animal/Rabbit_Nibble.png", true),0,32,0,7,10.15f,4
    };
    
    g_EnemySpriteInfo[EnemyType_Bear].animation[EnemyAnim_Eating] =
    {
        Texture_Load(L"assets/Animal/Rabbit_JumpL.png", true),32,32,0,9,0.15f,5
    };

    g_EnemySpriteInfo[EnemyType_Mole].animation[EnemyAnim_Burrow] =
    {
        Texture_Load(L"assets/Animal/Mole_Dirt.png", true), 96,96,0,8,0.15f,4
    };

    g_EnemySpriteInfo[EnemyType_Mole].animation[EnemyAnim_Appear] =
    {
        Texture_Load(L"assets/Animal/Mole_Appear.png", true), 96,96,0,8,0.15f,4
    };

    g_EnemySpriteInfo[EnemyType_Mole].collision =
    {
        {ENEMY_DRAW_SIZE * 0.5f, ENEMY_DRAW_SIZE * 0.5f },
        ENEMY_DRAW_SIZE * 0.4f
    };

}

void EnemyAnimation_Finalize()
{
    for (int type = 0; type < EnemyType_MAX; type++)
    {
        for (int stage = 0; stage < EnemyAnim_MAX; stage++)
        {
            Texture_Release(
                g_EnemySpriteInfo[type].animation[stage].textureID);
        }
    }
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


