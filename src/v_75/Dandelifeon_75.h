#pragma once

#define NOMINMAX
#include <algorithm>

#include "../core/ISimulation.h"
#include "Bitboard_75.h"
#include "Engine_75.h"


class DandelifeonSimulator_75 : public IDandelifeonSimulator<Bitboard_75> {
private:
    DandelifeonEngine_75 engine;

    const int maxTicks;
    const int manaPerCell;

    inline bool checkAbsorption(const Bitboard_75& board) const {
        // TODO: Implement complex 9-field absorption check
        return false;
    }

    inline int countAbsorbedCells(const Bitboard_75& board) const {
        return 0;
    }

public:
    DandelifeonSimulator_75(const SimulatorConfig& config)
        : maxTicks(config.maxTicks), manaPerCell(config.manaPerCell) {
    }

    SimulationResult<Bitboard_75> run(const Bitboard_75& startBoard, const Bitboard_75& obstacles) const override {
        SimulationResult<Bitboard_75> result;
        result.footstep.clear();

        Bitboard_75 currentState = startBoard;
        currentState.applyObstacles(obstacles);

        Bitboard_75 nextState;
        nextState.clear();

        Bitboard_75* curr = &currentState;
        Bitboard_75* nxt = &nextState;

        for (int t = 1; t <= maxTicks; ++t) {
            nxt->clear();

            engine.step(*curr, *nxt, obstacles);

            if (checkAbsorption(*nxt)) [[unlikely]] {
                result.success = true;
                result.ticks = t;
                result.mana = countAbsorbedCells(*nxt) * t * manaPerCell;
                result.footstep.merge(*curr);
                return result;
            }

            result.footstep.merge(*curr);
            std::swap(curr, nxt);

            if (curr->isEmpty()) [[unlikely]] {
                break;
            }
        }

        result.ticks = maxTicks;
        result.success = false;
        result.mana = 0;
        return result;
    }
};