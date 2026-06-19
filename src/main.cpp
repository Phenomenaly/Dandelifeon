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

#include "LuaAPI.h"
#include "../src/core/Bitboard.h"
#include "../src/core/Engine.h"
#include "../src/cpp/Dandelifeon.cpp"
#include "../src/core/BitboardHandler.h"
#include "../src/cUI/ConsoleUI.h"


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
    int totalCores = std::thread::hardware_concurrency() + 6;
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