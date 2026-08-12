#pragma once

#define NOMINMAX
#include <mutex>
#include <vector>
#include <atomic>
#include <algorithm>
#include <cmath>
#include "Types.h"
#include "BitboardHandler.h"


struct alignas(64) AlignedCounter {
    std::atomic<uint64_t> value{ 0 };
};

class ThreadSafeArchive {
private:
    std::mutex mtx;
    SharedLeaderboard leaderboard;
    std::vector<AlignedCounter> threadIterations;
    std::vector<LeaderboardEntry> topTen;

public:
    ThreadSafeArchive(int threadCount) : threadIterations(threadCount) {
        leaderboard.threads.resize(threadCount);
        for (int i = 0; i < threadCount; ++i) {
            leaderboard.threads[i].threadId = i;
        }
    }

    void incrementIterations(int threadId) {
        threadIterations[threadId].value.fetch_add(1, std::memory_order_relaxed);
    }

    void submit(int threadId, const Bitboard& cells, const Bitboard& walls, int ticks, int mana) {
        std::lock_guard<std::mutex> lock(mtx);

        leaderboard.threads[threadId].ticks = ticks;
        leaderboard.threads[threadId].mana = mana;

        double dist = BitboardHandler::getFigureDistance(cells);
        for (const auto& e : topTen) {
            if (e.mana == mana && std::abs(BitboardHandler::getFigureDistance(e.cells) - dist) < 0.001) {
                return;
            }
        }

        LeaderboardEntry entry;
        entry.cells = cells;
        entry.walls = walls;
        entry.ticks = ticks;
        entry.mana = mana;

        topTen.push_back(entry);
        
        std::sort(topTen.begin(), topTen.end(), [](const LeaderboardEntry& a, const LeaderboardEntry& b) {
            if (a.mana != b.mana) return a.mana > b.mana;
            
            int wallsA = a.walls.popcount();
            int wallsB = b.walls.popcount();
            if (wallsA != wallsB) return wallsA < wallsB;
            
            return BitboardHandler::getFigureDistance(a.cells) > BitboardHandler::getFigureDistance(b.cells);
        });

        if (topTen.size() > 10) {
            topTen.resize(10);
        }

        if (!topTen.empty()) {
            leaderboard.bestMana = topTen[0].mana;
            leaderboard.bestTicks = topTen[0].ticks;
            leaderboard.bestCells = topTen[0].cells;
            leaderboard.bestWalls = topTen[0].walls;
        }
    }

    bool getElite(Bitboard& destCells, Bitboard& destWalls, int randomIndex) {
        std::lock_guard<std::mutex> lock(mtx);
        if (topTen.empty()) return false;

        int idx = randomIndex % static_cast<int>(topTen.size());
        destCells = topTen[idx].cells;
        destWalls = topTen[idx].walls;
        return true;
    }

    bool getWorstElite(Bitboard& destCells, Bitboard& destWalls) {
        std::lock_guard<std::mutex> lock(mtx);
        if (topTen.empty()) return false;

        int idx = static_cast<int>(topTen.size()) - 1;
        destCells = topTen[idx].cells;
        destWalls = topTen[idx].walls;
        return true;
    }

    std::vector<LeaderboardEntry> getTopTen() {
        std::lock_guard<std::mutex> lock(mtx);
        return topTen;
    }

    SharedLeaderboard getSnapshot() {
        std::lock_guard<std::mutex> lock(mtx);
        for (size_t i = 0; i < threadIterations.size(); ++i) {
            leaderboard.threads[i].iterations = threadIterations[i].value.load(std::memory_order_relaxed);
        }
        return leaderboard;
    }
};