#pragma once

// =======================================================================//
// !
// ! Dev Lua runner
// !
// ! Runs binary/client/test.lua inside the game's own Lua 5.1 VM (the one
// ! mission scripts use) and logs the result or the error to debug.log.
// ! Bound to F8 in m2o_module::tick.
// !
// ! Addresses target the Steam Classic mafia2.exe (same build m2sdk uses).
// ! loadbuffer/pcall/gettop/settop are fixed addresses; tolstring is found
// ! with a byte-pattern scan (same signature the rc4 mod used).
// !
// =======================================================================//

typedef int          (__cdecl *m2_lua_loadbuffer_t)(void *L, const char *buff, size_t size, const char *name);
typedef int          (__cdecl *m2_lua_pcall_t)(void *L, int nargs, int nresults, int errfunc);
typedef int          (__cdecl *m2_lua_gettop_t)(void *L);
typedef void         (__cdecl *m2_lua_settop_t)(void *L, int idx);
typedef const char * (__cdecl *m2_lua_tolstring_t)(void *L, int idx, size_t *len);

static const uintptr_t M2_LUA_PCALL      = 0x5D3F80;
static const uintptr_t M2_LUA_LOADBUFFER = 0x5D5BD0;
static const uintptr_t M2_LUA_SETTOP     = 0x5D2820;
static const uintptr_t M2_LUA_GETTOP     = 0x5D2800;
static const int       M2_LUA_MULTRET    = -1;

// Directory that m2o-client.dll lives in (no trailing slash).
static std::string luadev_client_dir() {
    char dll_path[MAX_PATH] = { 0 };
    HMODULE self = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCSTR)&luadev_client_dir, &self);
    GetModuleFileNameA(self, dll_path, MAX_PATH);
    std::string path(dll_path);
    return path.substr(0, path.rfind('\\'));
}

// Scan mafia2.exe's image for a byte pattern ('x' = must match, '?' = any).
static uintptr_t luadev_find_pattern(const unsigned char *pattern, const char *mask) {
    auto base = (unsigned char *)GetModuleHandle(NULL);
    auto dos  = (IMAGE_DOS_HEADER *)base;
    auto nt   = (IMAGE_NT_HEADERS *)(base + dos->e_lfanew);
    size_t image_size = nt->OptionalHeader.SizeOfImage;
    size_t len = strlen(mask);

    for (size_t i = 0; i + len <= image_size; ++i) {
        size_t j = 0;
        for (; j < len; ++j) {
            if (mask[j] == 'x' && base[i + j] != pattern[j]) break;
        }
        if (j == len) return (uintptr_t)(base + i);
    }
    return 0;
}

static m2_lua_tolstring_t luadev_tolstring() {
    static m2_lua_tolstring_t fn = nullptr;
    static bool searched = false;
    if (!searched) {
        searched = true;
        static const unsigned char pat[] = { 0x55,0x8B,0xEC,0x51,0x8B,0x45,0x0C,0x50,0x8B,0x4D,0x08,0x51,0xE8,0x8F,0xF8,0xFF };
        fn = (m2_lua_tolstring_t)luadev_find_pattern(pat, "xxxxxxxxxxxxxxxx");
        mod_log("[lua] lua_tolstring %s at 0x%p\n", fn ? "found" : "NOT found", (void *)fn);
    }
    return fn;
}

// Same chain as M2::Wrappers::lua::Execute (script machine 0's lua_State).
static void *luadev_state() {
    auto mgr = M2::C_ScriptMachineManager::Get();
    if (!mgr || !mgr->GetInterface()) return nullptr;
    auto machine = mgr->GetInterface()->m_pScriptMachine;
    if (!machine || !machine->m_pScriptPool) return nullptr;
    auto entry = machine->m_pScriptPool->m_pUnknow[0];
    if (!entry) return nullptr;
    return (void *)entry->m_pLuaState;
}

// Run a chunk of Lua in the game VM. Logs every return value, or the error.
static bool luadev_run(const std::string &code, const char *chunk_name) {
    void *L = luadev_state();
    if (!L) { mod_log("[lua] no lua_State yet (script machine not ready)\n"); return false; }

    auto loadbuffer = (m2_lua_loadbuffer_t)M2_LUA_LOADBUFFER;
    auto pcall      = (m2_lua_pcall_t)M2_LUA_PCALL;
    auto gettop     = (m2_lua_gettop_t)M2_LUA_GETTOP;
    auto settop     = (m2_lua_settop_t)M2_LUA_SETTOP;
    auto tolstring  = luadev_tolstring();

    int base = gettop(L);
    int status = loadbuffer(L, code.c_str(), code.size(), chunk_name);
    if (status == 0) {
        status = pcall(L, 0, M2_LUA_MULTRET, 0);
    }

    int top = gettop(L);
    if (status != 0) {
        const char *err = tolstring ? tolstring(L, -1, nullptr) : nullptr;
        mod_log("[lua] %s failed (status %d): %s\n", chunk_name, status, err ? err : "(no message)");
    } else {
        mod_log("[lua] %s ok, %d result(s)\n", chunk_name, top - base);
        for (int i = base + 1; i <= top; ++i) {
            const char *s = tolstring ? tolstring(L, i, nullptr) : nullptr;
            mod_log("[lua]   [%d] %s\n", i - base, s ? s : "(nil/bool/table/userdata)");
        }
    }

    settop(L, base); // leave the game's stack exactly as we found it
    return status == 0;
}

static void luadev_run_test_file() {
    std::string path = luadev_client_dir() + "\\test.lua";
    FILE *f = fopen(path.c_str(), "rb");
    if (!f) { mod_log("[lua] %s not found\n", path.c_str()); return; }

    std::string code;
    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0) code.append(buf, n);
    fclose(f);

    mod_log("[lua] running test.lua (%u bytes)\n", (unsigned)code.size());
    luadev_run(code, "@test.lua");
}
