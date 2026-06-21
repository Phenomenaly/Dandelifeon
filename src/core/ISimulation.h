#pragma once

#include "Types.h"
#include "IBitboard.h"


template <typename BitboardType>
class IDandelifeonSimulator {
public:
    virtual ~IDandelifeonSimulator() = default;

    virtual SimulationResult<BitboardType> run(
        const BitboardType& startBoard,
        const BitboardType& obstacles
    ) const = 0;
};