#pragma once
#pragma once
#ifndef ENEMY_H
#define ENEMY_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include "structs.h"
#include "tilemap.h"
#include "player.h"
using namespace std;

static const float ENEMY_GRAVITY = 1200.f;
static const float ENEMY_MAX_FALL = 900.f;

void applyEnemyGravity(Enemy& e, float dt,
    int mapWidth, int tileWidth, int tileHeight,
    const vector<int>& tiles, const vector<TilesetInfo*>& tilesets)
{
    e.vy += ENEMY_GRAVITY * dt;
    if (e.vy > ENEMY_MAX_FALL) e.vy = ENEMY_MAX_FALL;

    e.position.y += e.vy * dt;
    e.onGround = false;

    float eW = 256.f * e.scaleX;
    float eH = 256.f * e.scaleY;

    const float VISUAL_PADDING = 100.f * e.scaleY;  
    float visualH = eH - VISUAL_PADDING;

    int leftCol = (int)((e.position.x + eW * 0.3f) / tileWidth);
    int rightCol = (int)((e.position.x + eW * 0.7f) / tileWidth);
    int botTile = (int)((e.position.y + visualH) / tileHeight);  
    int topTile = (int)(e.position.y / tileHeight);

    for (int col = leftCol; col <= rightCol; col++) {
        int idxBot = botTile * mapWidth + col;
        if (idxBot >= 0 && idxBot < (int)tiles.size() &&
            isSolid(tiles[idxBot], tilesets) && e.vy >= 0)
        {
            e.position.y = botTile * tileHeight - visualH;  
            e.vy = 0.f;
            e.onGround = true;
        }
        int idxTop = topTile * mapWidth + col;
        if (idxTop >= 0 && idxTop < (int)tiles.size() &&
            isSolid(tiles[idxTop], tilesets) && e.vy < 0)
        {
            e.position.y = (topTile + 1) * tileHeight;
            e.vy = 0.f;
        }
    }
}

void preventEnemyObstacleCollision(Enemy& e,
    int mapWidth, int tileWidth, int tileHeight,
    const vector<int>& tiles,
    const vector<TilesetInfo*>& tilesets)
{
    float eW = 256.f * e.scaleX;
    float eH = 256.f * e.scaleY;
    int   bodyMidRow = (int)((e.position.y + eH * 0.5f) / tileHeight);
    float frontEdgeX = (e.direction == 1)
        ? e.position.x + eW + 2.f
        : e.position.x - 2.f;
    int frontCol = (int)(frontEdgeX / tileWidth);
    int frontIdx = bodyMidRow * mapWidth + frontCol;
    if (frontIdx >= 0 && frontIdx < (int)tiles.size()
        && isObstacle(tiles[frontIdx], tilesets))
    {
        if (e.direction == 1) e.position.x = frontCol * tileWidth - eW;
        else                  e.position.x = (frontCol + 1) * tileWidth;
        e.direction *= -1;
    }
}

void updateEnemy(Enemy& e, Player& player, bool& isGameOver,
    bool& canTakeDamage, float& damageTimer,
    float dt, int mapWidth, int tileWidth, int tileHeight,
    const vector<int>& tiles, const vector<TilesetInfo*>& tilesetPtrs,
    sf::Texture eTexAttack[], sf::Texture eTexDeath[],
    sf::Texture eTexHurt[], sf::Texture eTexIdle[],
    sf::Texture eTexWalk[])
{
    if (e.dead) return;

    applyEnemyGravity(e, dt, mapWidth, tileWidth, tileHeight, tiles, tilesetPtrs);

    sf::FloatRect playerBox(player.x, player.y, player.width, player.height);
    sf::FloatRect enemyBodyBox(
        e.position.x + 256.f * e.scaleX * 0.38f,
        e.position.y + 256.f * e.scaleY * 0.25f,
        256.f * e.scaleX * 0.25f,
        256.f * e.scaleY * 0.55f);

    if (playerBox.intersects(enemyBodyBox)) {
        float pCX = player.x + player.width / 2.f;
        float eCX = e.position.x + 256.f * e.scaleX * 0.5f;
        float pushDir = (pCX > eCX) ? 1.f : -1.f;
        player.x += pushDir * 3.f;
        e.position.x -= pushDir * 3.f;
    }

    float eCenterX = e.position.x + 256.f * e.scaleX * 0.5f;
    float pCenterX = player.x + player.width * 0.5f;
    float dx = pCenterX - eCenterX;
    float dy = (player.y + player.height * 0.5f) - (e.position.y + 256.f * e.scaleY * 0.5f);
    float dist = sqrtf(dx * dx + dy * dy);

    Enemy::State prevEState = e.state;

    if (e.state != Enemy::HURT && e.state != Enemy::DEAD && !e.dying)
    {
        if (dist < e.attackRange)
        {
            e.state = Enemy::ATTACK;
            e.speed = 0.f;
            e.attackTimer += dt;
            if (e.attackTimer >= e.attackCooldown && canTakeDamage) {
                damagePlayer(player, e.damage, isGameOver);
                canTakeDamage = false;
                damageTimer = 0.f;
                e.attackTimer = 0.f;
                float kd = (player.x > e.position.x) ? 1.f : -1.f;
                player.x += kd * 40.f;
                player.vy = -200.f;
            }
        }
        else if (dist < 400.f)
        {
            e.state = Enemy::WALK;
            e.speed = 80.f;
            float eW = 256.f * e.scaleX;
            float eH = 256.f * e.scaleY;
            float visualH = eH - (100.f * e.scaleY);

            int tryDir = (dx > 0) ? 1 : -1;
            float edgeX = (tryDir == 1)
                ? e.position.x + eW * 0.7f
                : e.position.x + eW * 0.3f;
            float belowY = e.position.y + visualH + 2.f;

            int edgeCol = (int)(edgeX / tileWidth);
            int belowRow = (int)(belowY / tileHeight);
            int idx = belowRow * mapWidth + edgeCol;

            if (idx >= 0 && idx < (int)tiles.size() && isSolid(tiles[idx], tilesetPtrs)) {
                e.direction = tryDir;
                e.position.x += e.direction * e.speed * dt;
            }
            else {
                e.state = Enemy::IDLE;
                e.speed = 0.f;
            }

        }
        else
        {
            e.state = Enemy::WALK;
            e.speed = 60.f;
            float eW = 256.f * e.scaleX;
            float eH = 256.f * e.scaleY;
            float visualH = eH - (100.f * e.scaleY);  

            float edgeX = (e.direction == 1)
                ? e.position.x + eW * 0.7f   
                : e.position.x + eW * 0.3f;

            float belowY = e.position.y + visualH + 2.f;

            int edgeCol = (int)(edgeX / tileWidth);
            int belowRow = (int)(belowY / tileHeight);
            int idx = belowRow * mapWidth + edgeCol;

            if (idx >= 0 && idx < (int)tiles.size() && isSolid(tiles[idx], tilesetPtrs))
                e.position.x += e.direction * e.speed * dt;
            else
                e.direction *= -1;

        }
    }

    preventEnemyObstacleCollision(e, mapWidth, tileWidth, tileHeight, tiles, tilesetPtrs);

    if (e.direction == 1) { e.sprite.setOrigin(0, 0);     e.sprite.setScale(e.scaleX, e.scaleY); }
    else { e.sprite.setOrigin(256.f, 0); e.sprite.setScale(-e.scaleX, e.scaleY); }

    if (e.state != prevEState) { e.currentFrame = 0; e.animTimer = 0.f; }

    e.animTimer += dt;
    if (e.animTimer >= e.animSpeed) {
        e.animTimer = 0.f;
        int eTF = E_IDLE_FRAMES;
        switch (e.state) {
        case Enemy::IDLE:   eTF = E_IDLE_FRAMES;   break;
        case Enemy::WALK:   eTF = E_WALK_FRAMES;   break;
        case Enemy::ATTACK: eTF = E_ATTACK_FRAMES; break;
        case Enemy::HURT:   eTF = E_HURT_FRAMES;   break;
        case Enemy::DEAD:   eTF = E_DEATH_FRAMES;  break;
        }
        if (e.state == Enemy::DEAD || e.state == Enemy::HURT) {
            if (e.currentFrame < eTF - 1) e.currentFrame++;
            else if (e.state == Enemy::DEAD) e.dead = true;
            else { e.state = Enemy::IDLE; e.currentFrame = 0; }
        }
        else { e.currentFrame = (e.currentFrame + 1) % eTF; }
    }
    sf::Texture* texSets[] = { eTexIdle, eTexWalk, eTexAttack, eTexHurt, eTexDeath };
    int stateIdx = (int)e.state;
    if (stateIdx >= 0 && stateIdx <= 4)
        e.sprite.setTexture(*(texSets[stateIdx] + e.currentFrame));

    if (e.flashTimer > 0.f) {
        e.flashTimer -= dt;
        sf::Uint8 fl = (sf::Uint8)(255 * (e.flashTimer / 0.3f));
        e.sprite.setColor(sf::Color(255, 255 - fl, 255 - fl, 255));
    }
    else { e.sprite.setColor(sf::Color::White); }

    e.sprite.setPosition(e.position);
}

#endif