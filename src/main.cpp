#define NOMINMAX
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>
#include <new>
#include <random>
#include <lua.hpp>

//#include "../../Lua/include/lua.h"
//#include "../../Lua/include/lauxlib.h"
//#include "../../Lua/include/lualib.h"

#define GRID_SIZE 25

#if GRID_SIZE == 25
#include "v_25/Bitboard_25.h"
#include "v_25/Engine_25.h"
#include "v_25/Dandelifeon_25.h"
using ConcreteBitboard = Bitboard_25;
using ConcreteSimulator = DandelifeonSimulator_25;
#elif GRID_SIZE == 75
#include "v_75/Bitboard_75.h"
#include "v_75/Engine_75.h"
#include "v_75/Dandelifeon_75.h"
using ConcreteBitboard = Bitboard_75;
using ConcreteSimulator = DandelifeonSimulator_75;
#endif

#include "core/Archive.h"
#include "LuaAPI.h"
#include "cUI/ConsoleUI.h"

using ActiveContext = LuaThreadContext<ConcreteBitboard, ConcreteSimulator>;
using ActiveArchive = ThreadSafeArchive<ConcreteBitboard>;
using ActiveUI = ConsoleUI<ConcreteBitboard>;


void workerThreadTask(ActiveContext context) {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    registerCppInLua<ConcreteBitboard, ConcreteSimulator>(L, &context);

#if GRID_SIZE == 25
    const char* scriptPath = "scripts/main_25.lua";
#else
    const char* scriptPath = "scripts/main_75.lua";
#endif

    if (luaL_dofile(L, scriptPath) != LUA_OK) {
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

    ConcreteSimulator simulator(config);
    ActiveArchive archive(workerCount);

    std::vector<std::thread> workers;
    std::vector<ActiveContext> contexts;
    workers.reserve(workerCount);
    contexts.reserve(workerCount);

    for (int i = 0; i < workerCount; ++i) {
        contexts.push_back({ i, &archive, &simulator });
        workers.push_back(std::thread(workerThreadTask, contexts[i]));
    }

    ActiveUI ui;
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ui.render(archive.getSnapshot(), archive.getTopTen());
    }

    for (auto& w : workers) {
        if (w.joinable()) w.join();
    }

    return 0;
}