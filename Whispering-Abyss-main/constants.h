#pragma once
#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>
using namespace std;

const int    FRAME_W = 128;
const int    FRAME_H = 128;
const string assetsPath = "assets/";
const float  playerHitboxOffsetX = (128.f * 0.7f - 32.f) / 2.f;
const int MENU_COUNT = 5;   // Play, Settings, Leaderboard, Credits, Exit

// Player animation
const float animSpeed = 0.1f;
const float attackAnimSpeed = 0.03f;
const int   FRAMES_IDLE = 6;
const int   FRAMES_RUN = 8;
const int   FRAMES_JUMP = 3;
const int   FRAMES_HURT = 3;
const int   FRAMES_DEAD = 5;
const int   FRAMES_ATK1 = 6;
const int   FRAMES_ATK2 = 4;
const int   FRAMES_ATK3 = 3;
const int   FRAMES_ATTACK = FRAMES_ATK1 + FRAMES_ATK2 + FRAMES_ATK3;

// Enemy animation
const int   E_ATTACK_FRAMES = 5;
const int   E_DEATH_FRAMES = 6;
const int   E_HURT_FRAMES = 2;
const int   E_IDLE_FRAMES = 3;
const int   E_WALK_FRAMES = 6;
const float ENEMY_SCALE = 0.5f;

// Terminal velocity cap (prevents falling through thin tiles)
const float MAX_FALL_SPEED = 900.f;

#endif