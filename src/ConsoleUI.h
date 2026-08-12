#pragma once

#define NOMINMAX
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include "Types.h"
#include "BitboardHandler.h"


class ConsoleUI {
private:
    uint64_t lastTotalIterations = 0;
    std::chrono::steady_clock::time_point lastTime;
    int lastSavedBestMana = -1;
    int lastSavedBestWallsCount = 9999;

    const std::string COLOR_RESET  = "\033[0m";
    const std::string COLOR_GREEN  = "\033[32m";
    const std::string COLOR_YELLOW = "\033[33m";
    const std::string COLOR_CYAN   = "\033[36m";
    const std::string ERASE_LINE_END = "\033[K";

    std::string formatNumber(uint64_t num) const {
        std::string s = std::to_string(num);
        std::string result;
        int count = 0;
        for (int i = static_cast<int>(s.length()) - 1; i >= 0; --i) {
            result.push_back(s[i]);
            count++;
            if (count % 3 == 0 && i != 0) {
                result.push_back(' ');
            }
        }
        std::reverse(result.begin(), result.end());
        return result;
    }

    void savePatternToFile(const Bitboard& cells, const Bitboard& walls) {
        std::ofstream out("best_result.txt");
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

    void render(const SharedLeaderboard& leaderboard, const std::vector<LeaderboardEntry>& topTen, const SimulatorConfig& config) {
        auto currentTime = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed = currentTime - lastTime;
        lastTime = currentTime;

       int currentWallsCount = leaderboard.bestWalls.popcount();
        bool shouldSave = false;

        if (leaderboard.bestMana > lastSavedBestMana) {
            shouldSave = true;
        } else if (leaderboard.bestMana == lastSavedBestMana) {
            if (currentWallsCount < lastSavedBestWallsCount) {
                shouldSave = true;
            }
        }

        if (shouldSave) {
            savePatternToFile(leaderboard.bestCells, leaderboard.bestWalls);
            lastSavedBestMana = leaderboard.bestMana;
            lastSavedBestWallsCount = currentWallsCount;
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

        ss << "_________________________________\n" << ERASE_LINE_END;
        ss << " STREAM  |  TICKS    |  MANA" << ERASE_LINE_END << "\n";
        for (const auto& t : leaderboard.threads) {
            ss << " S" << t.threadId << "      |  "
               << t.ticks << (t.ticks < 10 ? "        |  " : (t.ticks < 100 ? "       |  " : "      |  "))
               << formatNumber(t.mana) << ERASE_LINE_END << "\n";
        }
        ss << "_________________________________\n" << ERASE_LINE_END;

        ss << "         ABSOLUTE RECORD         " << ERASE_LINE_END << "\n";
        
        if (leaderboard.bestMana > 0) {
            int absorbedCells = leaderboard.bestMana / (leaderboard.bestTicks * 60);
            std::string successTag = (leaderboard.bestMana >= 6 * config.maxTicks * config.manaPerCell) ? (" " + this->COLOR_GREEN + "[SUCCESS]" + this->COLOR_RESET) : "";

            ss << " Ticks:  " << leaderboard.bestTicks << ERASE_LINE_END << "\n";
            ss << " Cells:  " << absorbedCells << ERASE_LINE_END << "\n";
            ss << " Mana:   " << COLOR_CYAN << formatNumber(leaderboard.bestMana) << COLOR_RESET << successTag << ERASE_LINE_END << "\n";
        } else {
            ss << " No records found yet..." << ERASE_LINE_END << "\n";
        }
        ss << "_________________________________\n" << ERASE_LINE_END;

        ss << " Total simulations:  " << COLOR_GREEN << formatNumber(totalIterations) << COLOR_RESET << ERASE_LINE_END << "\n";
        ss << " Speed:              " << COLOR_YELLOW << formatNumber(static_cast<uint64_t>(iterationsPerSecond)) << COLOR_RESET << " O/s" << ERASE_LINE_END << "\n";

        std::cout << ss.str() << std::flush;
    }
};