#define DEFINE_VARS(id) \
static bool id##_loaded; \
static int id##_frames; \
static long *id##_tiles;

#define VARS2(cls,id) \
bool cls##_twbt::id##_loaded; \
int cls##_twbt::id##_frames; \
long *cls##_twbt::id##_tiles;

#define DEFINE_VARS_DYN() \
static bool *loaded; \
static int *frames; \
static long **tiles; \
static bool *tried;

#define VARS_DYN2(cls) \
bool *cls##_twbt::loaded; \
int *cls##_twbt::frames; \
long **cls##_twbt::tiles; \
bool *cls##_twbt::tried;


#define LOAD_BEGIN \
static bool loaded = false; \
if (!loaded) \
{ \
    bool ok = false;

#define LOAD_END \
    if (!ok) \
    { \
        unhook(); \
        return; \
    } \
    loaded = true; \
}

#define LOAD_IMAGE(id,fn) \
    id##_loaded = load_tiles("data/art/tiles/" fn ".png", &id##_tiles, &id##_frames, bldw, bldh); \
    ok |= id##_loaded;

#define LOAD_IMAGE_DYN(id,fn) \
    id##_loaded = load_tiles("data/art/tiles/" fn ".png", &id##_tiles, &id##_frames, bldw, bldh); \
    id_tried = true;

#define IS_LOADED(id) id##_loaded

#define DEFINE_TICK \
unsigned int tick = df::global::enabler->gputicks.value / 10;

#define TICK tick

#define DEFINE_SIZE(w,h) \
const int bldw = w, bldh = h;

#define FILL_PLACEHOLDER(fg,bg,br) \
    for (int i = 0; i < bldw; i++) \
        for (int j = 0; j < bldh; j++) \
        { \
            dbuf->fore[i][j] = fg; \
            dbuf->bright[i][j] = br; \
            dbuf->back[i][j] = bg; \
            dbuf->tile[i][j] = 0xfe + (TICK%2); \
        }

#define FILL_PLACEHOLDER_1 { \
    MaterialInfo material; \
    material.decode(this->mat_type, this->mat_index); \
    int fg = material.material->basic_color[0]; \
    int br = material.material->basic_color[1]; \
    FILL_PLACEHOLDER(fg,0,br) \
}

#define FILL_PLACEHOLDER_2 { \
    MaterialInfo material; \
    material.decode(this->mat_type, this->mat_index); \
    int fg = material.material->build_color[0]; \
    int bg = material.material->build_color[1]; \
    int br = material.material->build_color[2]; \
    FILL_PLACEHOLDER(fg,bg,br) \
}


bool load_tiles(const char *fn, long **tiles, int *frames, int w, int h)
{
    long dx, dy;
    DFSDL_Surface *surf = IMG_Load(fn);
    if (!surf)
        return false;

    if (surf->w % surf->h != 0)
    {
        *out2 << "invalid size " << fn << std::endl;
        return false;
    }

    *frames = surf->w / surf->h;
    *tiles = (long*) malloc(sizeof(long)*(*frames));

    *out2 << surf->w << " " << surf->h << std::endl;
    load_tileset(fn, *tiles, *frames*w, h, &dx, &dy);

    SDL_FreeSurface(surf);    
    
    ((renderer_cool*)enabler->renderer)->needs_full_update = true; \
    gps->force_full_display_count = 1;
    return true;
}

#define OVR_BEGIN(cls,code) \
struct cls##_twbt : public df::cls \
{ \
    typedef df::cls interpose_base; \
\
    void unhook() { \
        INTERPOSE_HOOK(cls##_twbt, drawBuilding).apply(false); \
    } \
    DEFINE_VMETHOD_INTERPOSE(void, drawBuilding, (df::building_drawbuffer* dbuf, int16_t smth)) \
    { \
        DEFINE_TICK \
        code \
    } \
}; \
IMPLEMENT_VMETHOD_INTERPOSE(cls##_twbt, drawBuilding);

#define OVR_END(cls) \
    } \
}; \
IMPLEMENT_VMETHOD_INTERPOSE(cls##_twbt, drawBuilding);

#define OVR_SIMPLE(cls, fn) \
OVR_BEGIN(cls, \
{ \
    DEFINE_SIZE(1,1) \
    DEFINE_VARS(normal) \
\
    LOAD_BEGIN \
    LOAD_IMAGE(normal, fn) \
    LOAD_END \
\
    FILL_PLACEHOLDER_2 \
\
    DRAW_IMAGE(normal) \
})

#define OVR_SIMPLEST(name) \
    OVR_SIMPLE(building_##name##st, #name)