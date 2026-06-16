#include <lua.hpp>

void runSimulation(lua_State* L);

int main() {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    runSimulation(L);

    lua_close(L);
    return 0;
}