static bool parse_int(std::string &str, int &ret, int base=10)
{
    char *e;
    ret = strtol(str.c_str(), &e, base);
    return (*e == 0);
}

static bool parse_float(std::string &str, float &ret)
{
    char *e;
    ret = strtod(str.c_str(), &e);
    return (*e == 0);
}

template <class T>
static bool parse_enum_or_int(std::string &str, int &ret, int def=-1)
{
    T val;

    if (str.length())
    {
        if (!parse_int(str, ret))
        {  
            if (find_enum_item(&val, str))
                ret = val;
            else
                return false;
        }
    }
    else
        ret = def;

    return true;
}

static vector<string> split(const char *str, char c = ' ')
{
    vector<string> result;

    do
    {
        const char *begin = str;

        while(*str != c && *str)
            str++;

        result.push_back(string(begin, str));
    } while (0 != *str++);

    return result;
}

static void load_tileset_layers(tileset &ts, string &path)
{
    string ext = path.substr(path.length()-4);
    string base = path.substr(0, path.length()-4);
    
    long dx, dy;        
    struct stat buf;
 
    string bg = base+"-bg"+ext;
    if (stat(bg.c_str(), &buf) == 0)
    {
        load_tileset(bg, ts.bg_texpos, 16, 16, &dx, &dy);
    }
    else
    {
        for (int i = 0; i < 256; i++)
            ts.bg_texpos[i] = white_texpos;
    }

    string top = base+"-top"+ext;
    if (stat(top.c_str(), &buf) == 0)
    {
        load_tileset(top, ts.top_texpos, 16, 16, &dx, &dy);
    }
    else
    {        
        for (int i = 0; i < 256; i++)
            ts.top_texpos[i] = transparent_texpos;
    }
}

static bool load_map_font()
{
    string small_font_path, gsmall_font_path;
    string large_font_path, glarge_font_path;

    std::ifstream fseed("data/init/init.txt");
    if(fseed.is_open())
    {
        string str;

        while(std::getline(fseed,str))
        {
            size_t b = str.find("[");
            size_t e = str.rfind("]");

            if (b == string::npos || e == string::npos || str.find_first_not_of(" ") < b)
                continue;

            str = str.substr(b+1, e-1);
            vector<string> tokens = split(str.c_str(), ':');

            if (tokens.size() != 2)
                continue;
                                
            if(tokens[0] == "FONT")
            {
                small_font_path = "data/art/" + tokens[1];
                continue;
            }

            if(tokens[0] == "FULLFONT")
            {
                large_font_path = "data/art/" + tokens[1];
                continue;
            }

            if(tokens[0] == "GRAPHICS_FONT")
            {
                gsmall_font_path = "data/art/" + tokens[1];
                continue;
            }

            if(tokens[0] == "GRAPHICS_FULLFONT")
            {
                glarge_font_path = "data/art/" + tokens[1];
                continue;
            }                    
        }

        fseed.close();
    }
    
    //Map tileset - accessible at index 0
    if (!(small_font_path == gsmall_font_path && large_font_path == glarge_font_path))
    {
        struct tileset ts;

        long dx, dy;
        load_tileset(gsmall_font_path, (long*)ts.small_texpos, 16, 16, &dx, &dy);
        load_tileset_layers(ts, gsmall_font_path);

        small_map_dispx = dx;
        small_map_dispy = dy;

        tilesets.push_back(ts);        
        return true;
    }

    return false;
}

static bool load_text_font()
{
    string small_font_path, gsmall_font_path;
    string large_font_path, glarge_font_path;

    std::ifstream fseed("data/init/init.txt");
    if(fseed.is_open())
    {
        string str;

        while(std::getline(fseed,str))
        {
            size_t b = str.find("[");
            size_t e = str.rfind("]");

            if (b == string::npos || e == string::npos || str.find_first_not_of(" ") < b)
                continue;

            str = str.substr(b+1, e-1);
            vector<string> tokens = split(str.c_str(), ':');

            if (tokens.size() != 2)
                continue;
                                
            if(tokens[0] == "FONT")
            {
                small_font_path = "data/art/" + tokens[1];
                continue;
            }

            if(tokens[0] == "FULLFONT")
            {
                large_font_path = "data/art/" + tokens[1];
                continue;
            }

            if(tokens[0] == "GRAPHICS_FONT")
            {
                gsmall_font_path = "data/art/" + tokens[1];
                continue;
            }

            if(tokens[0] == "GRAPHICS_FULLFONT")
            {
                glarge_font_path = "data/art/" + tokens[1];
                continue;
            }                    
        }

        fseed.close();
    }

    // Also load layers for map tileset, since we read its filename
    load_tileset_layers(tilesets[0], gsmall_font_path);
    
    //Text tileset - accessible at index 1
    if (!(small_font_path == gsmall_font_path && large_font_path == glarge_font_path))
    {
        struct tileset ts;

        if (legacy_mode)
        {
            long dx, dy;

            load_tileset(small_font_path, (long*)ts.small_texpos, 16, 16, &dx, &dy);
        }
        else
        {
            // Load text font and set it as the main (and only) font in `init` structure
            load_tileset(small_font_path, (long*)init->font.small_font_texpos, 16, 16, (long*)&init->font.small_font_dispx, (long*)&init->font.small_font_dispy);
            memcpy(ts.small_texpos, init->font.small_font_texpos, sizeof(ts.small_texpos));

            // Use small font for large too
            memcpy(init->font.large_font_texpos, init->font.small_font_texpos, sizeof(init->font.large_font_texpos));
            init->font.large_font_dispx = init->font.small_font_dispx;
            init->font.large_font_dispy = init->font.small_font_dispy;
        }

        tilesets.push_back(ts);        
        return true;
    }

    return false;
}

// Either of
// tile:B|I:id:type:subtype:tileset:newtile:fg:bg
// tile:T:type:tileset:newtile:fg:bg
// tile:tileset:newtile - disabled for now
static bool handle_override_command(vector<string> &tokens, std::map<string, int> &tilesetnames)
{
    if (tokens.size() < 3)
        return false;

    int tile = atoi(tokens[0].c_str());
    if (tile < 0 || tile > 255)
    {
        *out2 << COLOR_YELLOW << "TWBT: invalid tile number " << tokens[0] << std::endl;
        *out2 << COLOR_RESET;
        return false;        
    }

    struct override o;
    char kind = tokens[1][0];
    int id;
    int basetoken;

    // Building or Item
    if (tokens.size() >= 7 && (kind == 'B' || kind == 'I'))
    {
        if (kind == 'B')
        {
            if (!parse_enum_or_int<buildings_other_id::buildings_other_id>(tokens[2], id, buildings_other_id::IN_PLAY))
                return false;
            if (!parse_enum_or_int<building_type::building_type>(tokens[3], o.type))
                return false;

            if (id == buildings_other_id::WORKSHOP_CUSTOM || id == buildings_other_id::FURNACE_CUSTOM)
                o.subtypename = tokens[4];

            if (tokens[4].length() > 0)
                o.subtype = atoi(tokens[4].c_str());
            else
                o.subtype = -1;
        }
        else if (kind == 'I')
        {
            if (!parse_enum_or_int<items_other_id::items_other_id>(tokens[2], id, items_other_id::IN_PLAY))
                return false;
            if (!parse_enum_or_int<item_type::item_type>(tokens[3], o.type))
                return false;

            if (tokens[4].length() > 0)
            {
                ItemTypeInfo item_type_info;
                if (item_type_info.find(tokens[3] + ":" + tokens[4]))
                    o.subtype = item_type_info.subtype;
                else
                    o.subtype = atoi(tokens[4].c_str());
            }
            else
                o.subtype = -1;

        }
        else
            return false;

        basetoken = 5;        
    }

    // Tiletype
    else if (tokens.size() >= 5 && kind == 'T')
    {
        std::string &typestr = tokens[2];
        int ln = typestr.length();
        if (!ln)
            return false;

        if (typestr[0] == '"' && typestr[ln-1] == '"')
        {
            std::string tn = typestr.substr(1, ln-2);

            o.type = -1;
            FOR_ENUM_ITEMS(tiletype, tt)
            {
                const char *ttn = tileName(tt);
                if (ttn && tn == ttn)
                {
                    o.type = tt;
                    break;
                }
            }
        }
        else if (!parse_enum_or_int<tiletype::tiletype>(typestr, o.type))
            return false;

        if (o.type == -1)
            return false;

        basetoken = 3;
    }
    /*else if (tokens.size() == 3)
    {
        tilesets[0].small_texpos[tile] = tilesets[tsidx].small_texpos[newtile];
        return true;
    }*/
    else
        return false;          

    // New tile number
    if (tokens.size() > basetoken+1 && tokens[basetoken+1].length())
    {
        int newtile = atoi(tokens[basetoken+1].c_str());
        if (newtile < 0 || newtile > 255)
        {
            *out2 << COLOR_YELLOW << "TWBT: invalid new tile number " << tokens[basetoken+1] << std::endl;
            *out2 << COLOR_RESET;
            return false;        
        }

        string &tsname = tokens[basetoken+0];
        if (!tilesetnames.count(tsname))
        {
            *out2 << COLOR_YELLOW << "TWBT: no tileset with id " << tsname << std::endl;
            *out2 << COLOR_RESET;

            return false;
        }
        int tsidx = tilesetnames[tsname];

        o.small_texpos = tilesets[tsidx].small_texpos[newtile];
        o.bg_texpos = tilesets[tsidx].bg_texpos[newtile];
        o.top_texpos = tilesets[tsidx].top_texpos[newtile];
    }
    else
        o.small_texpos = 0;

    // New foreground colour
    if (tokens.size() > basetoken+2 && tokens[basetoken+2].length())
    {
        int newfg = atoi(tokens[basetoken+2].c_str());
        if (newfg < 1 || newfg > 16)
        {
            *out2 << COLOR_YELLOW << "TWBT: invalid new fg " << tokens[basetoken+2] << std::endl;
            *out2 << COLOR_RESET;
            return false;        
        }

        o.fg = newfg - 1;
    }
    else
        o.fg = -1;

    // New background colour
    if (tokens.size() > basetoken+3 && tokens[basetoken+3].length())
    {
        int newbg = atoi(tokens[basetoken+3].c_str());
        if (newbg < 1 || newbg > 16)
        {
            *out2 << COLOR_YELLOW << "TWBT: invalid new bg " << tokens[basetoken+3] << std::endl;
            *out2 << COLOR_RESET;
            return false;        
        }

        o.bg = newbg - 1;
    }
    else
        o.bg = -1;

    if (!(o.small_texpos != -1 || o.fg != -1 || o.bg != -1))
        return false;

    if (!overrides[tile])
        overrides[tile] = new tile_overrides;

    if (kind == 'T')
    {
        overrides[tile]->tiletype_overrides.push_back(o);
        return true;
    }

    auto &groups = (kind == 'I') ? overrides[tile]->item_overrides : overrides[tile]->building_overrides;

    for (auto it = groups.begin(); it != groups.end(); it++)
    {
        override_group &grp = *it;

        if (grp.other_id == id)
        {
            grp.overrides.push_back(o);
            return true;
        }
    }

    override_group grp;
    grp.other_id = id;
    grp.overrides.push_back(o);
    groups.push_back(grp);

    return true;
}

static bool load_overrides()
{
    bool any_overrides = false;

    std::ifstream fseed("data/init/overrides.txt");
    if(!fseed.is_open())
        return false;

    std::map<string, int> tilesetnames;
    tilesetnames["map"] = 0;
    tilesetnames["0"] = 0;
    tilesetnames["text"] = 1;
    tilesetnames["1"] = 1;

    string str;
    while(std::getline(fseed,str))
    {
        size_t b = str.find("[");
        size_t e = str.rfind("]");

        if (b == string::npos || e == string::npos || str.find_first_not_of(" ") < b)
            continue;

        str = str.substr(b+1, e-1);
        vector<string> tokens = split(str.c_str(), ':');

        if (tokens[0] == "TILESET")
        {
            if (tokens.size() == 3 || tokens.size() == 4)
            {
                if (tokens.size() == 4 && tilesetnames.count(tokens[3]))
                {
                    *out2 << COLOR_YELLOW << "TWBT: ignoring duplicate tileset with id " << tokens[3] << std::endl;
                    *out2 << COLOR_RESET;
                    continue;
                }

                struct tileset ts;
                string small_font_path = "data/art/" + tokens[1];
                //string large_font_path = "data/art/" + tokens[2];
                //TODO: show warning that large font is no longer loaded

                struct stat buf;
                if (stat(small_font_path.c_str(), &buf) != 0)
                {
                    *out2 << COLOR_YELLOW << "TWBT: " << small_font_path << " not found" << std::endl;
                    *out2 << COLOR_RESET;
                    continue;
                }

                long dx, dy;        
                load_tileset(small_font_path, ts.small_texpos, 16, 16, &dx, &dy);
                load_tileset_layers(ts, small_font_path);

                tilesets.push_back(ts);

                int idx = tilesets.size() - 1;
                string n = (tokens.size() == 4) ?
                    tokens[3] :
                    static_cast<std::ostringstream*>(&(std::ostringstream() << idx))->str();
                tilesetnames[n] = idx;
            }

            continue;
        }
        
        if (tokens[0] == "OVERRIDE")
        {
            if (tokens.size() > 1)
            {
                tokens.erase(tokens.begin());
                if (handle_override_command(tokens, tilesetnames))
                    any_overrides = true;
                else
                {
                    *out2 << COLOR_YELLOW << "TWBT: invalid override specification " << str << std::endl;
                    *out2 << COLOR_RESET;
                }
            }

            continue;
        }

        if (tokens[0] == "CURSOR")
        {
            if (!tilesetnames.count(tokens[1]))
            {
                *out2 << COLOR_YELLOW << "TWBT: no tileset with id " << tokens[1] << std::endl;
                *out2 << COLOR_RESET;
                continue;
            }

            int tsidx = tilesetnames[tokens[1]];
            int newtile = atoi(tokens[2].c_str());

            cursor_small_texpos = tilesets[tsidx].small_texpos[newtile];

            any_overrides = true;
            continue;
        }

        fseed.close();
    }

    return any_overrides;
}

static int color_name_to_index(std::string &name)
{
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);

    static const char *names[] = {
        "black", "blue", "green", "cyan", "red", "magenta", "brown", "lgray",
        "dgray", "lblue", "lgreen", "lcyan", "lred", "lmagenta", "yellow", "white"
    };

    for (int i = 0; i < 16; i++)
        if (name == names[i])
            return i;

    return -1;
}

static void load_colormap()
{
    std::ifstream fseed("data/init/colors.txt");
    if(fseed.is_open())
    {
        string str;

        while(std::getline(fseed,str))
        {
            size_t b = str.find("[");
            size_t e = str.rfind("]");

            if (b == string::npos || e == string::npos || str.find_first_not_of(" ") < b)
                continue;

            str = str.substr(b+1, e-1);
            vector<string> tokens = split(str.c_str(), ':');
            if (tokens.size() != 2)
                continue;

            std::string &token0 = tokens[0];
            if (token0.length() < 3)
                continue;

            std::string name (token0.substr(0, token0.length()-2));
            int cidx = color_name_to_index(name);
            if (cidx == -1)
                continue;

            float val;
            if (!parse_float(tokens[1], val))
                continue;

            val /= 255.0f;

            const std::string &comp = token0.substr(token0.length()-2);
            if (comp == "_R")
                enabler->ccolor[cidx][0] = val;
            else if (comp == "_G")
                enabler->ccolor[cidx][1] = val;
            else if (comp == "_B")
                enabler->ccolor[cidx][2] = val;
        }

        fseed.close();
    }
}

int get_mode()
{
    int mode = 0;

    std::ifstream fseed("data/init/init.txt");
    if(fseed.is_open())
    {
        string str;

        while(std::getline(fseed,str))
        {
            size_t b = str.find("[");
            size_t e = str.rfind("]");

            if (b == string::npos || e == string::npos || str.find_first_not_of(" ") < b)
                continue;

            str = str.substr(b+1, e-1);
            vector<string> tokens = split(str.c_str(), ':');

            if (tokens[0] == "PRINT_MODE")
            {
                if (tokens.size() == 2)
                {
                    if (tokens[1] == "TWBT")
                        mode = 1;
                    else if (tokens[1] == "TWBT-LEGACY" || tokens[1] == "TWBT_LEGACY")
                        mode = -1;
                }

                break;
            }
        }

        fseed.close();    
    }

    return mode;
}

void update_custom_building_overrides()
{
    for (int j = 0; j < 256; j++)
    {
        if (!overrides[j])
            continue;

        for (auto it = overrides[j]->building_overrides.begin(); it != overrides[j]->building_overrides.end(); it++)
        {
            override_group &og = *it;

            if (og.other_id == buildings_other_id::WORKSHOP_CUSTOM || og.other_id == buildings_other_id::FURNACE_CUSTOM)
            {
                for (auto it3 = og.overrides.begin(); it3 != og.overrides.end(); it3++)
                {
                    override &o = *it3;

                    if (o.subtypename.length())
                    {
                        o.subtype = -2;
                        auto ilist = world->raws.buildings.all; //TODO: should use different arrays for workshops and furnaces?

                        for (auto it = ilist.begin(); it != ilist.end(); it++)
                        {
                            df::building_def *bdef = *it;

                            if (bdef->code == o.subtypename)
                            {
                                o.subtype = bdef->id;
                                break;
                            }
                        }

                        if (o.subtype == -2)
                        {
                            *out2 << COLOR_YELLOW << "TWBT: no custom building for name " << o.subtypename << std::endl;
                            *out2 << COLOR_RESET;
                            continue;
                        }                    
                    }
                }
            }
        }
    }
}