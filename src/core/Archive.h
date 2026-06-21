#pragma once

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

template <typename BitboardType>
class ThreadSafeArchive {
private:
    std::mutex mtx;
    SharedLeaderboard<BitboardType> leaderboard;
    std::vector<AlignedCounter> threadIterations;
    std::vector<LeaderboardEntry<BitboardType>> topTen;

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

    void submit(int threadId, const BitboardType& cells, const BitboardType& walls, int ticks, int mana) {
        std::lock_guard<std::mutex> lock(mtx);

        leaderboard.threads[threadId].ticks = ticks;
        leaderboard.threads[threadId].mana = mana;

        double dist = BitboardHandler::getFigureDistance(cells);
        for (const auto& e : topTen) {
            if (e.mana == mana && std::abs(BitboardHandler::getFigureDistance(e.cells) - dist) < 0.001) {
                return;
            }
        }

        LeaderboardEntry<BitboardType> entry;
        entry.cells = cells;
        entry.walls = walls;
        entry.ticks = ticks;
        entry.mana = mana;

        topTen.push_back(entry);
        std::sort(topTen.begin(), topTen.end(), [](const LeaderboardEntry<BitboardType>& a, const LeaderboardEntry<BitboardType>& b) {
            if (a.mana != b.mana) return a.mana > b.mana;
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

    bool getElite(BitboardType& destCells, BitboardType& destWalls, int randomIndex) {
        std::lock_guard<std::mutex> lock(mtx);
        if (topTen.empty()) return false;

        int idx = randomIndex % static_cast<int>(topTen.size());
        destCells = topTen[idx].cells;
        destWalls = topTen[idx].walls;
        return true;
    }

    std::vector<LeaderboardEntry<BitboardType>> getTopTen() {
        std::lock_guard<std::mutex> lock(mtx);
        return topTen;
    }

    SharedLeaderboard<BitboardType> getSnapshot() {
        std::lock_guard<std::mutex> lock(mtx);
        for (size_t i = 0; i < threadIterations.size(); ++i) {
            leaderboard.threads[i].iterations = threadIterations[i].value.load(std::memory_order_relaxed);
        }
        return leaderboard;
    }
};