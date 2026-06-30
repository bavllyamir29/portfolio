#pragma once
#ifndef BIGE_H
#define BIGE_H

#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include "structs.h"
#include "player.h"
using namespace std;

bool loadBigE(BigE& boss, const string& path) {
    string base = path + "entity/";
    string       anims[] = { "ATTACK", "DIE", "HURT", "IDLE", "JUMP", "RUN", "WALK" };
    sf::Texture* arrays[] = {
        boss.texAttack, boss.texDie, boss.texHurt,
        boss.texIdle,   boss.texJump, boss.texRun, boss.texWalk
    };
    int frameCounts[] = { 10, 10, 10, 10, 10, 10, 10 };

    for (int a = 0; a < 7; a++)
        for (int f = 0; f < frameCounts[a]; f++) {
            char buf[8]; snprintf(buf, sizeof(buf), "%03d", f);
            string file = base + anims[a] + "_" + buf + ".png";
            if (!arrays[a][f].loadFromFile(file)) {
                cerr << "Failed to load " << file << "\n";
                return false;
            }
        }
    boss.sprite.setTexture(boss.texIdle[0]);
    boss.width = 200.f;
    boss.height = 125.f;
    return true;
}

void setBigEFrame(BigE* boss) {
    const float FEET_OFFSET = -20.f;
    sf::Texture* tex = boss->texIdle;
    switch (boss->state) {
    case BigE::IDLE:   tex = boss->texIdle;   break;
    case BigE::WALK:   tex = boss->texWalk;   break;
    case BigE::ATTACK: tex = boss->texAttack; break;
    case BigE::CHARGE: tex = boss->texRun;    break;
    case BigE::HURT:   tex = boss->texHurt;   break;
    case BigE::DEAD:   tex = boss->texDie;    break;
    }
    int f = boss->currentFrame;
    if (f < 0) f = 0;

    const float scaleX = 200.f / 1600.f;
    const float scaleY = 125.f / 1000.f;
    boss->sprite.setTexture(tex[f]);

    if (boss->direction == 1) {
        boss->sprite.setOrigin(0.f, FEET_OFFSET / scaleY);
        boss->sprite.setScale(scaleX, scaleY);
    }
    else {
        boss->sprite.setOrigin(1600.f, FEET_OFFSET / scaleY);
        boss->sprite.setScale(-scaleX, scaleY);
    }
}

void resetBigE(BigE* boss, float spawnX, float spawnY) {
    boss->health = boss->maxHealth;
    boss->dead = false;
    boss->dying = false;
    boss->isCharging = false;
    boss->state = BigE::IDLE;
    boss->currentFrame = 0;
    boss->animTimer = 0.f;
    boss->attackTimer = 0.f;
    boss->chargeTimer = 0.f;
    boss->chargeClock = 0.f;
    boss->flashTimer = 0.f;
    boss->attackHitDealt = false;
    boss->chargeHitDealt = false;
    boss->vx = boss->vy = 0.f;
    boss->x = spawnX;
    boss->y = spawnY;
    boss->sprite.setPosition(boss->x, boss->y);
}

static void bigEHorizontalCollision(BigE* boss,
    int mapWidth, int tileWidth, int tileHeight,
    const vector<int>& tiles, const vector<TilesetInfo*>& tilesets,
    float moveDir)
{
    int bTop = (int)(boss->y + boss->height * 0.1f) / tileHeight;
    int bBottom = (int)(boss->y + boss->height * 0.9f) / tileHeight;
    int bFront = (moveDir >= 0)
        ? (int)(boss->x + boss->width * 0.85f) / tileWidth
        : (int)(boss->x + boss->width * 0.15f) / tileWidth;

    for (int row = bTop; row <= bBottom; row++) {
        int idx = row * mapWidth + bFront;
        if (idx >= 0 && idx < (int)tiles.size() && isSolid(tiles[idx], tilesets)) {
            if (moveDir >= 0)
                boss->x = bFront * tileWidth - boss->width * 0.85f;
            else
                boss->x = (bFront + 1) * tileWidth - boss->width * 0.15f;
            boss->vx = 0.f;
            if (boss->state == BigE::WALK) boss->direction *= -1;
            break;
        }
    }
}

void updateBigE(BigE* boss, Player& player, bool& gameOver,
    bool& canTakeDamage, float& damageTimer, float dt,
    int mapWidth, int tileWidth, int tileHeight,
    const vector<int>& tiles, const vector<TilesetInfo*>& tilesets)
{
    if (boss->dead) return;

    const float CHARGE_SPEED = 180.f;
    const float CHARGE_DURATION = 1.0f;
    const float CHARGE_ANIM_SPD = 0.10f;
    const float WALK_SPEED = 70.f;
    const float WALK_ANIM_SPD = 0.10f;
    const float ATTACK_ANIM_SPD = 0.10f;
    const float IDLE_ANIM_SPD = 0.12f;
    const float HURT_ANIM_SPD = 0.04f;
    const float DEAD_ANIM_SPD = 0.05f;

    boss->attackTimer += dt;
    boss->chargeTimer += dt;
    boss->flashTimer -= dt;
    if (boss->flashTimer < 0.f) boss->flashTimer = 0.f;

    float bCX = boss->x + boss->width * 0.5f;
    float pCX = player.x + player.width * 0.5f;
    float dx = pCX - bCX;
    float dist = fabsf(dx);

    if (boss->state != BigE::CHARGE && boss->state != BigE::HURT && !boss->dying)
        boss->direction = (dx > 0) ? 1 : -1;

    BigE::State prevState = boss->state;

    if (!boss->dying && boss->state != BigE::HURT)
    {
        if (boss->isCharging)
        {
            boss->chargeClock += dt;
            float chargeMove = boss->direction * CHARGE_SPEED * dt;
            boss->x += chargeMove;

            bigEHorizontalCollision(boss, mapWidth, tileWidth, tileHeight,
                tiles, tilesets, (float)boss->direction);
            if (boss->vx == 0.f && boss->isCharging) {
                boss->isCharging = false;
                boss->chargeHitDealt = false;
                boss->chargeClock = 0.f;
                boss->chargeTimer = 0.f;
                boss->state = BigE::IDLE;
            }

            const int CHARGE_HIT_FRAME_MIN = 5, CHARGE_HIT_FRAME_MAX = 8;
            bool inImpactWindow = (boss->currentFrame >= CHARGE_HIT_FRAME_MIN &&
                boss->currentFrame <= CHARGE_HIT_FRAME_MAX);

            sf::FloatRect bBox(boss->x + boss->width * 0.1f,
                boss->y + boss->height * 0.15f,
                boss->width * 0.8f, boss->height * 0.7f);
            sf::FloatRect pBox(player.x, player.y, player.width, player.height);

            if (bBox.intersects(pBox) && !boss->chargeHitDealt
                && canTakeDamage && inImpactWindow)
            {
                damagePlayer(player, 1, gameOver);
                canTakeDamage = false; damageTimer = 0.f;
                boss->chargeHitDealt = true;
                float knock = (player.x > boss->x) ? 1.f : -1.f;
                player.x += knock * 60.f; player.vy = -250.f;
            }

            if (boss->chargeClock >= CHARGE_DURATION) {
                boss->isCharging = false;
                boss->chargeHitDealt = false;
                boss->chargeClock = boss->chargeTimer = 0.f;
                boss->state = BigE::IDLE;
            }
        }
        else if (boss->state == BigE::ATTACK)
        {
            boss->vx = 0.f;
        }
        else if (dist < boss->aggroRange)
        {
            if (dist < boss->attackRange && boss->attackTimer >= boss->attackCooldown)
            {
                boss->state = BigE::ATTACK; boss->currentFrame = 0;
                boss->animTimer = 0.f; boss->attackTimer = 0.f;
                boss->attackHitDealt = false;
            }
            else if (dist > boss->attackRange && dist < boss->chargeRange
                && boss->chargeTimer >= boss->chargeCooldown)
            {
                boss->state = BigE::CHARGE; boss->currentFrame = 0;
                boss->animTimer = 0.f; boss->isCharging = true;
                boss->chargeClock = 0.f; boss->chargeHitDealt = false;
            }
            else if (dist > boss->attackRange)
            {
                if (boss->state != BigE::WALK) boss->state = BigE::WALK;
                boss->vx = boss->direction * WALK_SPEED;
                boss->x += boss->vx * dt;
                bigEHorizontalCollision(boss, mapWidth, tileWidth, tileHeight,
                    tiles, tilesets, boss->vx);
            }
            else
            {
                boss->vx = 0.f;
                if (boss->state != BigE::IDLE) boss->state = BigE::IDLE;
            }
        }
        else
        {
            boss->vx = 0.f;
            if (boss->state != BigE::IDLE) boss->state = BigE::IDLE;
        }
    }

    if (boss->state != prevState && !boss->dying) {
        boss->currentFrame = 0; boss->animTimer = 0.f;
    }

    float activeAnimSpeed = IDLE_ANIM_SPD;
    switch (boss->state) {
    case BigE::IDLE:   activeAnimSpeed = IDLE_ANIM_SPD;   break;
    case BigE::WALK:   activeAnimSpeed = WALK_ANIM_SPD;   break;
    case BigE::CHARGE: activeAnimSpeed = CHARGE_ANIM_SPD; break;
    case BigE::ATTACK: activeAnimSpeed = ATTACK_ANIM_SPD; break;
    case BigE::HURT:   activeAnimSpeed = HURT_ANIM_SPD;   break;
    case BigE::DEAD:   activeAnimSpeed = DEAD_ANIM_SPD;   break;
    }

    boss->animTimer += dt;
    if (boss->animTimer >= activeAnimSpeed)
    {
        boss->animTimer = 0.f;
        switch (boss->state)
        {
        case BigE::IDLE:
            boss->currentFrame = (boss->currentFrame + 1) % BigE::FRAMES_IDLE;
            break;
        case BigE::WALK:
            boss->currentFrame = (boss->currentFrame + 1) % BigE::FRAMES_WALK;
            break;
        case BigE::CHARGE:
            boss->currentFrame = (boss->currentFrame + 1) % BigE::FRAMES_RUN;
            break;
        case BigE::ATTACK:
        {
            if (boss->currentFrame < BigE::FRAMES_ATTACK - 1) {
                boss->currentFrame++;
                if (boss->currentFrame >= 4 && boss->currentFrame <= 6 && !boss->attackHitDealt) {
                    float swingOffX = (boss->direction == 1) ? boss->width * 0.7f : -boss->width * 0.5f;
                    sf::FloatRect swingBox(boss->x + swingOffX, boss->y + boss->height * 0.2f,
                        boss->width * 0.8f, boss->height * 0.6f);
                    sf::FloatRect pBox(player.x, player.y, player.width, player.height);
                    if (swingBox.intersects(pBox) && canTakeDamage) {
                        damagePlayer(player, 1, gameOver);
                        canTakeDamage = false; damageTimer = 0.f;
                        boss->attackHitDealt = true;
                        float knock = (player.x > boss->x) ? 1.f : -1.f;
                        player.x += knock * 50.f; player.vy = -200.f;
                    }
                }
            }
            else {
                boss->state = BigE::IDLE;
                boss->currentFrame = 0;
                boss->attackHitDealt = false;
            }
            break;
        }
        case BigE::HURT:
            if (boss->currentFrame < BigE::FRAMES_HURT - 1) boss->currentFrame++;
            else { boss->state = BigE::IDLE; boss->currentFrame = 0; }
            break;
        case BigE::DEAD:
            if (boss->currentFrame < BigE::FRAMES_DEATH - 1) boss->currentFrame++;
            else boss->dead = true;
            break;
        }
    }

    {
        sf::FloatRect bBox(boss->x + boss->width * 0.2f, boss->y + boss->height * 0.15f,
            boss->width * 0.6f, boss->height * 0.7f);
        sf::FloatRect pBox(player.x, player.y, player.width, player.height);
        if (bBox.intersects(pBox)) {
            float pushDir = (player.x + player.width * 0.5f > bCX) ? 1.f : -1.f;
            player.x += pushDir * 2.f;
            boss->x -= pushDir * 2.f;
        }
    }

    if (boss->flashTimer > 0.f) {
        sf::Uint8 f = (sf::Uint8)(255 * (boss->flashTimer / 0.3f));
        boss->sprite.setColor(sf::Color(255, 255 - f, 255 - f, 255));
    }
    else {
        boss->sprite.setColor(sf::Color::White);
    }

    boss->vy += 1200.f * dt;
    if (boss->vy > 900.f) boss->vy = 900.f;
    boss->y += boss->vy * dt;

    {
        int bLeft = (int)(boss->x + boss->width * 0.2f) / tileWidth;
        int bRight = (int)(boss->x + boss->width * 0.8f) / tileWidth;
        int bFeet = (int)((boss->y + boss->height) / tileHeight);
        int bHead = (int)(boss->y / tileHeight);

        for (int col = bLeft; col <= bRight; col++) {
            int idxBot = bFeet * mapWidth + col;
            if (idxBot >= 0 && idxBot < (int)tiles.size()
                && isSolid(tiles[idxBot], tilesets) && boss->vy >= 0) {
                boss->y = bFeet * tileHeight - boss->height;
                boss->vy = 0.f;
            }
            int idxTop = bHead * mapWidth + col;
            if (idxTop >= 0 && idxTop < (int)tiles.size()
                && isSolid(tiles[idxTop], tilesets) && boss->vy < 0) {
                boss->y = (bHead + 1) * tileHeight;
                boss->vy = 0.f;
            }
        }
    }

    setBigEFrame(boss);
    boss->sprite.setPosition(boss->x, boss->y);
}

void checkPlayerHitsBigE(BigE* boss, int currentFrame,
    int playerFacing, const Player& player)
{
    if (boss->dead || boss->dying || boss->state == BigE::HURT) return;

    static const FrameHitbox swordFrames[] = {
        { 3, 0.6f,  0.3f,  25.f, 20.f },
        { 4, 0.9f,  0.3f,  30.f, 20.f },
        { 5, 0.7f,  0.35f, 25.f, 20.f },
    };
    static const int SWORD_FRAME_COUNT = 3;

    for (int i = 0; i < SWORD_FRAME_COUNT; i++) {
        const FrameHitbox* fh = &swordFrames[i];
        if (currentFrame != fh->frame) continue;

        float sx = (playerFacing == 1)
            ? player.x + player.width * fh->offsetX
            : player.x + player.width * (1.f - fh->offsetX) - fh->w;
        float sy = player.y + player.height * fh->offsetY;

        sf::FloatRect swordBox(sx, sy, fh->w, fh->h);
        sf::FloatRect bossBox(boss->x + boss->width * 0.2f,
            boss->y + boss->height * 0.1f,
            boss->width * 0.6f, boss->height * 0.8f);

        if (swordBox.intersects(bossBox)) {
            boss->health--;
            boss->flashTimer = 0.3f;
            if (boss->health <= 0) {
                boss->dying = true; boss->state = BigE::DEAD;
                boss->currentFrame = 0; boss->animTimer = 0.f;
                boss->isCharging = false;
            }
            else {
                boss->state = BigE::HURT;
                boss->currentFrame = 0; boss->animTimer = 0.f;
            }
        }
        break;
    }
}

void drawBigEHealthBar(sf::RenderWindow& window, const BigE* boss,
    const sf::Font& font, const Player& player)
{
    if (boss->dead) return;
    float dx = (player.x + player.width * 0.5f) - (boss->x + boss->width * 0.5f);
    float dy = (player.y + player.height * 0.5f) - (boss->y + boss->height * 0.5f);
    if (sqrtf(dx * dx + dy * dy) > 700.f) return;

    const float barW = 300.f, barH = 22.f;
    const float barX = (1920.f - barW) * 0.5f;
    const float barY = 1080.f - 60.f;
    float ratio = max(0.f, (float)boss->health / (float)boss->maxHealth);
    float radius = barH * 0.5f;

    auto drawRoundedBar = [&](float w, sf::Color col) {
        if (w <= 0.f) return;
        float innerW = max(0.f, w - barH);
        sf::CircleShape capL(radius); capL.setFillColor(col); capL.setPosition(barX, barY);
        window.draw(capL);
        if (w >= barH) {
            sf::CircleShape capR(radius); capR.setFillColor(col);
            capR.setPosition(barX + w - barH, barY); window.draw(capR);
        }
        if (innerW > 0.f) {
            sf::RectangleShape mid(sf::Vector2f(innerW, barH));
            mid.setFillColor(col); mid.setPosition(barX + radius, barY); window.draw(mid);
        }
        };

    drawRoundedBar(barW, sf::Color(40, 0, 0, 220));
    drawRoundedBar(barW * ratio, sf::Color(220, 30, 30, 255));

    for (float ox : {-2.f, barW - barH - 2.f}) {
        sf::CircleShape oc(radius + 2.f);
        oc.setFillColor(sf::Color::Transparent);
        oc.setOutlineThickness(2.f); oc.setOutlineColor(sf::Color(200, 0, 0, 200));
        oc.setPosition(barX + ox, barY - 2.f); window.draw(oc);
    }
    sf::RectangleShape outlineMid(sf::Vector2f(barW - barH, barH + 4.f));
    outlineMid.setFillColor(sf::Color::Transparent);
    outlineMid.setOutlineThickness(2.f); outlineMid.setOutlineColor(sf::Color(200, 0, 0, 200));
    outlineMid.setPosition(barX + radius, barY - 2.f); window.draw(outlineMid);

    sf::Text label; label.setFont(font); label.setString("BIG E");
    label.setCharacterSize(20); label.setFillColor(sf::Color::White);
    sf::FloatRect lb = label.getLocalBounds();
    label.setOrigin(lb.width * 0.5f, 0.f);
    label.setPosition(barX + barW * 0.5f, barY - 28.f);
    window.draw(label);
}

#endif