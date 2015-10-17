int find_top_z()
{
    int z;
    bool ramp = false;
    for (z = 1; z < df::global::world->map.z_count; z++)
    {
        for (int bx = 0; bx < df::global::world->map.x_count_block; bx++)
        {
            for (int by = 0; by < df::global::world->map.y_count_block; by++)
            {
                df::map_block *block = world->map.block_index[bx][by][z];
                if (block)
                {
                    for (int x = 0; x < 16; x++)
                    {
                        for (int y = 0; y < 16; y++)
                        {
                            df::tiletype tt = block->tiletype[x][y];
                            df::tiletype_shape tshape = tileShape(tt);

                            if (!ramp && (tt == df::tiletype::SoilRamp ||
                                tt == df::tiletype::StoneRamp ||
                                tt == df::tiletype::MineralRamp))
                            {
                                //ramp = true;
                                goto nextz;
                            }

                            if (block->designation[x][y].bits.subterranean)
                            {
                                ramp = false;
                                goto nextz;
                            }
                        }
                    }
                }
            }            
        }

        break;

        nextz:;
    }
    done:;

    return z;
}

command_result mapshot_cmd (color_ostream &out, std::vector <std::string> & parameters)
{
    if (!enabled)
        return CR_FAILURE;

    for (int z = 10; z < df::global::world->map.z_count; z++)
    {
        for (int bx = 0; bx < df::global::world->map.x_count_block; bx++)
        {
            for (int by = 0; by < df::global::world->map.y_count_block; by++)
            {
                df::map_block *block = world->map.block_index[bx][by][z];
                if (block)
                {
                    memset(block->fog_of_war, 0x00, 16*16);

                    for (int x = 0; x < 16; x++)
                    {
                        for (int y = 0; y < 16; y++)
                        {
                            block->designation[x][y].bits.hidden = false;
                            block->designation[x][y].bits.pile = true;
                        }
                    }                    
                }
            }
        }
    }    

    int z = find_top_z();
    *out2 << " z = " << z << std::endl;
    df::global::world->units.active[0]->pos.z = z;

    domapshot = 10;

    return CR_OK;    
}

command_result multilevel_cmd (color_ostream &out, std::vector <std::string> & parameters)
{
    if (!enabled)
        return CR_FAILURE;

    CoreSuspender suspend;

    int pcnt = parameters.size();

    if (pcnt >= 1)
    {
        std::string &param0 = parameters[0];
        int newmaxlevels = maxlevels;

        if (param0 == "shadowcolor" && pcnt >= 5)
        {
            float c[4];
            bool ok = parse_float(parameters[1], c[0]) &&
                      parse_float(parameters[2], c[1]) &&
                      parse_float(parameters[3], c[2]) &&
                      parse_float(parameters[4], c[3]);

            if (ok)
                memcpy(shadowcolor, c, sizeof(shadowcolor));
            else
                return CR_WRONG_USAGE;
        }        

        else if (param0 == "fogcolor" && pcnt >= 4)
        {
            float c[3];
            bool ok = parse_float(parameters[1], c[0]) &&
                      parse_float(parameters[2], c[1]) &&
                      parse_float(parameters[3], c[2]);

            if (ok)
                memcpy(fogcolor, c, sizeof(fogcolor));
            else
                return CR_WRONG_USAGE;
        }

        else if (param0 == "fogdensity" && pcnt >= 2)
        {
            float val[3];
            bool ok;

            ok = parse_float(parameters[1], val[0]);
            if (pcnt >= 3)
                ok &= parse_float(parameters[2], val[1]);
            if (pcnt >= 4)
                ok &= parse_float(parameters[3], val[2]);

            if (!ok)
                return CR_WRONG_USAGE;                

            fogdensity = val[0];
            if (pcnt >= 3)
                fogstart = val[1];
            else
                fogstart = 0;
            if (pcnt >= 4)
                fogstep = val[2];
            else
                fogstep = 1;

            ((renderer_cool*)enabler->renderer)->needs_full_update = true;
        }

        else if (param0 == "more")
        {
            if (maxlevels < 127)
                newmaxlevels = maxlevels + 1;
        }
        else if (param0 == "less")
        {
            if (maxlevels > 0)
                newmaxlevels = maxlevels - 1;
        }        
        else 
        {
            int val;
            if (parse_int(param0, val))
                newmaxlevels = std::max (std::min(val, 127), 0);
            else
                return CR_WRONG_USAGE;
        }

        if (newmaxlevels && !maxlevels)
        {
            if (!legacy_mode)
                patch_rendering(false);

            ((renderer_cool*)enabler->renderer)->needs_full_update = true;
        }
        else if (!newmaxlevels && maxlevels)
        {
            if (!legacy_mode)
                patch_rendering(true);

            multi_rendered = false;

            ((renderer_cool*)enabler->renderer)->needs_full_update = true;
        }

        maxlevels = newmaxlevels;            
    }
    else
        return CR_WRONG_USAGE;

    return CR_OK;    
}

command_result twbt_cmd (color_ostream &out, std::vector <std::string> & parameters)
{
    if (!enabled)
        return CR_FAILURE;

    CoreSuspender suspend;

    int pcnt = parameters.size();

    if (pcnt >= 1)
    {
        std::string &param0 = parameters[0];

        if (param0 == "tilesize")
        {
            renderer_cool *r = (renderer_cool*) enabler->renderer;
            std::string &param2 = parameters[1];

            if (pcnt == 1)
            {
                *out2 << "Map tile size is " << r->gdispx << "x" << r->gdispy << std::endl;
                return CR_OK;
            }

            if (parameters[1] == "bigger")
            {
                r->gdispx++;
                r->gdispy++;

                r->needs_reshape = true;
            }
            else if (parameters[1] == "smaller")
            {
                if (r->gdispx > 1 && r->gdispy > 1 && (r->gdimxfull < world->map.x_count || r->gdimyfull < world->map.y_count))
                {
                    r->gdispx--;
                    r->gdispy--;

                    r->needs_reshape = true;
                }
            }

            else if (parameters[1] == "reset")
            {
                r->gdispx = enabler->fullscreen ? small_map_dispx : large_map_dispx;
                r->gdispy = enabler->fullscreen ? small_map_dispy : large_map_dispy;

                r->needs_reshape = true;                    
            }

            else if (parameters[1][0] == '+' || parameters[1][0] == '-')
            {
                int delta;
                if (!parse_int(parameters[1], delta))
                    return CR_WRONG_USAGE;

                r->gdispx += delta;
                r->gdispy += delta;

                r->needs_reshape = true;
            }

            else if (pcnt >= 3)
            {
                int w, h;
                bool ok = parse_int(parameters[1], w) &&
                          parse_int(parameters[2], h);

                if (ok)
                {
                    r->gdispx = w;
                    r->gdispy = h;
                    r->needs_reshape = true;
                }
                else
                    return CR_WRONG_USAGE;   

            }
            else
                return CR_WRONG_USAGE;
        }
    }

    return CR_OK;    
}

command_result colormap_cmd (color_ostream &out, std::vector <std::string> & parameters)
{
    if (!enabled)
        return CR_FAILURE;

    CoreSuspender suspend;

    int pcnt = parameters.size();

    if (pcnt == 1)
    {
        std::string &param0 = parameters[0];
        if (param0 == "reload")
        {
            load_colormap();
        }
        else
        {
            int cidx = color_name_to_index(param0);

            if (cidx != -1)
                out << param0 << " = " <<
                    roundf(enabler->ccolor[cidx][0] * 255) << " " <<
                    roundf(enabler->ccolor[cidx][1] * 255) << " " <<
                    roundf(enabler->ccolor[cidx][2] * 255) << std::endl;
        }

        gps->force_full_display_count = 1;
        ((renderer_cool*)enabler->renderer)->needs_full_update = true;
    }
    else if (pcnt == 4)
    {
        int cidx = color_name_to_index(parameters[0]);

        if (cidx != -1)
        {
            float c[3];
            bool ok = parse_float(parameters[1], c[0]) &&
                      parse_float(parameters[2], c[1]) &&
                      parse_float(parameters[3], c[2]);

            if (ok)
            {
                c[0] /= 255.0f, c[1] /= 255.0f, c[2] /= 255.0f;
                memcpy(enabler->ccolor[cidx], c, sizeof(enabler->ccolor[cidx]));

                gps->force_full_display_count = 1;
                ((renderer_cool*)enabler->renderer)->needs_full_update = true; 
            }
            else
                return CR_WRONG_USAGE;            
        }  
    }

    return CR_OK;    
}

typedef void (*TTT)(void *world, short rx, short ry, short ex, short ey, int unk1, int unk2, void *unk3);
typedef void (*QQQ)(void *world);

int _rx;
int _ry;
int _ex;
int _ey;

command_result ttt_cmd (color_ostream &out, std::vector <std::string> & parameters)
{
#ifndef __APPLE__
    TTT ttt = (TTT)0x8de5e30;    
#else
    TTT ttt = (TTT)0xd43a10;
#endif

    int kk[] = { 8, -1, 0, -1, -1 };
    int kk2[] = { 0, -1, 0, -1, 0x18 };

    int rx;
    int ry;
    int ex;
    int ey;
    if (parameters.size())
    {
        rx = atoi(parameters[0].c_str());
        ry = atoi(parameters[1].c_str());
        ex = atoi(parameters[2].c_str());
        ey = atoi(parameters[3].c_str());
    }
    else
    {
        rx = _rx;
        ry = _ry;
        ex = _ex;
        ey = _ey;
    }

    MemoryPatcher p(Core::getInstance().p);
#ifndef __APPLE__
    p.makeWritable((void*)0x8dad12a, 5);
    memset((void*)0x8dad12a, 0x90, 5);
#else
    p.makeWritable((void*)0xd0681b, 5);
    memset((void*)0xd06817, 0x90, 5);
#endif

    ttt(df::global::world, rx,ry,ex,ey,   1, 0,   kk);

    df::global::world->units.active[0]->pos.x = 10;
    df::global::world->units.active[0]->pos.y = 10;
    df::global::world->world_data->unk_x1 = 0;
    df::global::world->world_data->unk_x2 = 144;
    df::global::world->world_data->unk_y1 = 0;
    df::global::world->world_data->unk_y2 = 144;
    
    *df::global::window_z = find_top_z();

    return CR_OK;
}

command_result qqq_cmd (color_ostream &out, std::vector <std::string> & parameters)
{
#ifndef __APPLE__
    QQQ qqq = (QQQ)0x8de7320;
#else
    //QQQ qqq = (QQQ)0x34edd0;
    QQQ qqq = (QQQ)0xd454f0;
    //QQQ qqq = (QQQ)0xd42e40;
#endif

    qqq(df::global::world);
    return CR_OK;
}

command_result www_cmd (color_ostream &out, std::vector <std::string> & parameters)
{
    CoreSuspender suspend;
    qqq_cmd(out, parameters);

    {
        *df::global::building_next_id = 1;
        *df::global::item_next_id = 1;
        *df::global::unit_next_id = 1;
        /*<global-address name='activity_next_id' value='0x01bad92c'/>
        <global-address name='agreement_next_id' value='0x01bad958'/>
        <global-address name='army_controller_next_id' value='0x01bad94c'/>
        <global-address name='army_next_id' value='0x01bad948'/>
        <global-address name='army_tracking_info_next_id' value='0x01bad950'/>
        <global-address name='art_image_chunk_next_id' value='0x01bad91c'/>
        <global-address name='artifact_next_id' value='0x01bad8f0'/>
        <global-address name='building_next_id' value='0x01bad900'/>
        <global-address name='crime_next_id' value='0x01bad940'/>
        <global-address name='cultural_identity_next_id' value='0x01bad954'/>
        <global-address name='entity_next_id' value='0x01bad8e8'/>
        <global-address name='flow_guide_next_id' value='0x01bad908'/>
        <global-address name='formation_next_id' value='0x01bad928'/>
        <global-address name='hist_event_collection_next_id' value='0x01bad914'/>
        <global-address name='hist_event_next_id' value='0x01bad910'/>
        <global-address name='hist_figure_next_id' value='0x01bad90c'/>
        <global-address name='identity_next_id' value='0x01bad938'/>
        <global-address name='incident_next_id' value='0x01bad93c'/>
        <global-address name='interaction_instance_next_id' value='0x01bad930'/>
        <global-address name='item_next_id' value='0x01bad8dc'/>
        <global-address name='job_next_id' value='0x01bad8f4'/>
        <global-address name='machine_next_id' value='0x01bad904'/>
        <global-address name='nemesis_next_id' value='0x01bad8ec'/>
        <global-address name='proj_next_id' value='0x01bad8fc'/>
        <global-address name='schedule_next_id' value='0x01bad8f8'/>
        <global-address name='squad_next_id' value='0x01bad924'/>
        <global-address name='unit_chunk_next_id' value='0x01bad918'/>
        <global-address name='unit_next_id' value='0x01bad8e0'/>
        <global-address name='vehicle_next_id' value='0x01bad944'/>
        <global-address name='written_content_next_id' value='0x01bad934'/>*/
    }

    ttt_cmd(out, parameters);


    {
        renderer_cool *r = (renderer_cool*)enabler->renderer;
        r->gdimx = r->gdimxfull = world->map.x_count;
        r->gdimy = r->gdimyfull = world->map.y_count;

        //if (df::viewscreen_dwarfmodest::_identity.is_direct_instance(ws))
            r->goff_x = r->goff_y_gl = 0;

        int tiles = r->gdimx * r->gdimy;

        // Recreate tile buffers
        r->allocate_buffers(tiles);

        // Recreate OpenGL buffers
        r->gvertexes = (GLfloat*)realloc(r->gvertexes, sizeof(GLfloat) * tiles * 2 * 6);
        r->gfg = (GLfloat*)realloc(r->gfg, sizeof(GLfloat) * tiles * 4 * 6);
        r->gbg = (GLfloat*)realloc(r->gbg, sizeof(GLfloat) * tiles * 4 * 6);
        r->gtex = (GLfloat*)realloc(r->gtex, sizeof(GLfloat) * tiles * 2 * 6);

        // Initialise vertex coords
        int tile = 0;   
        for (GLfloat x = 0; x < r->gdimx; x++)
            for (GLfloat y = 0; y < r->gdimy; y++, tile++)
                write_tile_vertexes(x, y, r->gvertexes + 6 * 2 * tile);

        r->needs_full_update = true;


        *df::global::window_x = 0;
        *df::global::window_y = 0;
        gps->force_full_display_count = 1;
    }    

    df::viewscreen *ws = Gui::getCurViewscreen();
    for (int i = 0; i < 3; i++)
    {
        ws->logic();
        ws->render();
    }
    mapshot_cmd(out, parameters);
    return CR_OK;
}

typedef void (*PPP)(void *world, int unload, void *coords);

command_result ppp_cmd (color_ostream &out, std::vector <std::string> & parameters)
{
    std::vector <std::string> q;
    int x1 = atoi(parameters[0].c_str());
    int x2 = atoi(parameters[1].c_str());
    int y1 = atoi(parameters[2].c_str());
    int y2 = atoi(parameters[3].c_str());

    /*x1 /= 3;
    x1 *= 3;
    y1 /= 3;
    y1 *= 3;*/

    for (int y = y1; y <= y2; y+=3)
    {
        for (int x = x1; x <= x2; x+=3)
        {
            _rx = x / 16;
            _ry = y / 16;
            _ex = x % 16+1;
            _ey = y % 16+1;
    
            *out2 << x << " " << y << std::endl; 
            www_cmd(out, q);
            while(domapshot);
        }        
    }

    return CR_OK;

    while(df::global::world->map.region_x < 500){
    {
        CoreSuspender suspend;

     /*   PPP ppp = (PPP)0xd42e40;


        int x1 = atoi(parameters[0].c_str());
        int x2 = atoi(parameters[1].c_str());
        int y1 = atoi(parameters[2].c_str());
        int y2 = atoi(parameters[3].c_str());

        int kk[] = { x1, x2, y1, y2 };

        *out2 << "ppp 1" << std::endl;
        //ppp(df::global::world, 0, 0);
        *out2 << "ppp 2" << std::endl;
        ppp(df::global::world, 1, kk);
        *out2 << "ppp 3" << std::endl;

        *out2 << df::global::world->map.region_x << std::endl;*/

        df::global::world->units.active[0]->pos.x = 150;
        df::viewscreen *ws = Gui::getCurViewscreen();
        for (int i = 0; i < 3; i++)
        {
            ws->logic();
            ws->render();
        }

        mapshot_cmd(out, parameters);
    }
    while(domapshot);

    *out2 << "done" << std::endl;
}
    return CR_OK;
}