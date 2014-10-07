static bool is_text_tile(int x, int y, bool &is_map)
{
    const int tile = x * tdimy + y;
    df::viewscreen * ws = Gui::getCurViewscreen();

    int32_t w = tdimx, h = tdimy;

    is_map = false;

#define IS_SCREEN(_sc) df::_sc::_identity.is_direct_instance(ws)     

    if (IS_SCREEN(viewscreen_dungeonmodest))
    {
        //df::viewscreen_dungeonmodest *s = strict_virtual_cast<df::viewscreen_dungeonmodest>(ws);
        //TODO:

        if (y >= h-2)
            return true;

        return false;
    }    

    if (!x || !y || x == w - 1 || y == h - 1)
       return has_textfont;

    if (IS_SCREEN(viewscreen_dwarfmodest))
    {
        uint8_t menu_width, area_map_width;
        Gui::getMenuWidth(menu_width, area_map_width);
        int32_t menu_left = w - 1, menu_right = w - 1;

        bool menuforced = (ui->main.mode != df::ui_sidebar_mode::Default || df::global::cursor->x != -30000);

        if ((menuforced || menu_width == 1) && area_map_width == 2) // Menu + area map
        {
            menu_left = w - 56;
            menu_right = w - 25;
        }
        else if (menu_width == 2 && area_map_width == 2) // Area map only
        {
            menu_left = menu_right = w - 25;
        }
        else if (menu_width == 1) // Wide menu
            menu_left = w - 56;
        else if (menuforced || (menu_width == 2 && area_map_width == 3)) // Menu only
            menu_left = w - 32; 

        if (x >= menu_left && x <= menu_right)
        {
            if (menuforced && ui->main.mode == df::ui_sidebar_mode::Burrows && ui->burrows.in_define_mode)
            {
                // Make burrow symbols use graphics font
                if ((y != 12 && y != 13 && !(x == menu_left + 2 && y == 2)) || x == menu_left || x == menu_right) 
                    return has_textfont;
            }
            else
                return has_textfont;
        }

        is_map = (x > 0 && x < menu_left);

        return false;
    }

    if (!has_textfont)
        return false;
    
    if (IS_SCREEN(viewscreen_setupadventurest))
    {
        df::viewscreen_setupadventurest *s = static_cast<df::viewscreen_setupadventurest*>(ws);
        if (s->subscreen != df::viewscreen_setupadventurest::Nemesis)
            return true;
        else if (x < 58 || x >= 78 || y == 0 || y >= 21)
            return true;

        return false;
    }
    
    if (IS_SCREEN(viewscreen_choose_start_sitest))
    {
        if (y <= 1 || y >= h - 6 || x == 0 || x >= 57)
            return true;

        return false;
    }
        
    if (IS_SCREEN(viewscreen_new_regionst))
    {
        if (y <= 1 || y >= h - 2 || x <= 37 || x == w - 1)
            return true;

        return false;
    }

    if (IS_SCREEN(viewscreen_layer_export_play_mapst))
    {
        if (x == w - 1 || x < w - 23)
            return true;

        return false;
    }

    if (IS_SCREEN(viewscreen_overallstatusst))
    {
        if ((x == 46 || x == 71) && y >= 8)
            return false;

        return true;
    }

    if (IS_SCREEN(viewscreen_movieplayerst))
    {
        df::viewscreen_movieplayerst *s = static_cast<df::viewscreen_movieplayerst*>(ws);
        return !s->is_playing;
    }    

    /*if (IS_SCREEN(viewscreen_petst))
    {
        if (x == 41 && y >= 7)
            return false;

        return true;
    }*/

    //*out2 << Core::getInstance().p->readClassName(*(void**)ws) << " " << (int)ws->breakdown_level << std::endl;

    return true;
}

static void screen_to_texid_text(renderer_cool *r, int tile, struct texture_fullid &ret)
{
    const unsigned char *s = r->screen + tile*4;

    int bold = (s[3] != 0) * 8;
    int fg   = (s[1] + bold) % 16;
    int bg   = s[2] % 16;

    const long texpos = r->screentexpos[tile];

    if (!texpos)
    {
        ret.texpos = text_texpos[s[0]];

        ret.r = enabler->ccolor[fg][0];
        ret.g = enabler->ccolor[fg][1];
        ret.b = enabler->ccolor[fg][2];
        ret.br = enabler->ccolor[bg][0];
        ret.bg = enabler->ccolor[bg][1];
        ret.bb = enabler->ccolor[bg][2];

        return;
    }        

    ret.texpos = texpos;

    if (r->screentexpos_grayscale[tile])
    {
        const unsigned char cf = r->screentexpos_cf[tile];
        const unsigned char cbr = r->screentexpos_cbr[tile];

        ret.r = enabler->ccolor[cf][0];
        ret.g = enabler->ccolor[cf][1];
        ret.b = enabler->ccolor[cf][2];
        ret.br = enabler->ccolor[cbr][0];
        ret.bg = enabler->ccolor[cbr][1];
        ret.bb = enabler->ccolor[cbr][2];
    }
    else if (r->screentexpos_addcolor[tile])
    {
        ret.r = enabler->ccolor[fg][0];
        ret.g = enabler->ccolor[fg][1];
        ret.b = enabler->ccolor[fg][2];
        ret.br = enabler->ccolor[bg][0];
        ret.bg = enabler->ccolor[bg][1];
        ret.bb = enabler->ccolor[bg][2];
    }
    else
    {
        ret.r = ret.g = ret.b = 1;
        ret.br = ret.bg = ret.bb = 0;
    }
}

static void write_tile_arrays_text(renderer_cool *r, int x, int y, GLfloat *fg, GLfloat *bg, GLfloat *tex)
{
    const int tile = x * tdimy + y;

    // Fast path for transparent area where we render the map
    df::viewscreen *ws = Gui::getCurViewscreen();
    if (df::viewscreen_dwarfmodest::_identity.is_direct_instance(ws) && x > 0 && y > 0 && y < tdimy-1 && x < tdimx-gmenu_w-1)
    {
        const unsigned char *s = r->screen + tile*4;
        if (s[0] == 0)
        {
            fg[11] = fg[23] = 0;
            bg[11] = bg[23] = 0;
            return;
        }
    }
    else if (df::viewscreen_dungeonmodest::_identity.is_direct_instance(ws))
    {
        int m = df::global::ui_advmode->menu;
        bool tmode = (m == df::ui_advmode_menu::Default || m == df::ui_advmode_menu::Look || m == df::ui_advmode_menu::ThrowAim || m == df::ui_advmode_menu::Talk || m == 14 || m == df::ui_advmode_menu::Fire || m == df::ui_advmode_menu::Unk35);
        if (y < tdimy-2 && tmode)
        {
            const unsigned char *s = r->screen + tile*4;
            if (s[0] == 0)
            {
                fg[11] = fg[23] = 0;
                bg[11] = bg[23] = 0;
                return;
            }
        }
    }    

    struct texture_fullid ret;
    screen_to_texid_text(r, tile, ret);

    for (int i = 0; i < 2; i++) {
        fg += 8;
        *(fg++) = ret.r;
        *(fg++) = ret.g;
        *(fg++) = ret.b;
        *(fg++) = 1;
        
        bg += 8;
        *(bg++) = ret.br;
        *(bg++) = ret.bg;
        *(bg++) = ret.bb;
        *(bg++) = 1;
    }

    //TODO: handle special cases of graphics tiles outside of the map here with is_text_tile as before (+aux font)
    
    // Set texture coordinates
    gl_texpos *txt = (gl_texpos*) enabler->textures.gl_texpos;
    *(tex++) = txt[ret.texpos].left;   // Upper left
    *(tex++) = txt[ret.texpos].bottom;
    *(tex++) = txt[ret.texpos].right;  // Upper right
    *(tex++) = txt[ret.texpos].bottom;
    *(tex++) = txt[ret.texpos].left;   // Lower left
    *(tex++) = txt[ret.texpos].top;
    
    *(tex++) = txt[ret.texpos].left;   // Lower left
    *(tex++) = txt[ret.texpos].top;
    *(tex++) = txt[ret.texpos].right;  // Upper right
    *(tex++) = txt[ret.texpos].bottom;
    *(tex++) = txt[ret.texpos].right;  // Lower right
    *(tex++) = txt[ret.texpos].top;
}
