namespace twbt_gui_hooks {
    using namespace DFHack;
    using df::global::enabler;

    bool read_map_tile(Screen::Pen &pen, int x, int y)
    {
        renderer_cool *r = (renderer_cool*)enabler->renderer;
        if (!r->is_twbt())
            return false;

        if (x < 1 || x > r->gdimx || y < 1 || y > r->gdimy)
            return false;

        const int tile = (x-1) * r->gdimy + (y-1);
        unsigned char *s = r->gscreen + tile*4;

        pen.ch = s[0];
        pen.fg = s[1] % 8;
        pen.bg = s[2];
        pen.bold = s[3] & 0x1; // check?

        return true;
    }

    bool write_map_tile(const Screen::Pen &pen, int x, int y)
    {
        renderer_cool *r = (renderer_cool*)enabler->renderer;
        if (!r->is_twbt())
            return false;

        if (x < 1 || x > r->gdimx || y < 1 || y > r->gdimy)
            return false;

        const int tile = (x-1) * r->gdimy + (y-1);
        unsigned char *s = r->gscreen + tile*4;
        s[0] = pen.ch;
        s[1] = pen.fg % 8;
        s[2] = pen.bg;
        s[3] = (pen.fg / 8 | pen.bold) | (s[3]&0xf0);

        r->gscreentexpos[tile] = 0;

        return true;
    }

    Screen::Pen get_tile(int x, int y, bool map);
    GUI_HOOK_CALLBACK(Screen::Hooks::get_tile, get_tile_hook, get_tile);
    Screen::Pen get_tile(int x, int y, bool map)
    {
        Screen::Pen pen;
        if (map && read_map_tile(pen, x, y))
            return pen;

        return get_tile_hook.next()(x, y, map);
    }

    void set_tile(const Screen::Pen &pen, int x, int y, bool map);
    GUI_HOOK_CALLBACK(Screen::Hooks::set_tile, set_tile_hook, set_tile);
    void set_tile(const Screen::Pen &pen, int x, int y, bool map)
    {
        if (map && write_map_tile(pen, x, y))
            return;

        return set_tile_hook.next()(pen, x, y, map);
    }

}
