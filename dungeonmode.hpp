/*struct zzz2 : public df::viewscreen_dungeonmodest
{
    typedef df::viewscreen_dungeonmodest interpose_base;

    DEFINE_VMETHOD_INTERPOSE(void, feed, (std::set<df::interface_key> *input))
    {
        renderer_cool *r = (renderer_cool*)enabler->renderer;

        int oldgridx = init->display.grid_x;
        int oldgridy = init->display.grid_y;


        static uint8_t menu_width_last, area_map_width_last;
        static bool menuforced_last=0;

        int32_t w = gps->dimx, h = gps->dimy;
        uint8_t menu_width, area_map_width;
        Gui::getMenuWidth(menu_width, area_map_width);
        int32_t menu_w = 0;

        bool menuforced = (ui->main.mode != df::ui_sidebar_mode::Default || df::global::cursor->x != -30000);

        init->display.grid_x = r->gdimxfull+menu_w+2;
        init->display.grid_y = r->gdimyfull+2;
        gps->dimx = r->gdimxfull+menu_w+2;
        gps->dimy = r->gdimyfull+2;


        INTERPOSE_NEXT(feed)(input);
        init->display.grid_x = gps->dimx = oldgridx;
        init->display.grid_y = gps->dimy = oldgridy;

        bool menuforced_new = (ui->main.mode != df::ui_sidebar_mode::Default || df::global::cursor->x != -30000);
        if (menuforced != menuforced_new)
        {
            gmenu_w = 0;

            r->needs_reshape = true;
            gps->force_full_display_count = 1;            
        }
    }

    DEFINE_VMETHOD_INTERPOSE(void, render, ())
    {
        INTERPOSE_NEXT(render)();

#ifdef WIN32
        static bool patched = false;
        if (!patched)
        {
        unsigned char t1[] = {  0x90,0x90, 0x90, 0x90,0x90,0x90,0x90,0x90 };
        Core::getInstance().p->patchMemory((void*)(0x0058eabd+(Core::getInstance().vinfo->getRebaseDelta())), t1, sizeof(t1));

//          unsigned char t1[] = { 0x90,0x90,0x90,0x90,0x90 };
//            Core::getInstance().p->patchMemory((void*)(0x0058eac0+(Core::getInstance().vinfo->getRebaseDelta())), t1, sizeof(t1));
            patched = true;
        }
#endif

        renderer_cool *r = (renderer_cool*)enabler->renderer;

        if (r->needs_reshape)
        {
            if (r->needs_zoom)
            {
            if (r->needs_zoom > 0)
            {
                r->gdispx++;
                r->gdispy++;
            }
            else
            {
                r->gdispx--;
                r->gdispy--;

                if (r->gsize_x / r->gdispx > world->map.x_count)
                    r->gdispx = r->gdispy = r->gsize_x / world->map.x_count;
                else if (r->gsize_y / r->gdispy > world->map.y_count)
                    r->gdispx = r->gdispy = r->gsize_y / world->map.y_count;
            }
            r->needs_zoom = 0;
        }
        r->needs_reshape = false;
        r->reshape_graphics();
    }

#ifdef WIN32
        void (*render_map)(int) = (void (*)(int))(0x008f65c0+(Core::getInstance().vinfo->getRebaseDelta()));
#else
        void (*render_map)(void *, int) = (void (*)(void *, int))0x0084b4c0;
#endif

        uint8_t *sctop = enabler->renderer->screen;
        int32_t *screentexpostop = enabler->renderer->screentexpos;
        int8_t *screentexpos_addcolortop = enabler->renderer->screentexpos_addcolor;
        uint8_t *screentexpos_grayscaletop = enabler->renderer->screentexpos_grayscale;
        uint8_t *screentexpos_cftop = enabler->renderer->screentexpos_cf;
        uint8_t *screentexpos_cbrtop = enabler->renderer->screentexpos_cbr;

        gps->screen = enabler->renderer->screen = gscreen;
        gps->screen_limit = gps->screen + r->gdimx * r->gdimy * 4;
        gps->screentexpos = enabler->renderer->screentexpos = screentexpos2;
        gps->screentexpos_addcolor = enabler->renderer->screentexpos_addcolor = screentexpos_addcolor2;
        gps->screentexpos_grayscale = enabler->renderer->screentexpos_grayscale = screentexpos_grayscale2;
        gps->screentexpos_cf = enabler->renderer->screentexpos_cf = screentexpos_cf2;
        gps->screentexpos_cbr = enabler->renderer->screentexpos_cbr = screentexpos_cbr2;

        long *z = (long*)r->screen;
        for (int y = 0; y < r->gdimy; y++)
        {
            for (int x = world->map.x_count-*df::global::window_x; x < r->gdimx; x++)
            {
                z[x*r->gdimy+y] = 0;
            }
        }
        for (int x = 0; x < r->gdimx; x++)
        {
            for (int y = world->map.y_count-*df::global::window_y; y < r->gdimy; y++)
            {
                z[x*r->gdimy+y] = 0;
            }
        }

        int oldgridx = init->display.grid_x;
        int oldgridy = init->display.grid_y;

        init->display.grid_x = r->gdimx;
        init->display.grid_y = r->gdimy;
        gps->dimx = r->gdimx;
        gps->dimy = r->gdimy;
        gps->clipx[1] = r->gdimx-1;
        gps->clipy[1] = r->gdimy-1;

#ifdef WIN32
        render_map(1);
#else
#ifdef DFHACK_r5
        render_map(df::global::map_renderer, 1);
#else
        render_map(df::global::cursor_unit_list, 1);
#endif
#endif



  if (shadowsloaded)
        {




        uint8_t *sctop = enabler->renderer->screen;
        int32_t *screentexpostop = enabler->renderer->screentexpos;
        int8_t *screentexpos_addcolortop = enabler->renderer->screentexpos_addcolor;
        uint8_t *screentexpos_grayscaletop = enabler->renderer->screentexpos_grayscale;
        uint8_t *screentexpos_cftop = enabler->renderer->screentexpos_cf;
        uint8_t *screentexpos_cbrtop = enabler->renderer->screentexpos_cbr;

        gps->screen = screen3;
        gps->screen_limit = gps->screen + r->gdimx * r->gdimy * 4;
        gps->screentexpos = screentexpos3;
        gps->screentexpos_addcolor = screentexpos_addcolor3;
        gps->screentexpos_grayscale = screentexpos_grayscale3;
        gps->screentexpos_cf = screentexpos_cf3;
        gps->screentexpos_cbr = screentexpos_cbr3;

        //this->*this->interpose_render.get_first_interpose(&df::viewscreen_dwarfmodest::_identity).saved_chain;

        //void (*render_map)(void *, int) = (void (*)(void *, int))0x0084b4c0;

        bool empty_tiles_left;
        int p = 1;
        int x0 = 0;
        int zz0 = *df::global::window_z;        
        do
        {
            //TODO: if z=0 should just render and use for all tiles always
            if (*df::global::window_z == 0)
                break;

            (*df::global::window_z)--;

            (*df::global::window_x) += x0;
            //init->display.grid_x -= x0-1;

#ifdef WIN32
        render_map(1);
#else
#ifdef DFHACK_r5
        render_map(df::global::map_renderer, 1);
#else
        render_map(df::global::cursor_unit_list, 1);
#endif
#endif
        
            (*df::global::window_x) -= x0;
            //init->display.grid_x += x0-1;

            empty_tiles_left = false;
            int x00 = x0;
            int zz = zz0 - p + 1;

            //*out2 << p << " " << x0 << std::endl;
            
            GLfloat *vertices = ((renderer_opengl*)enabler->renderer)->vertexes;
            //TODO: test this
            int x1 = std::min(r->gdimx, world->map.x_count-*df::global::window_x);
            int y1 = std::min(r->gdimy, world->map.y_count-*df::global::window_y);
            for (int x = x0; x < x1; x++)
            {
                for (int y = 0; y < y1; y++)
                {
                    const int tile = x * r->gdimy + y;
                    const int tile2 = (x-(x00)) * r->gdimy + y;

                    if ((sctop[tile*4+3]&0xf0))
                        continue;

                    unsigned char ch = sctop[tile*4+0];
                    if (ch != 31 && ch != 249 && ch != 250 && ch != 254 && ch != skytile && ch != chasmtile && !(ch >= '1' && ch <= '7'))
                        continue;

                    int xx = *df::global::window_x + x;
                    int yy = *df::global::window_y + y;
                    if (xx < 0 || yy < 0)
                        continue;

                    //TODO: check for z=0
                    bool e0,h,h0;
                    //*out2 << xx << " " << world->map.x_count << " " << yy << " " << world->map.y_count << " " << *df::global::window_x << " " << *df::global::window_y << std::endl;
                    df::map_block *block0 = world->map.block_index[xx >> 4][yy >> 4][zz0];
                    h0 = block0 && block0->designation[xx&15][yy&15].bits.hidden;
                    if (h0)
                        continue;
                    e0 = !block0 || (block0->tiletype[xx&15][yy&15] == df::tiletype::OpenSpace || block0->tiletype[xx&15][yy&15] == df::tiletype::RampTop);
                    if (!(e0))
                        continue;

                    int d=p;
                    ch = screen3[tile2*4+0];
                    if (!(ch!=31&&ch != 249 && ch != 250 && ch != 254 && ch != skytile && ch != chasmtile && !(ch >= '1' && ch <= '7')))
                    {
                        df::map_block *block1 = world->map.block_index[xx >> 4][yy >> 4][zz-1];
                        if (!block1)
                        {
                            //TODO: skip all other y's in this block
                            if (p < maxlevels)
                            {
                                empty_tiles_left = true;
                                continue;
                            }
                            else
                                d = p+1;
                        }
                        else
                        {
                            //TODO: check for hidden also
                            df::tiletype t1 = block1->tiletype[xx&15][yy&15];
                            if (t1 == df::tiletype::OpenSpace || t1 == df::tiletype::RampTop)
                            {
                                if (p < maxlevels)
                                {
                                    empty_tiles_left = true;
                                    continue;
                                }
                                else
                                {
                                    if (t1 != df::tiletype::RampTop)
                                        d = p+1;
                                }
                            }
                        }
                    }

                    //*out2 << p << " !" << std::endl;
                    *((int*)gscreen+tile) = *((int*)screen3+tile2);
                    if (*(screentexpos3+tile2))
                    {
                        *(screentexpostop+tile) = *(screentexpos3+tile2);
                        *(screentexpos_addcolortop+tile) = *(screentexpos_addcolor3+tile2);
                        *(screentexpos_grayscaletop+tile) = *(screentexpos_grayscale3+tile2);
                        *(screentexpos_cftop+tile) = *(screentexpos_cf3+tile2);
                        *(screentexpos_cbrtop+tile) = *(screentexpos_cbr3+tile2);
                    }
                    sctop[tile*4+3] = (0x10*d) | (sctop[tile*4+3]&0x0f);
                }
                if (!empty_tiles_left)
                    x0 = x + 1;
            }

            if (p++ >= maxlevels)
                break;
        } while(empty_tiles_left);

        (*df::global::window_z) = zz0;







}









        init->display.grid_x = gps->dimx = oldgridx;
        init->display.grid_y = gps->dimy = oldgridy;
        gps->clipx[1] = gps->dimx-1;
        gps->clipy[1] = gps->dimy-1;


        gps->screen = enabler->renderer->screen = sctop;
        gps->screen_limit = gps->screen + gps->dimx * gps->dimy * 4;
        gps->screentexpos = enabler->renderer->screentexpos = screentexpostop;
        gps->screentexpos_addcolor = enabler->renderer->screentexpos_addcolor = screentexpos_addcolortop;
        gps->screentexpos_grayscale = enabler->renderer->screentexpos_grayscale = screentexpos_grayscaletop;
        gps->screentexpos_cf = enabler->renderer->screentexpos_cf = screentexpos_cftop;
        gps->screentexpos_cbr = enabler->renderer->screentexpos_cbr = screentexpos_cbrtop;

    }
};*/

//IMPLEMENT_VMETHOD_INTERPOSE(zzz2, render);
//IMPLEMENT_VMETHOD_INTERPOSE(zzz2, feed);
