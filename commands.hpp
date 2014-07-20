command_result mapshot_cmd (color_ostream &out, std::vector <std::string> & parameters)
{
    if (!enabled)
        return CR_FAILURE;

    CoreSuspender suspend;

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
        std::string &param1 = parameters[0];
        int newmaxlevels = maxlevels;

        if (param1 == "shadowcolor" && pcnt >= 5)
        {
            float c[4];
            char *e;
            do {
                c[0] = strtod(parameters[1].c_str(), &e);
                if (*e != 0)
                    break;
                c[1] = strtod(parameters[2].c_str(), &e);
                if (*e != 0)
                    break;
                c[2] = strtod(parameters[3].c_str(), &e);
                if (*e != 0)
                    break;
                c[3] = strtod(parameters[4].c_str(), &e);
                if (*e != 0)
                    break;

                shadowcolor[0] = c[0], shadowcolor[1] = c[1], shadowcolor[2] = c[2], shadowcolor[3] = c[3];
            } while(0);
        }        
        else if (param1 == "fogcolor" && pcnt >= 4)
        {
            float c[3];
            char *e;
            do {
                c[0] = strtod(parameters[1].c_str(), &e);
                if (*e != 0)
                    break;
                c[1] = strtod(parameters[2].c_str(), &e);
                if (*e != 0)
                    break;
                c[2] = strtod(parameters[3].c_str(), &e);
                if (*e != 0)
                    break;

                fogcolor[0] = c[0], fogcolor[1] = c[1], fogcolor[2] = c[2];
            } while(0);
        }
        else if (param1 == "fogdensity" && pcnt >= 2)
        {
            char *e;
            float l = strtod(parameters[1].c_str(), &e);
            if (*e == 0)
                fogdensity = l;
        }
        else if (param1 == "more")
        {
            if (maxlevels < 15)
                newmaxlevels = maxlevels + 1;
        }
        else if (param1 == "less")
        {
            if (maxlevels > 0)
                newmaxlevels = maxlevels - 1;
        }        
        else 
        {
            char *e;
            int l = (int)strtol(param1.c_str(), &e, 10);
            if (*e == 0)
                newmaxlevels = std::max (std::min(l, 15), 0);
        }

        if (newmaxlevels && !maxlevels)
            patch_rendering(false);
        else if (!newmaxlevels && maxlevels)
        {
            patch_rendering(true);

            multi_rendered = false;

            // Fog coords won't be updated once multilevel rendering is off, so we need to zero all of them out
            ((renderer_cool*)enabler->renderer)->needs_full_update = true;
        }

        maxlevels = newmaxlevels;            
    }

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
        std::string &param1 = parameters[0];

        if (param1 == "tilesize" && pcnt >= 2)
        {
            renderer_cool *r = (renderer_cool*) enabler->renderer;
            std::string &param2 = parameters[1];

            if (param2 == "bigger")
            {
                r->gdispx++;
                r->gdispy++;
                r->needs_reshape = true;
            }
            else if (param2 == "smaller")
            {
                if (r->gdispx > 0 && r->gdispy > 0)
                {
                    r->gdispx--;
                    r->gdispy--;
                    r->needs_reshape = true;
                }
            }
            else if (pcnt >= 3)
            {
                int w, h;
                char *e;
                do {
                    w = strtol(parameters[1].c_str(), &e, 10);
                    if (*e != 0)
                        break;
                    h = strtol(parameters[2].c_str(), &e, 10);
                    if (*e != 0)
                        break;

                    r->gdispx = w;
                    r->gdispy = h;
                    r->needs_reshape = true;
                } while(0);
            }
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
        std::string &param1 = parameters[0];
        if (param1 == "reload")
        {
            load_colormap();
        }
        else
        {
            int cidx = color_name_to_index(param1);

            if (cidx != -1)
                out << param1 << " = " <<
                    roundf(enabler->ccolor[cidx][0]*255) << " " <<
                    roundf(enabler->ccolor[cidx][1]*255) << " " <<
                    roundf(enabler->ccolor[cidx][2]*255) << std::endl;
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
            char *e;

            do {
                c[0] = strtod(parameters[1].c_str(), &e) / 255.0;
                if (*e != 0)
                    break;
                c[1] = strtod(parameters[2].c_str(), &e) / 255.0;
                if (*e != 0)
                    break;
                c[2] = strtod(parameters[3].c_str(), &e) / 255.0;
                if (*e != 0)
                    break;

                memcpy(enabler->ccolor[cidx], c, sizeof(enabler->ccolor[cidx]));
            } while(0);
        }  
    }

    return CR_OK;    
}