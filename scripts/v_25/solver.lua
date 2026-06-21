local Genome = require("Genome")

local Solver = {}
Solver.__index = Solver

local MUTATION_WALL_TOGGLE = 1
local MUTATION_SHIFT       = 2
local MUTATION_ROTATE      = 3

function Solver.new(local_rng)
    local self = setmetatable({}, Solver)
    self.rng = local_rng
    self.currentGenome = Genome.new()
    self.bestFitness = -1.0
    self.stagnation = 0
    self.lastFootprint = nil
    
    self.mutationWeights = {
        [MUTATION_WALL_TOGGLE] = 0.50,
        [MUTATION_SHIFT]       = 0.50,
        [MUTATION_ROTATE]      = 0.05
    }
    self.lastAppliedMutation = nil
    return self
end

function Solver:chooseMutationType()
    local roll = self.rng:randomFloat()
    local sum = 0.0
    for mType, weight in pairs(self.mutationWeights) do
        sum = sum + weight
        if roll <= sum then
            return mType
        end
    end
    return MUTATION_WALL_TOGGLE
end

function Solver:rewardLastMutation(success)
    if not self.lastAppliedMutation then return end
    local reward = 0.005
    local penalty = 0.001
    
    if success then
        self.mutationWeights[self.lastAppliedMutation] = self.mutationWeights[self.lastAppliedMutation] + reward
    else
        self.mutationWeights[self.lastAppliedMutation] = math.max(0.02, self.mutationWeights[self.lastAppliedMutation] - penalty)
    end
    
    local total = 0.0
    for _, w in pairs(self.mutationWeights) do total = total + w end
    for k, v in pairs(self.mutationWeights) do self.mutationWeights[k] = v / total end
end

function Solver:step()
    local nextGenome = Genome.clone(self.currentGenome)
    local mutationCount = 1
    if self.stagnation > 500000 then mutationCount = 3 end
    if self.stagnation > 5000000 then mutationCount = 10 end
    
    local mType = self:chooseMutationType()
    self.lastAppliedMutation = mType
    
    if mType == MUTATION_WALL_TOGGLE and self.lastFootprint then
        nextGenome:mutate(self.lastFootprint, mutationCount)
    elseif mType == MUTATION_SHIFT then
        local dx = self.rng:random(-1, 1)
        local dy = self.rng:random(-1, 1)
        nextGenome:translate(dx, dy)
    elseif mType == MUTATION_ROTATE then
        nextGenome:rotate90()
    end
    
    local res = run_simulation(nextGenome.cells, nextGenome.walls)
    
    local fitness = 0.0
    if res.success then
        fitness = res.mana
    end
    
    if fitness > self.bestFitness then
        self.currentGenome = nextGenome
        self.bestFitness = fitness
        self.lastFootprint = res.footstep
        self.stagnation = 0
        self:rewardLastMutation(true)
        
        Archive.submit(self.currentGenome.cells, self.currentGenome.walls, res.ticks, res.mana)
    else
        self.stagnation = self.stagnation + 1
        self:rewardLastMutation(false)
    end
    
    if self.stagnation > 1500000 then
        local eliteExists, eliteCells, eliteWalls = Archive.getElite()
        if eliteExists then
            self.currentGenome.cells = eliteCells
            self.currentGenome.walls = eliteWalls
            self.bestFitness = -1.0
            self.stagnation = 0
        else
            self.currentGenome:reset(self.rng)
            self.bestFitness = -1.0
            self.stagnation = 0
            
            local seed_res = run_simulation(self.currentGenome.cells, self.currentGenome.walls)
            self.lastFootprint = seed_res.footstep
        end
    end
end

return Solver