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

#include "../core/Types.h"
#include "../core/BitboardHandler.h"

template <typename BitboardType>
class ConsoleUI {
private:
    uint64_t lastTotalIterations = 0;
    std::chrono::steady_clock::time_point lastTime;
    int lastSavedBestMana = -1;

    const std::string COLOR_RESET = "\033[0m";
    const std::string COLOR_GREEN = "\033[32m";
    const std::string COLOR_YELLOW = "\033[33m";
    const std::string COLOR_CYAN = "\033[36m";
    const std::string ERASE_LINE_END = "\033[K";

    void savePatternToFile(const BitboardType& cells, const BitboardType& walls) {
        std::ofstream out("pattern.txt");
        if (!out.is_open()) return;

        for (int y = 1; y <= BitboardType::HEIGHT; ++y) {
            for (int x = 0; x < BitboardType::WIDTH; ++x) {
                if (y == BitboardType::CENTER_Y && x == BitboardType::CENTER_X) {
                    out << "F ";
                }
                else if (walls.getCell(x, y)) {
                    out << "W ";
                }
                else if (cells.getCell(x, y)) {
                    out << "C ";
                }
                else {
                    out << ". ";
                }
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

    void render(const SharedLeaderboard<BitboardType>& leaderboard, const std::vector<LeaderboardEntry<BitboardType>>& topTen) {
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

        ss << "---------------------------------" << ERASE_LINE_END << "\n";
        ss << " Stream  |  Ticks   |  Mana" << ERASE_LINE_END << "\n";
        ss << "---------------------------------" << ERASE_LINE_END << "\n";
        for (const auto& t : leaderboard.threads) {
            ss << " S" << t.threadId << "      |  "
                << t.ticks << (t.ticks < 10 ? "        |  " : (t.ticks < 100 ? "       |  " : "      |  "))
                << t.mana << ERASE_LINE_END << "\n";
        }
        ss << "---------------------------------\n" << ERASE_LINE_END;

        ss << "          TOP-10 ARCHIVE         " << ERASE_LINE_END << "\n";
        ss << "---------------------------------" << ERASE_LINE_END << "\n";
        ss << " Rank  |  Ticks  |  Mana  |  Dist" << ERASE_LINE_END << "\n";
        ss << "---------------------------------" << ERASE_LINE_END << "\n";

        size_t displayCount = std::min(static_cast<size_t>(10), topTen.size());
        for (size_t i = 0; i < displayCount; ++i) {
            const auto& entry = topTen[i];
            ss << COLOR_CYAN << " #" << (i + 1) << (i + 1 < 10 ? "    |  " : "   |  ")
                << entry.ticks << (entry.ticks < 10 ? "       |  " : (entry.ticks < 100 ? "      |  " : "     |  "))
                << entry.mana << (entry.mana < 1000 ? "    |  " : (entry.mana < 10000 ? "   |  " : "  |  "))
                << std::fixed << std::setprecision(1) << BitboardHandler::getFigureDistance(entry.cells) << COLOR_RESET << ERASE_LINE_END << "\n";
        }
        ss << "---------------------------------" << ERASE_LINE_END << "\n";

        ss << " Total Operations: " << COLOR_GREEN << totalIterations << COLOR_RESET << ERASE_LINE_END << "\n";
        ss << " Operations/Sec:   " << COLOR_YELLOW << static_cast<uint64_t>(iterationsPerSecond) << " O/s" << COLOR_RESET << "\n";
        ss << "---------------------------------" << ERASE_LINE_END << "\n";

        std::cout << ss.str() << std::flush;
    }
};