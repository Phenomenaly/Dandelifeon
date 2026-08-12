#define NOMINMAX
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>
#include <new>
#include <random>
#include <lua.hpp>

#include "Bitboard.h"
#include "Engine.h"
#include "Dandelifeon.h"
#include "BitboardHandler.h"
#include "ConsoleUI.h"
#include "Archive.h"
#include "LuaAPI.h"


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

    Simulator simulator(config);
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
        ui.render(archive.getSnapshot(), archive.getTopTen(), config);
    }

    for (auto& w : workers) {
        if (w.joinable()) w.join();
    }

    return 0;
}