static void screen_to_texid_map(renderer_cool *r, int tile, struct texture_fullid &ret)
{
    const unsigned char *s = gscreen + tile*4;

    int bold = (s[3] & 0x0f) * 8;
    int fg   = (s[1] + bold) % 16;
    int bg   = s[2] % 16;

    const long texpos = gscreentexpos[tile];

    if (!texpos)
    {
        ret.texpos = map_texpos[s[0]];

        ret.r = enabler->ccolor[fg][0];
        ret.g = enabler->ccolor[fg][1];
        ret.b = enabler->ccolor[fg][2];
        ret.br = enabler->ccolor[bg][0];
        ret.bg = enabler->ccolor[bg][1];
        ret.bb = enabler->ccolor[bg][2];

        return;
    }        

    ret.texpos = texpos;

    if (gscreentexpos_grayscale[tile])
    {
        const unsigned char cf = gscreentexpos_cf[tile];
        const unsigned char cbr = gscreentexpos_cbr[tile];

        ret.r = enabler->ccolor[cf][0];
        ret.g = enabler->ccolor[cf][1];
        ret.b = enabler->ccolor[cf][2];
        ret.br = enabler->ccolor[cbr][0];
        ret.bg = enabler->ccolor[cbr][1];
        ret.bb = enabler->ccolor[cbr][2];
    }
    else if (gscreentexpos_addcolor[tile])
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

static void write_tile_arrays_map(renderer_cool *r, int x, int y, GLfloat *fg, GLfloat *bg, GLfloat *tex)
{
    struct texture_fullid ret;
    const int tile = x * r->gdimy + y;        
    screen_to_texid_map(r, tile, ret);

    for (int i = 0; i < 6; i++) {
        *(fg++) = ret.r;
        *(fg++) = ret.g;
        *(fg++) = ret.b;
        *(fg++) = 1;
        
        *(bg++) = ret.br;
        *(bg++) = ret.bg;
        *(bg++) = ret.bb;
        *(bg++) = 1;
    }
    
    if (has_overrides)
    {
        const unsigned char *s = gscreen + tile*4;
        int s0 = s[0];

        if (overrides[s0])
        {
            int xx = *df::global::window_x + x;
            int yy = *df::global::window_y + y;

            if (s0 == 88 && df::global::cursor->x == xx && df::global::cursor->y == yy)
            {
                long texpos = enabler->fullscreen ? cursor_large_texpos : cursor_small_texpos;
                if (texpos)
                    ret.texpos = texpos;
            }
            else
            {
                int zz = *df::global::window_z - ((s[3]&0xf0)>>4);
                bool matched = false;
                int tiletype = -1;

                // Items / buildings
                for (int j = 0; j < overrides[s0]->size(); j++)
                {
                    struct override &o = (*overrides[s0])[j];

                    if (o.kind == 'B')
                    {
                        if (o.subtype == -2)
                            continue;

                        auto ilist = world->buildings.other[o.id];
                        for (auto it = ilist.begin(); it != ilist.end(); it++)
                        {
                            df::building *bld = *it;
                            if (zz != bld->z || xx < bld->x1 || xx > bld->x2 || yy < bld->y1 || yy > bld->y2)
                                continue;
                            if (o.type != -1 && bld->getType() != o.type)
                                continue;
                            
                            if (o.subtype != -1)
                            {
                                int subtype = (o.id == buildings_other_id::WORKSHOP_CUSTOM || o.id == buildings_other_id::FURNACE_CUSTOM) ?
                                    bld->getCustomType() : bld->getSubtype();

                                if (subtype != o.subtype)
                                    continue;
                            }

                            ret.texpos = enabler->fullscreen ? o.large_texpos : o.small_texpos;

                            matched = true;
                            break;
                        }
                    }
                    else if (o.kind == 'I')
                    {
                        auto ilist = world->items.other[o.id];
                        for (auto it = ilist.begin(); it != ilist.end(); it++)
                        {
                            df::item *item = *it;
                            if (!(zz == item->pos.z && xx == item->pos.x && yy == item->pos.y))
                                continue;
                            if (item->flags.whole & bad_item_flags.whole)
                                continue;
                            if (o.type != -1 && item->getType() != o.type)
                                continue;
                            if (o.subtype != -1 && item->getSubtype() != o.subtype)
                                continue;

                            ret.texpos = enabler->fullscreen ? o.large_texpos : o.small_texpos;

                            matched = true;
                            break;
                        }
                    }
                    else //if (o.kind == 'T')
                    {
                        if (tiletype == -1)
                        {
                            df::map_block *block = world->map.block_index[xx>>4][yy>>4][zz];
                            tiletype = block->tiletype[xx&15][yy&15];
                        }

                        if (tiletype == o.type)
                        {
                            ret.texpos = enabler->fullscreen ? o.large_texpos : o.small_texpos;

                            matched = true;
                            break;                        
                        }
                    }
                }
            }
        }
    }
    
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