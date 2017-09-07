static void screen_to_texid_text(renderer_cool *r, int tile, struct texture_fullid &ret)
{
    const unsigned char *s = r->screen + tile*4;

    int bold = (s[3] != 0) * 8;
    int fg   = (s[1] + bold) % 16;
    int bg   = s[2] % 16;

    const long texpos = ((long*)r->screentexpos)[tile];

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
        bool tmode = advmode_needs_map(m);
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
