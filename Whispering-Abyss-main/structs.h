#ifndef STRUCTS_H
#define STRUCTS_H

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
using namespace std;

struct Player {
    string name;
    int    health, maxHealth;
    float  x, y, vx, vy;
    float  width, height;
    sf::RectangleShape shape;
    float  speed, moveSpeed, jumpSpeed, gravity;
    float  coyoteTime, coyoteTimer, jumpBuffer;
    bool   jumpHeld, onGround, damaged;
    sf::Sprite sprite;
    float  PLAYER_SCALE;
};

struct Enemy {
    sf::Vector2f position;
    float  speed;
    int    direction, health, maxHealth, damage;
    float  attackRange, attackCooldown, attackTimer;
    bool   canAttack, dead, dying;
    sf::Sprite sprite;
    float  scaleX, scaleY, flashTimer, vy;
    bool   onGround;
    enum State { IDLE, WALK, ATTACK, HURT, DEAD } state;
    int    currentFrame;
    float  animTimer, animSpeed;
};

struct BigE {
    float x, y, vx = 0.f, vy = 0.f;
    float width, height;

    int   health = 12;
    int   maxHealth = 12;
    int   damage = 2;
    bool  dead = false;
    bool  dying = false;
    int   direction = -1;

    sf::Texture texAttack[10];
    sf::Texture texDie[10];
    sf::Texture texHurt[10];
    sf::Texture texIdle[10];
    sf::Texture texJump[10];
    sf::Texture texRun[10];
    sf::Texture texWalk[10];
    sf::Sprite  sprite;

    static const int FRAMES_WALK = 10;
    static const int FRAMES_ATTACK = 10;
    static const int FRAMES_CHARGE = 10;
    static const int FRAMES_HURT = 10;
    static const int FRAMES_DEATH = 10;
    static const int FRAMES_IDLE = 10;
    static const int FRAMES_RUN = 10;
    static const int FRAMES_JUMP = 10;

    int   currentFrame = 0;
    float animTimer = 0.f;
    float animSpeed = 0.06f;
    float walkAnimSpeed = 0.08f;
    float chargeAnimSpeed = 0.09f;
    float flashTimer = 0.f;

    enum State { IDLE, WALK, ATTACK, CHARGE, HURT, DEAD } state = IDLE;

    float aggroRange = 500.f;
    float attackRange = 90.f;
    float chargeRange = 300.f;

    float attackCooldown = 2.0f;
    float attackTimer = 0.f;
    float chargeCooldown = 4.0f;
    float chargeTimer = 0.f;
    float chargeSpeed = 400.f;
    bool  isCharging = false;
    float chargeDuration = 0.6f;
    float chargeClock = 0.f;
    float walkSpeed = 70.f;

    bool  attackHitDealt = false;
    bool  chargeHitDealt = false;
};

struct Camera {
    float x, y, width, height;
    float smoothSpeed, offsetX, offsetY;
};

struct TilesetInfo {
    int         firstGid, lastGid;
    sf::Texture texture;
    string      type;
};

struct ScoreEntry {
    string name;
    float  time;
};

// Per-frame hitbox data (used in attack hit detection)
struct FrameHitbox {
    int   frame;
    float offsetX, offsetY, w, h;
};

struct IntroText
{
    std::string fullText;
    std::string visibleText;

    float charDelay;
    float timer;

    int index;
    bool finished;
};

enum GameScene
{
    SCENE_INTRO,
    SCENE_GAME,
    SCENE_INSTRUCTIONS
};
#endif #pragma once
