#include <iostream>
#include <lua.hpp>

int cpp_HelloWorld(lua_State* L) {
    std::cout << "[C++] Hello! This message is printed by a C++ function called inside Lua." << std::endl;
    return 0;
}

int main() {
    lua_State* L = luaL_newstate();

    if (!L) {
        std::cerr << "Failed to initialize Lua state." << std::endl;
        return -1;
    }

    luaL_openlibs(L);

    lua_register(L, "cpp_hello", cpp_HelloWorld);

    std::cout << "[C++] Starting Lua execution..." << std::endl;

    const char* lua_script =
        "print('[Lua] Hello World from Lua environment!')\n"
        "cpp_hello()\n";

    int result = luaL_dostring(L, lua_script);

    if (result != LUA_OK) {
        std::cerr << "[Error] Lua error: " << lua_tostring(L, -1) << std::endl;
    }
    lua_close(L);

    std::cout << "[C++] Execution finished." << std::endl;
    return 0;
}