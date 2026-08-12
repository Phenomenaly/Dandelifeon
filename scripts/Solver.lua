---@diagnostic disable: undefined-global
local Genome = require("Genome")

local Solver = {}
Solver.__index = Solver

local MUTATION_WALL_TOGGLE = 1

local function cloneIndividual(src)
    local dest = {
        cells = Bitboard_new(),
        walls = Bitboard_new(),
        fitness = src.fitness,
        ticks = src.ticks,
        mana = src.mana,
        footstep = nil
    }
    dest.cells:merge(src.cells)
    dest.walls:merge(src.walls)
    if src.footstep then
        dest.footstep = Bitboard_new()
        dest.footstep:merge(src.footstep)
    end
    return dest
end

function Solver.new(local_rng)
    local self = setmetatable({}, Solver)
    self.rng = local_rng
    
    self.currentGenome = Genome.new()
    self.lastFootprint = nil
    
    self.parents = nil
    self.bestFitness = -1.0
    self.stagnation = 0
    self.generationCount = 0
    self.restartInterval = 1000000
    
    return self
end

function Solver:calculateFitness(res, walls)
    if res.success then
        local wallCount = walls:popcount()
        return 1000 + res.mana - (wallCount * 0.001)
    else
        local dist = BitboardHandler.getFigureDistance(res.footstep)
        local distBonus = (100 - dist) * 10
        local survivalBonus = res.ticks * 2
        return distBonus + survivalBonus
    end
end

function Solver:initializePopulation()
    self.parents = {}
    
    local firstInd = {
        cells = Bitboard_new(),
        walls = Bitboard_new(),
        fitness = 0,
        ticks = 0,
        mana = 0,
        footstep = nil
    }
    firstInd.cells:merge(self.currentGenome.cells)
    firstInd.walls:merge(self.currentGenome.walls)
    
    local res = run_simulation(firstInd.cells, firstInd.walls)
    firstInd.fitness = self:calculateFitness(res, firstInd.walls)
    firstInd.ticks = res.ticks
    firstInd.mana = res.mana
    firstInd.footstep = res.footstep
    
    table.insert(self.parents, firstInd)
    
    for i = 2, 8 do
        local ind = {
            cells = Bitboard_new(),
            walls = Bitboard_new(),
            fitness = 0,
            ticks = 0,
            mana = 0,
            footstep = nil
        }
        local offsetY = self.rng:random(2, 10)
        local pointingUp = self.rng:random(0, 1) == 1
        BitboardHandler.spawnTshapeSymmetric(ind.cells, offsetY, pointingUp)
        
        local r = run_simulation(ind.cells, ind.walls)
        ind.fitness = self:calculateFitness(r, ind.walls)
        ind.ticks = r.ticks
        ind.mana = r.mana
        ind.footstep = r.footstep
        
        table.insert(self.parents, ind)
    end
end

function Solver:step()
    if self.generationCount > 0 and self.generationCount % self.restartInterval == 0 then
        self.parents = nil
        self.bestFitness = -1.0
        self.stagnation = 0
        self.currentGenome:reset(self.rng)
    end

    if not self.parents then
        self:initializePopulation()
    end

    local children = {}
    
    for i = 1, 16 do
        local parent = self.parents[self.rng:random(1, 8)]
        local child = cloneIndividual(parent)
        
        local roll = self.rng:randomFloat()
        
        if roll < 0.15 then
            local dy = self.rng:random(-1, 1)
            if dy ~= 0 then
                local temp_cells = Bitboard_new()
                local temp_walls = Bitboard_new()
                BitboardHandler.translate(child.cells, temp_cells, 0, dy)
                BitboardHandler.translate(child.walls, temp_walls, 0, dy)
                child.cells = temp_cells
                child.walls = temp_walls
            end
            
        elseif roll < 0.30 then
            local rx = self.rng:random(0, 12)
            local ry = self.rng:random(1, 25)
            if child.walls:getCell(rx, ry) then
                child.walls:setCell(rx, ry, false)
                child.walls:setCell(24 - rx, ry, false)
            end
            
        else
            local mutationCount = 1
            if self.rng:randomFloat() < 0.15 then mutationCount = 2 end
            
            if parent.mana > 0 and self.rng:randomFloat() < 0.70 then
                for k = 1, mutationCount do
                    local rx = self.rng:random(7, 12)
                    local ry = self.rng:random(8, 18)
                    local val = child.walls:getCell(rx, ry)
                    child.walls:setCell(rx, ry, not val)
                    child.walls:setCell(24 - rx, ry, not val)
                end
            else
                if parent.footstep then
                    BitboardHandler.mutateWallsSymmetric(child.walls, parent.footstep, mutationCount)
                end
            end
        end
        
        local res = run_simulation(child.cells, child.walls)
        child.fitness = self:calculateFitness(res, child.walls)
        child.ticks = res.ticks
        child.mana = res.mana
        child.footstep = res.footstep
        
        table.insert(children, child)
    end
    
    local pool = {}
    for _, p in ipairs(self.parents) do table.insert(pool, p) end
    for _, c in ipairs(children) do table.insert(pool, c) end
    
    table.sort(pool, function(a, b)
        return a.fitness > b.fitness
    end)
    
    self.parents = {}
    for i = 1, 8 do
        table.insert(self.parents, pool[i])
    end
    
    local bestOfGen = self.parents[1]
    
    self.currentGenome.cells:clear()
    self.currentGenome.cells:merge(bestOfGen.cells)
    self.currentGenome.walls:clear()
    self.currentGenome.walls:merge(bestOfGen.walls)
    self.lastFootprint = bestOfGen.footstep
    
    if bestOfGen.fitness > self.bestFitness then
        self.bestFitness = bestOfGen.fitness
        self.stagnation = 0
        
        if bestOfGen.mana > 0 then
            Archive.submit(bestOfGen.cells, bestOfGen.walls, bestOfGen.ticks, bestOfGen.mana)
        end
    else
        self.stagnation = self.stagnation + 1
    end
    
    self.generationCount = self.generationCount + 1
    if self.generationCount % 25000 == 0 then
        local eliteExists, eliteCells, eliteWalls = Archive.getWorstElite()
        if eliteExists then
            local immigrant = {
                cells = Bitboard_new(),
                walls = Bitboard_new(),
                fitness = 0,
                ticks = 0,
                mana = 0,
                footstep = nil
            }
            immigrant.cells:merge(eliteCells)
            immigrant.walls:merge(eliteWalls)
            
            local res = run_simulation(immigrant.cells, immigrant.walls)
            immigrant.fitness = self:calculateFitness(res, immigrant.walls)
            immigrant.ticks = res.ticks
            immigrant.mana = res.mana
            immigrant.footstep = res.footstep
            
            self.parents[8] = immigrant
            
            table.sort(self.parents, function(a, b)
                return a.fitness > b.fitness
            end)
        end
    end
    
    if self.stagnation > 50000 then
        for i = 5, 8 do
            local ind = {
                cells = Bitboard_new(),
                walls = Bitboard_new(),
                fitness = 0,
                ticks = 0,
                mana = 0,
                footstep = nil
            }
            local offsetY = self.rng:random(2, 10)
            local pointingUp = self.rng:random(0, 1) == 1
            BitboardHandler.spawnTshapeSymmetric(ind.cells, offsetY, pointingUp)
            
            local res = run_simulation(ind.cells, ind.walls)
            ind.fitness = self:calculateFitness(res, ind.walls)
            ind.ticks = res.ticks
            ind.mana = res.mana
            ind.footstep = res.footstep
            
            self.parents[i] = ind
        end
        self.stagnation = 0
    end
end

return Solver