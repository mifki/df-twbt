#define DEFINE_VARS(id) \
static bool id##_loaded; \
static int id##_frames; \
static long *id##_tiles;

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

#define DRAW_IMAGE(id) \
    for (int i = 0; i < bldw; i++) \
        for (int j = 0; j < bldh; j++) \
            gscreen_over[(dbuf->x1+i)*256 + dbuf->y1 + j] = id##_tiles[((TICK % id##_frames) + j*id##_frames) * bldw + i];

#define DRAW_IMAGE_OR_DEFAULT(id) { \
    if (IS_LOADED(id)) \
        DRAW_IMAGE(id) \
    else \
        INTERPOSE_NEXT(drawBuilding)(dbuf, smth); \
}

#define DRAW_DEFAULT INTERPOSE_NEXT(drawBuilding)(dbuf, smth);

#define DRAW_IMAGE_AND_RETURN(id) { \
    if (IS_LOADED(id)) { \
        DRAW_IMAGE(id) \
        return; \
    } \
}

#define HOOK(cls) INTERPOSE_HOOK(cls##_twbt, drawBuilding).apply(true); 

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




#include <df/building_archerytargetst.h>
OVR_SIMPLEST(archerytarget)

#include <df/building_armorstandst.h>
OVR_SIMPLEST(armorstand)

#include <df/building_bars_floorst.h>
OVR_SIMPLEST(bars_floor)

#include <df/building_bars_verticalst.h>
OVR_SIMPLEST(bars_vertical)

#include <df/building_bedst.h>
OVR_BEGIN(building_bedst,
{
    DEFINE_SIZE(1,1)
    DEFINE_VARS(normal)
    DEFINE_VARS(dorm)

    LOAD_BEGIN
    LOAD_IMAGE(normal, "bed")
    LOAD_IMAGE(dorm, "bed-dorm")
    LOAD_END

    FILL_PLACEHOLDER_2

    if (this->anon_1.bits.dormitory)
        DRAW_IMAGE_AND_RETURN(dorm)

    DRAW_IMAGE_OR_DEFAULT(normal)
})

#include <df/building_boxst.h>
OVR_SIMPLEST(box)

#include <df/building_cabinetst.h>
OVR_SIMPLEST(cabinet)

#include <df/building_cagest.h>
OVR_SIMPLEST(cage)


#include <df/building_chainst.h>
OVR_SIMPLEST(chain)


#include <df/building_chairst.h>
OVR_SIMPLEST(chair)


#include <df/building_coffinst.h>
OVR_SIMPLEST(coffin)


#include <df/building_doorst.h>
OVR_BEGIN(building_doorst,
{
    DEFINE_SIZE(1,1)
    DEFINE_VARS(normal)
    DEFINE_VARS(locked)

    LOAD_BEGIN
    LOAD_IMAGE(normal, "door")
    LOAD_IMAGE(locked, "door-locked")
    LOAD_END

    FILL_PLACEHOLDER_2

    if (this->door_flags.bits.forbidden)
        DRAW_IMAGE_AND_RETURN(locked)

    DRAW_IMAGE_OR_DEFAULT(normal)
})

#include <df/building_floodgatest.h>
OVR_SIMPLEST(floodgate)


#include <df/building_grate_floorst.h>
OVR_SIMPLEST(grate_floor)

#include <df/building_grate_wallst.h>
OVR_SIMPLEST(grate_wall)

#include <df/building_hatchst.h>
OVR_SIMPLEST(hatch)

#include <df/building_hivest.h>
OVR_SIMPLEST(hive)

#include <df/building_nest_boxst.h>
OVR_SIMPLEST(nestbox)


#include <df/building_slabst.h>
OVR_SIMPLEST(slab)


#include <df/building_statuest.h>
OVR_SIMPLEST(statue)


#include <df/building_supportst.h>
OVR_SIMPLEST(support)

#include <df/building_tablest.h>
OVR_SIMPLEST(table)

#include <df/building_traction_benchst.h>
OVR_SIMPLEST(traction_bench)

//TODO: levers are traps
//#include <df/building_trapst.h>
//OVR_SIMPLEST(trap)

#include <df/building_weaponrackst.h>
OVR_SIMPLEST(weaponrack)


#include <df/building_wellst.h>
OVR_SIMPLEST(well)


#include <df/building_window_gemst.h>
OVR_SIMPLEST(window_gem)

#include <df/building_window_glassst.h>
OVR_SIMPLEST(window_glass)


#include <df/building_workshopst.h>
OVR_BEGIN(building_workshopst,
{
    DEFINE_SIZE(3,3)
    DEFINE_VARS(carpenters)
    DEFINE_VARS(masons)
    DEFINE_VARS(jewelers)

    LOAD_BEGIN
    LOAD_IMAGE(carpenters, "workshops/carpenters")
    LOAD_IMAGE(masons, "workshops/masons")
    LOAD_IMAGE(jewelers, "workshops/jewelers")
    LOAD_END

    FILL_PLACEHOLDER_1

    switch (this->type)
    {
        case df::workshop_type::Carpenters:
            DRAW_IMAGE_AND_RETURN(carpenters)
            break;

        case df::workshop_type::Masons:
            DRAW_IMAGE_AND_RETURN(masons)
            break;

        case df::workshop_type::Jewelers:
            DRAW_IMAGE_AND_RETURN(jewelers)
            break;

/*
        Farmers,
        Craftsdwarfs,
        MetalsmithsForge,
        MagmaForge,
        Bowyers,
        Mechanics,
        Siege,
        Butchers,
        Leatherworks,
        Tanners,
        Clothiers,
        Fishery,
        Still,
        Loom,
        Quern,
        Kennels,
        Kitchen,
        Ashery,
        Dyers,
        Millstone,
        Custom,
        Tool
*/        
    }

    DRAW_DEFAULT
})



void apply_building_hooks()
{
    HOOK(building_archerytargetst)
    HOOK(building_armorstandst)
    HOOK(building_bars_floorst)
    HOOK(building_bars_verticalst)
    HOOK(building_bedst)
    HOOK(building_boxst)
    HOOK(building_cabinetst)
    HOOK(building_cagest)
    HOOK(building_chainst)
    HOOK(building_chairst)
    HOOK(building_coffinst)
    HOOK(building_doorst)
    HOOK(building_floodgatest)
    HOOK(building_grate_floorst)
    HOOK(building_grate_wallst)
    HOOK(building_hatchst)
    HOOK(building_hivest)
    HOOK(building_nest_boxst)
    HOOK(building_tablest)
    HOOK(building_slabst)
    HOOK(building_statuest)
    HOOK(building_supportst)
    HOOK(building_traction_benchst)
    HOOK(building_weaponrackst)
    HOOK(building_wellst)
    HOOK(building_window_gemst)
    HOOK(building_window_glassst)
    HOOK(building_workshopst)
}