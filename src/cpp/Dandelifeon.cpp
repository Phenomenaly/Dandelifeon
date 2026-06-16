#include <iostream>
#include <lua.hpp>

int runCppLoop(lua_State* L) {
    for (int i = 1; i <= 9; ++i) {
        std::cout << i;
    }
    std::cout << std::endl;
    return 0;
}

void runSimulation(lua_State* L) {
    std::cout << "Hello from C++ (Dandelifeon)!" << std::endl;

    lua_register(L, "run_cpp_loop", runCppLoop);

    if (luaL_dofile(L, "scripts/main.lua") != LUA_OK) {
        std::cerr << "Error: " << lua_tostring(L, -1) << std::endl;
    }

    std::cout << "Goodbye from C++ (Dandelifeon)!" << std::endl;
}