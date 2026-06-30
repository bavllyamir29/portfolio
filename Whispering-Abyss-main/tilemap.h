#pragma once
#ifndef TILEMAP_H
#define TILEMAP_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "structs.h"
using namespace std;

string getTileType(int gid, const vector<TilesetInfo*>& tilesets)
{
    if (gid == 0) return "background";
    for (TilesetInfo* ts : tilesets)
        if (gid >= ts->firstGid && gid <= ts->lastGid)
            return ts->type;
    return "ground";
}

bool isSolid(int gid, const vector<TilesetInfo*>& tilesets)
{
    string t = getTileType(gid, tilesets);
    return (t == "ground" || t == "obstacle");
}

bool isObstacle(int gid, const vector<TilesetInfo*>& tilesets)
{
    return getTileType(gid, tilesets) == "obstacle";
}

bool isCheckpoint(int gid, const vector<TilesetInfo*>& tilesets)
{
    return getTileType(gid, tilesets) == "checkpoint";
}

bool isColliding(const Player& p, const sf::FloatRect& tileBounds)
{
    sf::FloatRect playerBounds(p.x, p.y, p.width, p.height);
    return playerBounds.intersects(tileBounds);
}

#endif #pragma once
