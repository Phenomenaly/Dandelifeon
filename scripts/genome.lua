local Genome = {}
Genome.__index = Genome

function Genome.new()
    local self = setmetatable({}, Genome)
    self.cells = Bitboard_25_new()
    self.walls = Bitboard_25_new()
    return self
end

function Genome.clone(src)
    local dest = Genome.new()
    dest.cells:clear()
    dest.cells:merge(src.cells)
    dest.walls:clear()
    dest.walls:merge(src.walls)
    return dest
end

function Genome:translate(dx, dy)
    local temp_cells = Bitboard_25_new()
    local temp_walls = Bitboard_25_new()
    BitboardHandler.translate(self.cells, temp_cells, dx, dy)
    BitboardHandler.translate(self.walls, temp_walls, dx, dy)
    self.cells = temp_cells
    self.walls = temp_walls
end

function Genome:rotate90()
    local temp_cells = Bitboard_25_new()
    local temp_walls = Bitboard_25_new()
    BitboardHandler.rotate90(self.cells, temp_cells)
    BitboardHandler.rotate90(self.walls, temp_walls)
    self.cells = temp_cells
    self.walls = temp_walls
end

function Genome:reset(local_rng)
    self.cells:clear()
    self.walls:clear()
    local offsetX = local_rng:random(5, 15)
    local offsetY = local_rng:random(5, 15)
    BitboardHandler.spawnTSpark(self.cells, offsetX, offsetY)
end

function Genome:mutate(footstep, mutation_count)
    BitboardHandler.mutateWallsOnFootprint(self.walls, footstep, mutation_count)
end

return Genome