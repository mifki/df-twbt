#define BLD_DRAW_IMAGE(id) \
    for (int i = 0; i < bldw; i++) \
        for (int j = 0; j < bldh; j++) \
            gscreen_over[(dbuf->x1+i)*256 + dbuf->y1 + j] = id##_tiles[((TICK % id##_frames) + j*id##_frames) * bldw + i];

#define BLD_DRAW_IMAGE_OR_DEFAULT(id) { \
    if (IS_LOADED(id)) \
        BLD_DRAW_IMAGE(id) \
    else \
        INTERPOSE_NEXT(drawBuilding)(dbuf, smth); \
}

#define BLD_DRAW_DEFAULT INTERPOSE_NEXT(drawBuilding)(dbuf, smth);

#define BLD_DRAW_IMAGE_AND_RETURN(id) { \
    if (IS_LOADED(id)) { \
        BLD_DRAW_IMAGE(id) \
        return; \
    } \
}

#define BLD_HOOK(cls) INTERPOSE_HOOK(cls##_twbt, drawBuilding).apply(true); 


#define BLD_OVR_BEGIN(cls,code) \
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

#define BLD_OVR_END(cls) \
    } \
}; \
IMPLEMENT_VMETHOD_INTERPOSE(cls##_twbt, drawBuilding);

#define BLD_OVR_SIMPLE(cls, fn) \
BLD_OVR_BEGIN(cls, \
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
    BLD_DRAW_IMAGE(normal) \
})

#define BLD_OVR_SIMPLEST(name) \
    BLD_OVR_SIMPLE(building_##name##st, #name)




#include <df/building_archerytargetst.h>
BLD_OVR_SIMPLEST(archerytarget)

#include <df/building_armorstandst.h>
BLD_OVR_SIMPLEST(armorstand)

#include <df/building_bars_floorst.h>
BLD_OVR_SIMPLEST(bars_floor)

#include <df/building_bars_verticalst.h>
BLD_OVR_SIMPLEST(bars_vertical)

#include <df/building_bedst.h>
BLD_OVR_BEGIN(building_bedst,
{
    DEFINE_SIZE(1,1)
    DEFINE_VARS(normal)
    DEFINE_VARS(dorm)

    LOAD_BEGIN
    LOAD_IMAGE(normal, "bed")
    LOAD_IMAGE(dorm, "bed-dorm")
    LOAD_END

    FILL_PLACEHOLDER_2

    if (this->bed_flags.bits.dormitory)
        BLD_DRAW_IMAGE_AND_RETURN(dorm)

    BLD_DRAW_IMAGE_OR_DEFAULT(normal)
})

#include <df/building_boxst.h>
BLD_OVR_SIMPLEST(box)

#include <df/building_cabinetst.h>
BLD_OVR_SIMPLEST(cabinet)

#include <df/building_cagest.h>
BLD_OVR_SIMPLEST(cage)


#include <df/building_chainst.h>
BLD_OVR_SIMPLEST(chain)


#include <df/building_chairst.h>
BLD_OVR_SIMPLEST(chair)


#include <df/building_coffinst.h>
BLD_OVR_SIMPLEST(coffin)


#include <df/building_doorst.h>
BLD_OVR_BEGIN(building_doorst,
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
        BLD_DRAW_IMAGE_AND_RETURN(locked)

    BLD_DRAW_IMAGE_OR_DEFAULT(normal)
})

#include <df/building_floodgatest.h>
BLD_OVR_SIMPLEST(floodgate)


#include <df/building_grate_floorst.h>
BLD_OVR_SIMPLEST(grate_floor)

#include <df/building_grate_wallst.h>
BLD_OVR_SIMPLEST(grate_wall)

#include <df/building_hatchst.h>
BLD_OVR_SIMPLEST(hatch)

#include <df/building_hivest.h>
BLD_OVR_SIMPLEST(hive)

#include <df/building_nest_boxst.h>
BLD_OVR_SIMPLE(building_nest_boxst, "nestbox")


#include <df/building_slabst.h>
BLD_OVR_SIMPLEST(slab)


#include <df/building_statuest.h>
BLD_OVR_SIMPLEST(statue)


#include <df/building_supportst.h>
BLD_OVR_SIMPLEST(support)

#include <df/building_tablest.h>
BLD_OVR_SIMPLEST(table)

#include <df/building_traction_benchst.h>
BLD_OVR_SIMPLEST(traction_bench)

//TODO: levers are traps
//#include <df/building_trapst.h>
//OVR_SIMPLEST(trap)

#include <df/building_weaponrackst.h>
BLD_OVR_SIMPLEST(weaponrack)


#include <df/building_wellst.h>
BLD_OVR_SIMPLEST(well)


#include <df/building_window_gemst.h>
BLD_OVR_SIMPLEST(window_gem)

#include <df/building_window_glassst.h>
BLD_OVR_SIMPLEST(window_glass)


#include <df/building_workshopst.h>
BLD_OVR_BEGIN(building_workshopst,
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
            BLD_DRAW_IMAGE_AND_RETURN(carpenters)
            break;

        case df::workshop_type::Masons:
            BLD_DRAW_IMAGE_AND_RETURN(masons)
            break;

        case df::workshop_type::Jewelers:
            BLD_DRAW_IMAGE_AND_RETURN(jewelers)
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

    BLD_DRAW_DEFAULT
})



void apply_building_hooks()
{
    BLD_HOOK(building_archerytargetst)
    BLD_HOOK(building_armorstandst)
    BLD_HOOK(building_bars_floorst)
    BLD_HOOK(building_bars_verticalst)
    BLD_HOOK(building_bedst)
    BLD_HOOK(building_boxst)
    BLD_HOOK(building_cabinetst)
    BLD_HOOK(building_cagest)
    BLD_HOOK(building_chainst)
    BLD_HOOK(building_chairst)
    BLD_HOOK(building_coffinst)
    BLD_HOOK(building_doorst)
    BLD_HOOK(building_floodgatest)
    BLD_HOOK(building_grate_floorst)
    BLD_HOOK(building_grate_wallst)
    BLD_HOOK(building_hatchst)
    BLD_HOOK(building_hivest)
    BLD_HOOK(building_nest_boxst)
    BLD_HOOK(building_tablest)
    BLD_HOOK(building_slabst)
    BLD_HOOK(building_statuest)
    BLD_HOOK(building_supportst)
    BLD_HOOK(building_traction_benchst)
    BLD_HOOK(building_weaponrackst)
    BLD_HOOK(building_wellst)
    BLD_HOOK(building_window_gemst)
    BLD_HOOK(building_window_glassst)
    BLD_HOOK(building_workshopst)
}