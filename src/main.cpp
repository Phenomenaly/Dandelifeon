#define NOMINMAX
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>
#include <new>
#include <random>
#include <lua.hpp>

#include "../src/core/Bitboard.h"
#include "../src/core/Engine.h"
#include "../../Lua/include/lua.h"
#include "../../Lua/include/lauxlib.h"
#include "../../Lua/include/lualib.h"
#include "../src/cpp/Dandelifeon.cpp"
#include "../src/core/BitboardHandler.h"
#include "../src/cUI/ConsoleUI.h"


struct alignas(64) AlignedCounter {
    std::atomic<uint64_t> value{ 0 };
};

class ThreadSafeArchive {
private:
    std::mutex mtx;
    SharedLeaderboard leaderboard;
    std::vector<AlignedCounter> threadIterations;

public:
    ThreadSafeArchive(int threadCount) : threadIterations(threadCount) {
        leaderboard.threads.resize(threadCount);
        for (int i = 0; i < threadCount; ++i) {
            leaderboard.threads[i].threadId = i;
        }
    }

    void incrementIterations(int threadId) {
        threadIterations[threadId].value.fetch_add(1, std::memory_order_relaxed);
    }

    void submit(int threadId, const Bitboard_25& cells, const Bitboard_25& walls, int ticks, int mana) {
        std::lock_guard<std::mutex> lock(mtx);

        leaderboard.threads[threadId].ticks = ticks;
        leaderboard.threads[threadId].mana = mana;

        if (mana > leaderboard.bestMana) {
            leaderboard.bestMana = mana;
            leaderboard.bestCells = cells;
            leaderboard.bestWalls = walls;
        }
    }

    bool getElite(Bitboard_25& destCells, Bitboard_25& destWalls) {
        std::lock_guard<std::mutex> lock(mtx);
        if (leaderboard.bestMana == 0) return false;

        destCells = leaderboard.bestCells;
        destWalls = leaderboard.bestWalls;
        return true;
    }

    SharedLeaderboard getSnapshot() {
        std::lock_guard<std::mutex> lock(mtx);
        for (size_t i = 0; i < threadIterations.size(); ++i) {
            leaderboard.threads[i].iterations = threadIterations[i].value.load(std::memory_order_relaxed);
        }
        return leaderboard;
    }
};


int lua_Bitboard_new(lua_State* L) {
    void* storage = lua_newuserdata(L, sizeof(Bitboard_25));
    new (storage) Bitboard_25();
    luaL_getmetatable(L, "Bitboard_25_Meta");
    lua_setmetatable(L, -2);
    return 1;
}

int lua_Bitboard_clear(lua_State* L) {
    Bitboard_25* b = (Bitboard_25*)luaL_checkudata(L, 1, "Bitboard_25_Meta");
    b->clear();
    return 0;
}

int lua_Bitboard_isEmpty(lua_State* L) {
    Bitboard_25* b = (Bitboard_25*)luaL_checkudata(L, 1, "Bitboard_25_Meta");
    lua_pushboolean(L, b->isEmpty());
    return 1;
}

int lua_Bitboard_popcount(lua_State* L) {
    Bitboard_25* b = (Bitboard_25*)luaL_checkudata(L, 1, "Bitboard_25_Meta");
    lua_pushinteger(L, b->popcount());
    return 1;
}

int lua_Bitboard_merge(lua_State* L) {
    Bitboard_25* dest = (Bitboard_25*)luaL_checkudata(L, 1, "Bitboard_25_Meta");
    Bitboard_25* src = (Bitboard_25*)luaL_checkudata(L, 2, "Bitboard_25_Meta");
    dest->merge(*src);
    return 0;
}

int lua_Bitboard_setCell(lua_State* L) {
    Bitboard_25* b = (Bitboard_25*)luaL_checkudata(L, 1, "Bitboard_25_Meta");
    int x = (int)luaL_checkinteger(L, 2);
    int y = (int)luaL_checkinteger(L, 3);
    bool val = lua_toboolean(L, 4) != 0;
    b->setCell(x, y, val);
    return 0;
}

int lua_Bitboard_getCell(lua_State* L) {
    Bitboard_25* b = (Bitboard_25*)luaL_checkudata(L, 1, "Bitboard_25_Meta");
    int x = (int)luaL_checkinteger(L, 2);
    int y = (int)luaL_checkinteger(L, 3);
    lua_pushboolean(L, b->getCell(x, y));
    return 1;
}

int lua_Handler_spawnTSpark(lua_State* L) {
    Bitboard_25* b = (Bitboard_25*)luaL_checkudata(L, 1, "Bitboard_25_Meta");
    int x = (int)luaL_checkinteger(L, 2);
    int y = (int)luaL_checkinteger(L, 3);
    BitboardHandler::spawnTSpark(*b, x, y);
    return 0;
}

int lua_Handler_translate(lua_State* L) {
    Bitboard_25* src = (Bitboard_25*)luaL_checkudata(L, 1, "Bitboard_25_Meta");
    Bitboard_25* dest = (Bitboard_25*)luaL_checkudata(L, 2, "Bitboard_25_Meta");
    int dx = (int)luaL_checkinteger(L, 3);
    int dy = (int)luaL_checkinteger(L, 4);
    BitboardHandler::translate(*src, *dest, dx, dy);
    return 0;
}

int lua_Handler_rotate90(lua_State* L) {
    Bitboard_25* src = (Bitboard_25*)luaL_checkudata(L, 1, "Bitboard_25_Meta");
    Bitboard_25* dest = (Bitboard_25*)luaL_checkudata(L, 2, "Bitboard_25_Meta");
    BitboardHandler::rotate90(*src, *dest);
    return 0;
}

int lua_Handler_mutateWalls(lua_State* L) {
    Bitboard_25* obs = (Bitboard_25*)luaL_checkudata(L, 1, "Bitboard_25_Meta");
    Bitboard_25* footprint = (Bitboard_25*)luaL_checkudata(L, 2, "Bitboard_25_Meta");
    int count = (int)luaL_checkinteger(L, 3);
    BitboardHandler::mutateWallsOnFootprint(*obs, *footprint, count);
    return 0;
}

struct LuaThreadContext {
    int threadId;
    ThreadSafeArchive* archive;
    DandelifeonSimulator_25* simulator;
};

int lua_Simulator_run(lua_State* L) {
    LuaThreadContext* ctx = (LuaThreadContext*)lua_touserdata(L, lua_upvalueindex(1));
    Bitboard_25* cells = (Bitboard_25*)luaL_checkudata(L, 1, "Bitboard_25_Meta");
    Bitboard_25* walls = (Bitboard_25*)luaL_checkudata(L, 2, "Bitboard_25_Meta");

    ctx->archive->incrementIterations(ctx->threadId);

    SimulationResult_25 res = ctx->simulator->run(*cells, *walls);

    lua_newtable(L);

    lua_pushboolean(L, res.success);
    lua_setfield(L, -2, "success");

    lua_pushinteger(L, res.ticks);
    lua_setfield(L, -2, "ticks");

    lua_pushinteger(L, res.mana);
    lua_setfield(L, -2, "mana");

    void* storage = lua_newuserdata(L, sizeof(Bitboard_25));
    new (storage) Bitboard_25(res.footstep);
    luaL_getmetatable(L, "Bitboard_25_Meta");
    lua_setmetatable(L, -2);
    lua_setfield(L, -2, "footstep");

    return 1;
}

int lua_Archive_submit(lua_State* L) {
    LuaThreadContext* ctx = (LuaThreadContext*)lua_touserdata(L, lua_upvalueindex(1));
    Bitboard_25* cells = (Bitboard_25*)luaL_checkudata(L, 1, "Bitboard_25_Meta");
    Bitboard_25* walls = (Bitboard_25*)luaL_checkudata(L, 2, "Bitboard_25_Meta");
    int ticks = (int)luaL_checkinteger(L, 3);
    int mana = (int)luaL_checkinteger(L, 4);

    ctx->archive->submit(ctx->threadId, *cells, *walls, ticks, mana);
    return 0;
}

int lua_Archive_getElite(lua_State* L) {
    LuaThreadContext* ctx = (LuaThreadContext*)lua_touserdata(L, lua_upvalueindex(1));

    Bitboard_25 tempCells;
    Bitboard_25 tempWalls;

    if (ctx->archive->getElite(tempCells, tempWalls)) {
        lua_pushboolean(L, true);

        void* s1 = lua_newuserdata(L, sizeof(Bitboard_25));
        new (s1) Bitboard_25(tempCells);
        luaL_getmetatable(L, "Bitboard_25_Meta");
        lua_setmetatable(L, -2);

        void* s2 = lua_newuserdata(L, sizeof(Bitboard_25));
        new (s2) Bitboard_25(tempWalls);
        luaL_getmetatable(L, "Bitboard_25_Meta");
        lua_setmetatable(L, -2);

        return 3;
    }

    lua_pushboolean(L, false);
    return 1;
}

int lua_RNG_random(lua_State* L) {
    static thread_local std::random_device rd;
    static thread_local std::mt19937 gen(rd());

    int numArgs = lua_gettop(L);
    if (numArgs == 0) {
        std::uniform_real_distribution<double> dis(0.0, 1.0);
        lua_pushnumber(L, dis(gen));
        return 1;
    }
    else if (numArgs == 2) {
        int minVal = (int)luaL_checkinteger(L, 1);
        int maxVal = (int)luaL_checkinteger(L, 2);
        std::uniform_int_distribution<> dis(minVal, maxVal);
        lua_pushinteger(L, dis(gen));
        return 1;
    }
    return 0;
}

void registerCppInLua(lua_State* L, LuaThreadContext* context) {
    luaL_newmetatable(L, "Bitboard_25_Meta");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, lua_Bitboard_clear);
    lua_setfield(L, -2, "clear");
    lua_pushcfunction(L, lua_Bitboard_isEmpty);
    lua_setfield(L, -2, "isEmpty");
    lua_pushcfunction(L, lua_Bitboard_popcount);
    lua_setfield(L, -2, "popcount");
    lua_pushcfunction(L, lua_Bitboard_merge);
    lua_setfield(L, -2, "merge");
    lua_pushcfunction(L, lua_Bitboard_setCell);
    lua_setfield(L, -2, "setCell");
    lua_pushcfunction(L, lua_Bitboard_getCell);
    lua_setfield(L, -2, "getCell");
    lua_pop(L, 1);

    lua_register(L, "Bitboard_25_new", lua_Bitboard_new);

    lua_newtable(L);
    lua_pushcfunction(L, lua_Handler_spawnTSpark);
    lua_setfield(L, -2, "spawnTSpark");
    lua_pushcfunction(L, lua_Handler_translate);
    lua_setfield(L, -2, "translate");
    lua_pushcfunction(L, lua_Handler_rotate90);
    lua_setfield(L, -2, "rotate90");
    lua_pushcfunction(L, lua_Handler_mutateWalls);
    lua_setfield(L, -2, "mutateWallsOnFootprint");
    lua_setglobal(L, "BitboardHandler");

    lua_newtable(L);
    lua_pushlightuserdata(L, context);
    lua_pushcclosure(L, lua_Archive_submit, 1);
    lua_setfield(L, -2, "submit");

    lua_pushlightuserdata(L, context);
    lua_pushcclosure(L, lua_Archive_getElite, 1);
    lua_setfield(L, -2, "getElite");
    lua_setglobal(L, "Archive");

    lua_pushlightuserdata(L, context);
    lua_pushcclosure(L, lua_Simulator_run, 1);
    lua_setglobal(L, "run_simulation");

    lua_register(L, "cpp_random", lua_RNG_random);
}

void workerThreadTask(LuaThreadContext context) {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    registerCppInLua(L, &context);

    if (luaL_dofile(L, "scripts/main.lua") != LUA_OK) {
        std::cerr << "[Thread " << context.threadId << " Error] " << lua_tostring(L, -1) << std::endl;
    }

    lua_close(L);
}

int main() {
    int totalCores = std::thread::hardware_concurrency();
    int workerCount = totalCores > 1 ? totalCores - 1 : 1;

    SimulatorConfig config;
    config.maxTicks = 100;
    config.manaPerCell = 60;

    DandelifeonSimulator_25 simulator(config);
    ThreadSafeArchive archive(workerCount);

    std::vector<std::thread> workers;
    std::vector<LuaThreadContext> contexts;
    workers.reserve(workerCount);
    contexts.reserve(workerCount);

    for (int i = 0; i < workerCount; ++i) {
        contexts.push_back({ i, &archive, &simulator });
        workers.push_back(std::thread(workerThreadTask, contexts[i]));
    }

    ConsoleUI ui;
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ui.render(archive.getSnapshot());
    }

    for (auto& w : workers) {
        if (w.joinable()) w.join();
    }

    return 0;
}