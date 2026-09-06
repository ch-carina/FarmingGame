#pragma once
#ifndef LEADERBOARD_H
#define LEADERBOARD_H
#include "level.h"

struct LeaderboardEntry
{
    LevelType levelReached;
    int money;
};

static constexpr int LEADERBOARD_MAX_ENTRIES = 5;

void Leaderboard_Initialize(); // loads existing entries from disk
void Leaderboard_RecordAttempt(LevelType levelReached, int money); // inserts if it makes the top 5, saves to disk

int Leaderboard_GetCount();
const LeaderboardEntry& Leaderboard_GetEntry(int index); // 0 = best

#endif //LEADERBOARD_H