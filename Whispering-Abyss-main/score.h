#pragma once
#pragma once
#ifndef SCORE_H
#define SCORE_H

#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include "structs.h"
using namespace std;

string formatTime(float t)
{
    int mins = (int)t / 60;
    int secs = (int)t % 60;
    int ms = (int)((t - (int)t) * 100);
    char buf[32];
    snprintf(buf, sizeof(buf), "%02d:%02d.%02d", mins, secs, ms);
    return string(buf);
}

vector<ScoreEntry> loadScores()
{
    vector<ScoreEntry> scores;
    ifstream file("scores.txt");
    string line;
    while (getline(file, line)) {
        size_t comma = line.rfind(',');
        if (comma == string::npos) continue;
        scores.push_back({ line.substr(0, comma), stof(line.substr(comma + 1)) });
    }
    sort(scores.begin(), scores.end(),
        [](const ScoreEntry& a, const ScoreEntry& b) { return a.time < b.time; });
    return scores;
}

void saveScore(const string& name, float t)
{
    ofstream file("scores.txt", ios::app);
    file << name << "," << t << "\n";
}

#endif 