#pragma once
#define NOMINMAX
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <chrono>
#include <fstream>

#include "../core/Bitboard.h"


struct ThreadStatus {
    int threadId = 0;
    int ticks = 0;
    int mana = 0;
    uint64_t iterations = 0;
};

struct SharedLeaderboard {
    std::vector<ThreadStatus> threads;
    Bitboard_25 bestCells;
    Bitboard_25 bestWalls;
    int bestMana = 0;
};

class ConsoleUI {
private:
    uint64_t lastTotalIterations = 0;
    std::chrono::steady_clock::time_point lastTime;
    int lastSavedBestMana = -1;

    const std::string COLOR_RESET = "\033[0m";
    const std::string COLOR_GREEN = "\033[32m";
    const std::string COLOR_YELLOW = "\033[33m";

    void savePatternToFile(const Bitboard_25& cells, const Bitboard_25& walls) {
        std::ofstream out("pattern.txt");
        if (!out.is_open()) return;

        for (int y = 1; y <= 25; ++y) {
            for (int x = 0; x < 25; ++x) {
                if (y == 13 && x == 12)         { out << "F "; }
                else if (walls.getCell(x, y))   { out << "W "; }
                else if (cells.getCell(x, y))   { out << "C "; }
                else                            { out << ". "; }
            }
            out << "\n";
        }
        out.close();
    }

public:
    ConsoleUI() {
        std::cout << "\033[?25l" << std::flush;
        lastTime = std::chrono::steady_clock::now();
    }

    ~ConsoleUI() {
        std::cout << "\033[?25h" << std::flush;
    }

    void render(const SharedLeaderboard& leaderboard) {
        auto currentTime = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed = currentTime - lastTime;
        lastTime = currentTime;

        if (leaderboard.bestMana > lastSavedBestMana) {
            savePatternToFile(leaderboard.bestCells, leaderboard.bestWalls);
            lastSavedBestMana = leaderboard.bestMana;
        }

        uint64_t totalIterations = 0;
        for (const auto& t : leaderboard.threads) {
            totalIterations += t.iterations;
        }

        double iterationsPerSecond = 0.0;
        if (elapsed.count() > 0.0) {
            iterationsPerSecond = (totalIterations - lastTotalIterations) / elapsed.count();
        }
        lastTotalIterations = totalIterations;

        std::stringstream ss;
        ss << "\033[H"; 

        ss << "=================================\n";
        ss << " Stream  |  Ticks  |  Mana\n";
        ss << "---------------------------------\n";
        for (const auto& t : leaderboard.threads) {
            ss << " S" << t.threadId << "      |  "
                << (t.ticks < 10 ? "0" : "") << t.ticks << "     |  "
                << t.mana << "\n";
        }
        ss << "=================================\n";

        ss << " Total Operations: " << COLOR_GREEN << totalIterations << COLOR_RESET << "\n";
        ss << " Operations/Sec:   " << COLOR_YELLOW << static_cast<uint64_t>(iterationsPerSecond) << COLOR_RESET << "\n";
        ss << "=================================\n";

        std::cout << ss.str() << std::flush;
    }
};