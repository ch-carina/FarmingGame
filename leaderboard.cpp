/*----------------------------------------------------
 HEADER COMMENT

 leaderboard.cpp
 Tracks the player's best 5 attempts (level reached + money),
 persisted to a small text file between sessions.
                                     Author: Carina Chao
								     Date: 2026/09/06
 ----------------------------------------------------*/
#include "leaderboard.h"
#include <fstream>

static const char* LEADERBOARD_FILE = "leaderboard.txt";

static LeaderboardEntry g_Entries[LEADERBOARD_MAX_ENTRIES];
static int g_EntryCount = 0;

static bool IsBetter(const LeaderboardEntry& a, const LeaderboardEntry& b)
{
    if (a.levelReached != b.levelReached) return a.levelReached > b.levelReached;
    return a.money > b.money;
}

static void Save()
{
    std::ofstream ofs(LEADERBOARD_FILE);
    if (!ofs) return;

    for (int i = 0; i < g_EntryCount; i++)
    {
        ofs << (int)g_Entries[i].levelReached << " " << g_Entries[i].money << "\n";
    }
}

void Leaderboard_Initialize()
{
    g_EntryCount = 0;

    std::ifstream ifs(LEADERBOARD_FILE);
    if (!ifs) return;

    int levelValue, money;
    while (g_EntryCount < LEADERBOARD_MAX_ENTRIES && (ifs >> levelValue >> money))
    {
        g_Entries[g_EntryCount].levelReached = (LevelType)levelValue;
        g_Entries[g_EntryCount].money = money;
        g_EntryCount++;
    }
}

void Leaderboard_RecordAttempt(LevelType levelReached, int money)
{
    LeaderboardEntry newEntry{ levelReached, money };

    int insertPos = g_EntryCount;
    for (int i = 0; i < g_EntryCount; i++)
    {
        if (IsBetter(newEntry, g_Entries[i]))
        {
            insertPos = i;
            break;
        }
    }

    if (insertPos >= LEADERBOARD_MAX_ENTRIES) return; // didn't make the cut

    int lastIndex = (g_EntryCount < LEADERBOARD_MAX_ENTRIES) ? g_EntryCount : LEADERBOARD_MAX_ENTRIES - 1;
    for (int i = lastIndex; i > insertPos; i--)
    {
        g_Entries[i] = g_Entries[i - 1];
    }
    g_Entries[insertPos] = newEntry;

    if (g_EntryCount < LEADERBOARD_MAX_ENTRIES) g_EntryCount++;

    Save();
}

int Leaderboard_GetCount()
{
    return g_EntryCount;
}

const LeaderboardEntry& Leaderboard_GetEntry(int index)
{
    return g_Entries[index];
}