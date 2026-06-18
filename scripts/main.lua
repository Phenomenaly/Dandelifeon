package.path = package.path .. ";scripts/?.lua;./scripts/?.lua"

local Genome = require("Genome")
local Solver = require("Solver")

local rng = {
    random = function(self, min, max)
        return cpp_random(min, max)
    end,
    randomFloat = function(self)
        return cpp_random()
    end
}

local solver = Solver.new(rng)
solver.currentGenome:reset(rng)

local init_res = run_simulation(solver.currentGenome.cells, solver.currentGenome.walls)
solver.lastFootprint = init_res.footstep

while true do
    solver:step()
end