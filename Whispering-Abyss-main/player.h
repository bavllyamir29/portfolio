#ifndef PLAYER_H
#define PLAYER_H

#include <SFML/Graphics.hpp>
#include <vector>
#include "structs.h"
#include "constants.h"
#include "tilemap.h"
using namespace std;

void damagePlayer(Player& player, int damage, bool& gameOver)
{
    if (player.health <= 0) return;
    player.health -= damage;
    player.damaged = true;         
    if (player.health <= 0) { 
        player.health = 0; 
        gameOver = true; 
    }
}

void InputUpdate(Player& p, int& playerFacing)
{
    p.vx = 0.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
        p.vx = p.moveSpeed; playerFacing = 1;
        p.sprite.setOrigin(0, 0);
        p.sprite.setScale(p.PLAYER_SCALE, p.PLAYER_SCALE);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
        p.vx = -p.moveSpeed; playerFacing = -1;
        p.sprite.setOrigin(FRAME_W, 0);
        p.sprite.setScale(-p.PLAYER_SCALE, p.PLAYER_SCALE);
    }
    p.jumpHeld = sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
    if (p.jumpHeld) p.jumpBuffer = 0.1f;
}

void PlayerPhysicsUpdate(Player& p, float dt,
    vector<int>& tiles, int mapWidth,
    int tileWidth, int tileHeight,
    const vector<TilesetInfo*>& tilesets,
    float playerHitboxWidth)
{
    p.coyoteTimer -= dt;
    p.jumpBuffer -= dt;
    if (p.onGround) p.coyoteTimer = p.coyoteTime;

    if (p.jumpBuffer > 0 && p.coyoteTimer > 0) {
        p.vy = -p.jumpSpeed;
        p.onGround = false;
        p.coyoteTimer = p.jumpBuffer = 0;
    }

    float gravMul = (p.vy < 0) ? 1.0f : 1.4f;
    p.vy += p.gravity * gravMul * dt;
    if (!p.jumpHeld && p.vy < 0) p.vy += p.gravity * 1.5f * dt;

    if (p.vy > MAX_FALL_SPEED) p.vy = MAX_FALL_SPEED;

    p.x += p.vx * dt;
    {
        int leftCol = (int)((p.x + playerHitboxOffsetX) / tileWidth);
        int rightCol = (int)((p.x + playerHitboxOffsetX + playerHitboxWidth - 0.1f) / tileWidth);
        int topRow = (int)(p.y / tileHeight);
        int botRow = (int)((p.y + p.height - 0.1f) / tileHeight);

        for (int row = topRow; row <= botRow; row++)
            for (int col = leftCol; col <= rightCol; col++) {
                int idx = row * mapWidth + col;
                if (idx >= 0 && idx < (int)tiles.size() && tiles[idx] && isSolid(tiles[idx], tilesets)) {
                    if (p.vx > 0)      p.x = col * tileWidth - playerHitboxOffsetX - playerHitboxWidth;
                    else if (p.vx < 0) p.x = (col + 1) * tileWidth - playerHitboxOffsetX;
                    p.vx = 0; break;
                }
            }
    }

    p.y += p.vy * dt;
    p.onGround = false;
    {
        int leftCol = (int)((p.x + playerHitboxOffsetX) / tileWidth);
        int rightCol = (int)((p.x + playerHitboxOffsetX + playerHitboxWidth - 0.1f) / tileWidth);
        int topTile = (int)(p.y / tileHeight);
        int botTile = (int)((p.y + p.height) / tileHeight);

        for (int col = leftCol; col <= rightCol; col++) {
            int idxBot = botTile * mapWidth + col;
            int idxTop = topTile * mapWidth + col;
            if (idxBot >= 0 && idxBot < (int)tiles.size() && tiles[idxBot] &&
                isSolid(tiles[idxBot], tilesets) && p.vy >= 0) {
                p.y = botTile * tileHeight - p.height;
                p.vy = 0; p.onGround = true;
            }
            if (idxTop >= 0 && idxTop < (int)tiles.size() && tiles[idxTop] &&
                isSolid(tiles[idxTop], tilesets) && p.vy < 0) {
                p.y = (topTile + 1) * tileHeight; p.vy = 0;
            }
        }
    }

    if (p.onGround) {
        int leftCol = (int)((p.x + playerHitboxOffsetX) / tileWidth);
        int rightCol = (int)((p.x + playerHitboxOffsetX + playerHitboxWidth - 0.1f) / tileWidth);
        int feetRow = (int)((p.y + p.height + 1.f) / tileHeight);
        for (int col = leftCol; col <= rightCol; col++) {
            int idx = feetRow * mapWidth + col;
            if (idx >= 0 && idx < (int)tiles.size() && isObstacle(tiles[idx], tilesets))
                p.damaged = false;
        }
    }

    p.sprite.setPosition(p.x, p.y);
}

void drawHearts(sf::RenderWindow& window, const Player& player,
    const sf::Texture* heartTexture)
{
    const float heartDisplaySize = 40.f;
    const float spacing = 8.f;
    for (int i = 0; i < player.maxHealth; i++) {
        sf::Sprite heartSprite(*heartTexture);
        auto texSize = heartTexture->getSize();
        heartSprite.setScale((heartDisplaySize / texSize.x) * 2,
            (heartDisplaySize / texSize.y) * 2);
        heartSprite.setPosition(10.f + i * (heartDisplaySize + spacing), 10.f);
        heartSprite.setColor(i < player.health
            ? sf::Color(255, 255, 255, 255)
            : sf::Color(100, 100, 100, 150));
        window.draw(heartSprite);
    }
}

#endif