#pragma once
#define NOMINMAX
#include <vector>
#include <random>
#include <cmath>
#include <algorithm>

#include "Bitboard.h"


class BitboardHandler {
private:
    static inline uint32_t reverse25(uint32_t x) {
        x = ((x >> 1) & 0x55555555) | ((x & 0x55555555) << 1);
        x = ((x >> 2) & 0x33333333) | ((x & 0x33333333) << 2);
        x = ((x >> 4) & 0x0F0F0F0F) | ((x & 0x0F0F0F0F) << 4);
        x = ((x >> 8) & 0x00FF00FF) | ((x & 0x00FF00FF) << 8);
        x = ((x >> 16) & 0x0000FFFF) | ((x & 0x0000FFFF) << 16);
        return x >> 7;
    }

public:
    static void spawnTSpark(Bitboard_25& board, int x, int y) {
        board.setCell(x, y, true);
        board.setCell(x + 1, y, true);
        board.setCell(x + 2, y, true);
        board.setCell(x + 1, y + 2, true);
    }

    static void spawnTSparkSymmetric(Bitboard_25& board, int y, bool pointingUp) {
        int x = 11;
        if (pointingUp) {
            board.setCell(x, y, true);
            board.setCell(x + 1, y, true);
            board.setCell(x + 2, y, true);
            board.setCell(x + 1, y + 2, true);
        }
        else {
            board.setCell(x + 1, y, true);
            board.setCell(x, y + 2, true);
            board.setCell(x + 1, y + 2, true);
            board.setCell(x + 2, y + 2, true);
        }
    }

    static double getFigureDistance(const Bitboard_25& cells) {
        double sumX = 0, sumY = 0;
        int count = 0;
        for (int y = 1; y <= 25; ++y) {
            uint32_t row = cells.data[y];
            if (!row) continue;
            for (int x = 0; x < 25; ++x) {
                if (row & (1U << x)) {
                    sumX += x;
                    sumY += y;
                    count++;
                }
            }
        }
        if (count == 0) return 0.0;
        double cx = sumX / count;
        double cy = sumY / count;
        return std::sqrt((cx - 12.0) * (cx - 12.0) + (cy - 13.0) * (cy - 13.0));
    }

    static void translate(const Bitboard_25& src, Bitboard_25& dest, int dx, int dy) {
        dest.clear();
        for (int y = 1; y <= 25; ++y) {
            int target_y = y + dy;
            if (target_y < 1 || target_y > 25) continue;

            uint32_t row = src.data[y];
            if (dx > 0) {
                dest.data[target_y] = (row << dx) & 0x1FFFFFF;
            }
            else if (dx < 0) {
                dest.data[target_y] = (row >> -dx);
            }
            else {
                dest.data[target_y] = row;
            }
        }
    }

    static void mirrorHorizontal(const Bitboard_25& src, Bitboard_25& dest) {
        dest.clear();
        for (int y = 1; y <= 25; ++y) {
            dest.data[y] = reverse25(src.data[y]);
        }
    }

    static void mirrorVertical(const Bitboard_25& src, Bitboard_25& dest) {
        dest.clear();
        for (int y = 1; y <= 25; ++y) {
            dest.data[y] = src.data[26 - y];
        }
    }

    static void rotate90(const Bitboard_25& src, Bitboard_25& dest) {
        dest.clear();
        for (int y = 1; y <= 25; ++y) {
            uint32_t row = src.data[y];
            if (!row) continue;

            for (int x = 0; x < 25; ++x) {
                if (row & (1U << x)) {
                    dest.setCell(24 - (y - 1), x + 1, true);
                }
            }
        }
    }

    static void mutateWallsOnFootprint(Bitboard_25& obstacles, const Bitboard_25& footstep, const Bitboard_25& startCells, int numMutations) {
        double startSumX = 0, startSumY = 0;
        int startCount = 0;
        for (int y = 1; y <= 25; ++y) {
            uint32_t row = startCells.data[y];
            if (!row) continue;
            for (int x = 0; x < 25; ++x) {
                if (row & (1U << x)) {
                    startSumX += x;
                    startSumY += y;
                    startCount++;
                }
            }
        }
        if (startCount == 0) return;
        double scx = startSumX / startCount;
        double scy = startSumY / startCount;

        std::vector<std::pair<int, int>> activeCoords;
        std::vector<double> distances;
        activeCoords.reserve(250);
        distances.reserve(250);

        for (int y = 1; y <= 25; ++y) {
            uint32_t row = footstep.data[y];
            if (!row) continue;
            for (int x = 0; x < 25; ++x) {
                if (row & (1U << x)) {
                    activeCoords.push_back({ x, y });
                    double dx = x - scx;
                    double dy = y - scy;
                    distances.push_back(std::sqrt(dx * dx + dy * dy));
                }
            }
        }
        if (activeCoords.empty()) 
            return;

        std::vector<double> cumulativeWeights(activeCoords.size());
        double sumWeights = 0.0;
        for (size_t i = 0; i < activeCoords.size(); ++i) {
            double w = distances[i] * distances[i];
            sumWeights += w;
            cumulativeWeights[i] = sumWeights;
        }

        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<double> dis(0.0, sumWeights);

        for (int i = 0; i < numMutations; ++i) {
            double roll = dis(gen);
            auto it = std::lower_bound(cumulativeWeights.begin(), cumulativeWeights.end(), roll);
            size_t idx = std::distance(cumulativeWeights.begin(), it);
            if (idx >= activeCoords.size()) idx = activeCoords.size() - 1;

            auto [x, y] = activeCoords[idx];
            bool currentWall = obstacles.getCell(x, y);
            obstacles.setCell(x, y, !currentWall);
        }
    }

    static void mutateWallsSymmetric(Bitboard_25& obstacles, const Bitboard_25& footstep, int numMutations) {
        std::vector<std::pair<int, int>> activeCoords;
        activeCoords.reserve(150);

        for (int y = 1; y <= 25; ++y) {
            uint32_t row = footstep.data[y];
            if (!row) continue;
            for (int x = 0; x <= 12; ++x) {
                if (row & (1U << x)) {
                    activeCoords.push_back({ x, y });
                }
            }
        }

        if (activeCoords.empty()) return;

        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, static_cast<int>(activeCoords.size() - 1));

        for (int i = 0; i < numMutations; ++i) {
            int idx = dis(gen);
            auto [x, y] = activeCoords[idx];

            bool currentWall = obstacles.getCell(x, y);
            obstacles.setCell(x, y, !currentWall);
            obstacles.setCell(24 - x, y, !currentWall);
        }
    }
};