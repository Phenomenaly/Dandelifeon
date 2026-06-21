#pragma once

#define NOMINMAX
#include <lua.hpp>
#include <new>
#include <random>
#include <iostream>
#include <algorithm>

#include "core/IBitboard.h"
#include "core/BitboardHandler.h"
#include "core/Archive.h"


template <typename BitboardType, typename SimulatorType>
struct LuaThreadContext {
    int threadId;
    ThreadSafeArchive<BitboardType>* archive;
    SimulatorType* simulator;
};

template <typename BitboardType>
inline int lua_Bitboard_new(lua_State* L) {
    void* storage = lua_newuserdata(L, sizeof(BitboardType));
    new (storage) BitboardType();
    luaL_getmetatable(L, BitboardType::META_NAME);
    lua_setmetatable(L, -2);
    return 1;
}

template <typename BitboardType>
inline int lua_Bitboard_clear(lua_State* L) {
    BitboardType* b = (BitboardType*)luaL_checkudata(L, 1, BitboardType::META_NAME);
    b->clear();
    return 0;
}

template <typename BitboardType>
inline int lua_Bitboard_isEmpty(lua_State* L) {
    BitboardType* b = (BitboardType*)luaL_checkudata(L, 1, BitboardType::META_NAME);
    lua_pushboolean(L, b->isEmpty());
    return 1;
}

template <typename BitboardType>
inline int lua_Bitboard_popcount(lua_State* L) {
    BitboardType* b = (BitboardType*)luaL_checkudata(L, 1, BitboardType::META_NAME);
    lua_pushinteger(L, b->popcount());
    return 1;
}

template <typename BitboardType>
inline int lua_Bitboard_merge(lua_State* L) {
    BitboardType* dest = (BitboardType*)luaL_checkudata(L, 1, BitboardType::META_NAME);
    BitboardType* src = (BitboardType*)luaL_checkudata(L, 2, BitboardType::META_NAME);
    dest->merge(*src);
    return 0;
}

template <typename BitboardType>
inline int lua_Bitboard_setCell(lua_State* L) {
    BitboardType* b = (BitboardType*)luaL_checkudata(L, 1, BitboardType::META_NAME);
    int x = (int)luaL_checkinteger(L, 2);
    int y = (int)luaL_checkinteger(L, 3);
    bool val = lua_toboolean(L, 4) != 0;
    b->setCell(x, y, val);
    return 0;
}

template <typename BitboardType>
inline int lua_Bitboard_getCell(lua_State* L) {
    BitboardType* b = (BitboardType*)luaL_checkudata(L, 1, BitboardType::META_NAME);
    int x = (int)luaL_checkinteger(L, 2);
    int y = (int)luaL_checkinteger(L, 3);
    lua_pushboolean(L, b->getCell(x, y));
    return 1;
}

template <typename BitboardType>
inline int lua_Handler_spawnTshape(lua_State* L) {
    BitboardType* b = (BitboardType*)luaL_checkudata(L, 1, BitboardType::META_NAME);
    int x = (int)luaL_checkinteger(L, 2);
    int y = (int)luaL_checkinteger(L, 3);
    BitboardHandler::spawnTshape(*b, x, y);
    return 0;
}

template <typename BitboardType>
inline int lua_Handler_translate(lua_State* L) {
    BitboardType* src = (BitboardType*)luaL_checkudata(L, 1, BitboardType::META_NAME);
    BitboardType* dest = (BitboardType*)luaL_checkudata(L, 2, BitboardType::META_NAME);
    int dx = (int)luaL_checkinteger(L, 3);
    int dy = (int)luaL_checkinteger(L, 4);
    BitboardHandler::translate(*src, *dest, dx, dy);
    return 0;
}

template <typename BitboardType>
inline int lua_Handler_rotate90(lua_State* L) {
    BitboardType* src = (BitboardType*)luaL_checkudata(L, 1, BitboardType::META_NAME);
    BitboardType* dest = (BitboardType*)luaL_checkudata(L, 2, BitboardType::META_NAME);
    BitboardHandler::rotate90(*src, *dest);
    return 0;
}

template <typename BitboardType>
inline int lua_Handler_mutateWallsSymmetric(lua_State* L) {
    BitboardType* obs = (BitboardType*)luaL_checkudata(L, 1, BitboardType::META_NAME);
    BitboardType* footprint = (BitboardType*)luaL_checkudata(L, 2, BitboardType::META_NAME);
    int count = (int)luaL_checkinteger(L, 3);
    BitboardHandler::mutateWallsSymmetric(*obs, *footprint, count);
    return 0;
}

template <typename BitboardType>
inline int lua_Handler_getFigureDistance(lua_State* L) {
    BitboardType* cells = (BitboardType*)luaL_checkudata(L, 1, BitboardType::META_NAME);
    double dist = BitboardHandler::getFigureDistance(*cells);
    lua_pushnumber(L, dist);
    return 1;
}

template <typename BitboardType>
inline int lua_Handler_spawnTshapeSymmetric(lua_State* L) {
    BitboardType* b = (BitboardType*)luaL_checkudata(L, 1, BitboardType::META_NAME);
    int y = (int)luaL_checkinteger(L, 2);
    bool pointingUp = lua_toboolean(L, 3) != 0;
    BitboardHandler::spawnTshapeSymmetric(*b, y, pointingUp);
    return 0;
}


template <typename BitboardType, typename SimulatorType>
inline int lua_Simulator_run(lua_State* L) {
    using ContextType = LuaThreadContext<BitboardType, SimulatorType>;
    ContextType* ctx = (ContextType*)lua_touserdata(L, lua_upvalueindex(1));

    BitboardType* cells = (BitboardType*)luaL_checkudata(L, 1, BitboardType::META_NAME);
    BitboardType* walls = (BitboardType*)luaL_checkudata(L, 2, BitboardType::META_NAME);

    ctx->archive->incrementIterations(ctx->threadId);

    SimulationResult<BitboardType> res = ctx->simulator->run(*cells, *walls);

    lua_newtable(L);
    lua_pushboolean(L, res.success);
    lua_setfield(L, -2, "success");
    lua_pushinteger(L, res.ticks);
    lua_setfield(L, -2, "ticks");
    lua_pushinteger(L, res.mana);
    lua_setfield(L, -2, "mana");

    void* storage = lua_newuserdata(L, sizeof(BitboardType));
    new (storage) BitboardType(res.footstep);
    luaL_getmetatable(L, BitboardType::META_NAME);
    lua_setmetatable(L, -2);
    lua_setfield(L, -2, "footstep");

    return 1;
}

template <typename BitboardType, typename SimulatorType>
inline int lua_Archive_submit(lua_State* L) {
    using ContextType = LuaThreadContext<BitboardType, SimulatorType>;
    ContextType* ctx = (ContextType*)lua_touserdata(L, lua_upvalueindex(1));

    BitboardType* cells = (BitboardType*)luaL_checkudata(L, 1, BitboardType::META_NAME);
    BitboardType* walls = (BitboardType*)luaL_checkudata(L, 2, BitboardType::META_NAME);
    int ticks = (int)luaL_checkinteger(L, 3);
    int mana = (int)luaL_checkinteger(L, 4);

    ctx->archive->submit(ctx->threadId, *cells, *walls, ticks, mana);
    return 0;
}

template <typename BitboardType, typename SimulatorType>
inline int lua_Archive_getElite(lua_State* L) {
    using ContextType = LuaThreadContext<BitboardType, SimulatorType>;
    ContextType* ctx = (ContextType*)lua_touserdata(L, lua_upvalueindex(1));

    static thread_local std::random_device rd;
    static thread_local std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 100);
    int random_index = dis(gen);

    BitboardType tempCells;
    BitboardType tempWalls;

    if (ctx->archive->getElite(tempCells, tempWalls, random_index)) {
        lua_pushboolean(L, true);

        void* s1 = lua_newuserdata(L, sizeof(BitboardType));
        new (s1) BitboardType(tempCells);
        luaL_getmetatable(L, BitboardType::META_NAME);
        lua_setmetatable(L, -2);

        void* s2 = lua_newuserdata(L, sizeof(BitboardType));
        new (s2) BitboardType(tempWalls);
        luaL_getmetatable(L, BitboardType::META_NAME);
        lua_setmetatable(L, -2);

        return 3;
    }

    lua_pushboolean(L, false);
    return 1;
}

inline int lua_RNG_random(lua_State* L) {
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

template <typename BitboardType, typename SimulatorType>
inline void registerCppInLua(lua_State* L, LuaThreadContext<BitboardType, SimulatorType>* context) {
    luaL_newmetatable(L, BitboardType::META_NAME);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, lua_Bitboard_clear<BitboardType>);
    lua_setfield(L, -2, "clear");
    lua_pushcfunction(L, lua_Bitboard_isEmpty<BitboardType>);
    lua_setfield(L, -2, "isEmpty");
    lua_pushcfunction(L, lua_Bitboard_popcount<BitboardType>);
    lua_setfield(L, -2, "popcount");
    lua_pushcfunction(L, lua_Bitboard_merge<BitboardType>);
    lua_setfield(L, -2, "merge");
    lua_pushcfunction(L, lua_Bitboard_setCell<BitboardType>);
    lua_setfield(L, -2, "setCell");
    lua_pushcfunction(L, lua_Bitboard_getCell<BitboardType>);
    lua_setfield(L, -2, "getCell");
    lua_pop(L, 1);

    lua_register(L, "Bitboard_25_new", lua_Bitboard_new<BitboardType>);

    lua_newtable(L);
    lua_pushcfunction(L, lua_Handler_spawnTshape<BitboardType>);
    lua_setfield(L, -2, "spawnTshape");
    lua_pushcfunction(L, lua_Handler_translate<BitboardType>);
    lua_setfield(L, -2, "translate");
    lua_pushcfunction(L, lua_Handler_rotate90<BitboardType>);
    lua_setfield(L, -2, "rotate90");
    lua_pushcfunction(L, lua_Handler_getFigureDistance<BitboardType>);
    lua_setfield(L, -2, "getFigureDistance");
    lua_pushcfunction(L, lua_Handler_spawnTshapeSymmetric<BitboardType>);
    lua_setfield(L, -2, "spawnTshapeSymmetric");
    lua_pushcfunction(L, lua_Handler_mutateWallsSymmetric<BitboardType>);
    lua_setfield(L, -2, "mutateWallsSymmetric");
    lua_setglobal(L, "BitboardHandler");

    lua_newtable(L);
    lua_pushlightuserdata(L, context);
    lua_pushcclosure(L, (lua_Archive_submit<BitboardType, SimulatorType>), 1);
    lua_setfield(L, -2, "submit");

    lua_pushlightuserdata(L, context);
    lua_pushcclosure(L, (lua_Archive_getElite<BitboardType, SimulatorType>), 1);
    lua_setfield(L, -2, "getElite");
    lua_setglobal(L, "Archive");

    lua_pushlightuserdata(L, context);
    lua_pushcclosure(L, (lua_Simulator_run<BitboardType, SimulatorType>), 1);
    lua_setglobal(L, "run_simulation");

    lua_register(L, "cpp_random", lua_RNG_random);
}