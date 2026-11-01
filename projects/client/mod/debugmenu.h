#pragma once

// =======================================================================//
// !
// ! Debug menu (F6)
// !
// ! Up/Down: choose a row   Left/Right: change the value   Enter: do it
// !
// ! Weapons / weather / time / heal / teleport act on YOUR game only.
// ! Spawn car goes through the server, so both players get the car.
// !
// =======================================================================//

struct dbg_weapon { int id; const char *name; };
static const dbg_weapon dbg_weapons[] = {
    {  2, "Model 12 Revolver" }, {  3, "Mauser C96" },     {  4, "Colt M1911A1" },
    {  5, "Colt M1911 Special" }, { 6, "Model 19 Revolver" }, { 8, "Remington 870" },
    {  9, "M3 Grease Gun" },     { 10, "MP40" },            { 11, "Thompson 1928" },
    { 12, "M1A1 Thompson" },     { 13, "Beretta 38A" },     { 15, "M1 Garand" },
    { 17, "Kar98k" },            { 20, "Mk II Grenade" },   { 21, "Molotov Cocktail" },
    /* left out: 7 + 16 (grenades that don't explode), 14 MG42 (broken anim), 18 (missing model) */
};

static const char *dbg_weathers[] = {
    "DTFreeRideDay", "DTFreeRideDayRain", "DTFreerideNight",
    "DT_RTRclear_day_early_morn1", "DT_RTRclear_day_morning", "DT_RTRclear_day_noon",
    "DT_RTRclear_day_afternoon", "DT_RTRclear_day_late_afternoon", "DT_RTRclear_day_evening",
    "DT_RTRclear_day_night",
    "DT_RTRrainy_day_morning", "DT_RTRrainy_day_noon", "DT_RTRrainy_day_afternoon",
    "DT_RTRrainy_day_evening", "DT_RTRrainy_day_late_even",
    "DT_RTRfoggy_day_morning", "DT_RTRfoggy_day_noon", "DT_RTRfoggy_day_evening",
    "DT_RTRfoggy_day_night",
    "DTFreeRideDaySnow", "DTFreeRideNightSnow", "DTFreeRideDayWinter",
};

#define DBG_ARRAY_COUNT(a) ((int)(sizeof(a) / sizeof((a)[0])))
#define DBG_CAR_MIN 0
#define DBG_CAR_MAX 54 /* server clamps to 0..54 (M2_CAR_MODELS) */
#define DBG_ROWS    8

static struct {
    bool open;
    bool god;          /* old m2o debug behaviour: health pinned to max while connected */
    int  row;
    int  car;          /* model id 0..54 */
    int  weapon;       /* index into dbg_weapons */
    int  weather;      /* index into dbg_weathers */
    int  hour;         /* 0..23 */
    bool dirty;
    gfx_handle bg;
    gfx_handle lines[DBG_ROWS + 2];
    char status[128];
    bool inited;
} dbg = { false, true, 0, 10, 9, 0, 12, true, -1, {0}, "", false };

static bool debugmenu_godmode() { return dbg.god; }
static int  debugmenu_car_model() { return dbg.car; }

static void dbg_status(const char *fmt, ...) {
    va_list ap; va_start(ap, fmt);
    vsnprintf(dbg.status, sizeof(dbg.status), fmt, ap);
    va_end(ap);
    mod_log("[debugmenu] %s\n", dbg.status);
    dbg.dirty = true;
}

static M2::C_Human2 *dbg_local_human() {
    auto p = M2::C_Game::Get() ? M2::C_Game::Get()->GetLocalPed() : nullptr;
    return (M2::C_Human2 *)p;
}

static void dbg_row_text(int row, char *out, size_t n) {
    const char *cur = (row == dbg.row) ? "> " : "  ";
    switch (row) {
        case 0: snprintf(out, n, "%sSpawn car           < model %d >", cur, dbg.car); break;
        case 1: snprintf(out, n, "%sGive weapon         < %s >", cur, dbg_weapons[dbg.weapon].name); break;
        case 2: snprintf(out, n, "%sWeather             < %s >", cur, dbg_weathers[dbg.weather]); break;
        case 3: snprintf(out, n, "%sTime of day         < %02d:00 >", cur, dbg.hour); break;
        case 4: snprintf(out, n, "%sHeal", cur); break;
        case 5: snprintf(out, n, "%sGod mode            < %s >", cur, dbg.god ? "ON" : "OFF"); break;
        case 6: snprintf(out, n, "%sTeleport to other player", cur); break;
        case 7: snprintf(out, n, "%sRun test.lua (F8)", cur); break;
    }
}

static void dbg_redraw() {
    for (int i = 0; i < DBG_ROWS + 2; ++i) {
        if (dbg.lines[i] >= 0) { gfx_destroy(dbg.lines[i]); dbg.lines[i] = -1; }
    }
    if (dbg.bg >= 0 && !dbg.open) { gfx_destroy(dbg.bg); dbg.bg = -1; }
    if (!dbg.open) return;

    const int x = 40, y = 120, line_h = 24, w = 560;
    if (dbg.bg < 0) {
        dbg.bg = gfx_create_rect(0, 0, w, line_h * (DBG_ROWS + 3) + 16, vec4f(15, 15, 15, 200));
        gfx_render_add(dbg.bg, 30);
        gfx_position_set(dbg.bg, x - 12, y - 12);
    }

    char buf[160];
    snprintf(buf, sizeof(buf), "DEBUG MENU  (F6 close, arrows + Enter)");
    dbg.lines[0] = gfx_create_text(0, 18, buf, vec4f(255, 200, 80, 255));
    gfx_render_add(dbg.lines[0], 31);
    gfx_position_set(dbg.lines[0], x, y);

    for (int r = 0; r < DBG_ROWS; ++r) {
        dbg_row_text(r, buf, sizeof(buf));
        vec4 col = (r == dbg.row) ? vec4f(255, 255, 255, 255) : vec4f(190, 190, 190, 255);
        dbg.lines[r + 1] = gfx_create_text(0, 16, buf, col);
        gfx_render_add(dbg.lines[r + 1], 31);
        gfx_position_set(dbg.lines[r + 1], x, y + line_h * (r + 1) + 6);
    }

    dbg.lines[DBG_ROWS + 1] = gfx_create_text(0, 14, dbg.status[0] ? dbg.status : " ", vec4f(140, 220, 140, 255));
    gfx_render_add(dbg.lines[DBG_ROWS + 1], 31);
    gfx_position_set(dbg.lines[DBG_ROWS + 1], x, y + line_h * (DBG_ROWS + 1) + 12);
}

static void dbg_apply_time() {
    auto fx = M2::C_GfxEnvironmentEffects::Get();
    if (fx && fx->GetWeatherManager()) {
        fx->GetWeatherManager()->SetTime(dbg.hour / 24.0f);
        dbg_status("time set to %02d:00 (only on your game)", dbg.hour);
    }
}

static void dbg_activate() {
    auto me = dbg_local_human();
    switch (dbg.row) {
        case 0: {
            if (!mod_connected()) { dbg_status("connect first (F5) - cars come from the server"); break; }
            mod_message_send(ctx, M2O_CAR_CREATE, [&](librg_data_t *data) { librg_data_wu16(data, (u16)dbg.car); });
            dbg_status("asked the server for car model %d", dbg.car);
        } break;
        case 1: {
            if (!me) break;
            me->GetInventory()->AddWeapon(dbg_weapons[dbg.weapon].id, 200);
            dbg_status("gave %s (id %d) with 200 ammo", dbg_weapons[dbg.weapon].name, dbg_weapons[dbg.weapon].id);
        } break;
        case 2: {
            auto fx = M2::C_GfxEnvironmentEffects::Get();
            if (fx && fx->GetWeatherManager()) {
                fx->GetWeatherManager()->SetDayTemplate(dbg_weathers[dbg.weather]);
                dbg_status("weather: %s (only on your game)", dbg_weathers[dbg.weather]);
            }
        } break;
        case 3: dbg_apply_time(); break;
        case 4: {
            if (!me) break;
            me->GetScript()->SetHealth(720.0f);
            dbg_status("healed");
        } break;
        case 5: dbg.god = !dbg.god; dbg_status("god mode %s", dbg.god ? "on" : "off"); break;
        case 6: {
            if (!me || !mod.player) break;
            bool done = false;
            mod_entity_iterate(ctx, LIBRG_ENTITY_ALIVE, [&](librg_entity_t *entity) {
                if (done || entity == mod.player || entity->type != M2O_ENTITY_PLAYER_PED) return;
                auto target = entity->position + vec3f(1.5f, 0, 0.2f);
                reinterpret_cast<M2::C_Entity *>(me)->SetPosition(target);
                done = true;
            });
            dbg_status(done ? "teleported next to the other player" : "no other player in range");
        } break;
        case 7: luadev_run_test_file(); dbg_status("ran test.lua - see debug.log"); break;
    }
}

static int dbg_wrap(int v, int lo, int hi) { return v < lo ? hi : (v > hi ? lo : v); }

static void dbg_change(int dir) {
    switch (dbg.row) {
        case 0: dbg.car     = dbg_wrap(dbg.car + dir, DBG_CAR_MIN, DBG_CAR_MAX); break;
        case 1: dbg.weapon  = dbg_wrap(dbg.weapon + dir, 0, DBG_ARRAY_COUNT(dbg_weapons) - 1); break;
        case 2: dbg.weather = dbg_wrap(dbg.weather + dir, 0, DBG_ARRAY_COUNT(dbg_weathers) - 1); break;
        case 3: dbg.hour    = dbg_wrap(dbg.hour + dir, 0, 23); dbg_apply_time(); break;
        case 5: dbg.god     = !dbg.god; dbg_status("god mode %s", dbg.god ? "on" : "off"); break;
        default: return;
    }
    dbg.dirty = true;
}

/* runs on the render thread (see gfx_before_draw_set) - the only place we touch gfx objects */
static volatile LONG dbg_redraw_pending = 1;
static void debugmenu_render_cb() {
    if (InterlockedExchange(&dbg_redraw_pending, 0)) {
        dbg_redraw();
    }
}

// call once per tick
static void debugmenu_tick() {
    if (!dbg.inited) {
        dbg.inited = true;
        dbg.bg = -1;
        for (int i = 0; i < DBG_ROWS + 2; ++i) dbg.lines[i] = -1;
        gfx_before_draw_set(debugmenu_render_cb);
    }

    if (input_key_down(VK_F6)) {
        dbg.open  = !dbg.open;
        dbg.dirty = true;
        input_keys_block_set(dbg.open); /* the game doesn't see arrows/Enter/etc while the menu is open */
        mod_log("[debugmenu] %s\n", dbg.open ? "opened" : "closed");
    }

    if (dbg.open) {
        if (input_key_down(VK_UP))     { dbg.row = dbg_wrap(dbg.row - 1, 0, DBG_ROWS - 1); dbg.dirty = true; }
        if (input_key_down(VK_DOWN))   { dbg.row = dbg_wrap(dbg.row + 1, 0, DBG_ROWS - 1); dbg.dirty = true; }
        if (input_key_down(VK_LEFT))   { dbg_change(-1); }
        if (input_key_down(VK_RIGHT))  { dbg_change(+1); }
        if (input_key_down(VK_RETURN)) { dbg_activate(); }
    }

    if (dbg.dirty) { dbg.dirty = false; InterlockedExchange(&dbg_redraw_pending, 1); }
}
