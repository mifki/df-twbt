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

#define BLD_DRAW_IMAGE_FILL(id) \
    for (int x = dbuf->x1; x <= dbuf->x2; x++) \
        for (int y = dbuf->y1; y <= dbuf->y2; y++) \
            gscreen_over[x*256 + y] = id##_tiles[((TICK % id##_frames) + 0) * 1 + 0];

#define BLD_DRAW_IMAGE_FILL_AND_RETURN(id) { \
    if (IS_LOADED(id)) { \
        BLD_DRAW_IMAGE_FILL(id) \
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

#define BLD_OVR_SIMPLE(cls,fn,w,h) \
BLD_OVR_BEGIN(cls, \
{ \
    DEFINE_SIZE(w,h) \
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
    BLD_OVR_SIMPLE(building_##name##st, #name, 1, 1)




#include <df/building_archerytargetst.h>
BLD_OVR_SIMPLEST(archerytarget)

#include <df/building_armorstandst.h>
BLD_OVR_SIMPLEST(armorstand)

#include <df/building_axle_horizontalst.h>
BLD_OVR_BEGIN(building_axle_horizontalst,
{
    DEFINE_SIZE(1,1)
    DEFINE_VARS(any)
    DEFINE_VARS(any_active)
    DEFINE_VARS(ew)
    DEFINE_VARS(ew_active)
    DEFINE_VARS(ns)
    DEFINE_VARS(ns_active)

    LOAD_BEGIN
    LOAD_IMAGE(any, "axle_horizontal")
    LOAD_IMAGE(any_active, "axle_horizontal-active")
    LOAD_END

    {
        int bldw = dbuf->x2 - dbuf->x1 + 1;
        int bldh = dbuf->y2 - dbuf->y1 + 1;
        FILL_PLACEHOLDER_2
    }

    //TODO: shouldn't animate if game paused
    df::machine *machine = df::machine::find(this->machine.machine_id);
    if (machine && machine->cur_power >= machine->min_power && machine->cur_power > 0)
        BLD_DRAW_IMAGE_FILL_AND_RETURN(any_active)

    BLD_DRAW_IMAGE_FILL_AND_RETURN(any)

    INTERPOSE_NEXT(drawBuilding)(dbuf, smth);
})

#include <df/building_axle_verticalst.h>
BLD_OVR_BEGIN(building_axle_verticalst,
{
    DEFINE_SIZE(1,1)
    DEFINE_VARS(normal)
    DEFINE_VARS(active)

    LOAD_BEGIN
    LOAD_IMAGE(normal, "axle_vertical")
    LOAD_IMAGE(active, "axle_vertical-active")
    LOAD_END

    FILL_PLACEHOLDER_2

    //TODO: shouldn't animate if game paused
    df::machine *machine = df::machine::find(this->machine.machine_id);
    if (machine && machine->cur_power >= machine->min_power && machine->cur_power > 0)
        BLD_DRAW_IMAGE_AND_RETURN(active)

    BLD_DRAW_IMAGE_OR_DEFAULT(normal)
})

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

#include <df/building_bridgest.h>
BLD_OVR_BEGIN(building_bridgest,
{
    DEFINE_SIZE(3,3)
    DEFINE_VARS(normal)

    LOAD_BEGIN
    LOAD_IMAGE(normal, "bridge")
    LOAD_END

    {
        int bldw = dbuf->x2 - dbuf->x1 + 1;
        int bldh = dbuf->y2 - dbuf->y1 + 1;
        FILL_PLACEHOLDER_2
    }

    if (IS_LOADED(normal))
    {
        //TODO: handle 1x1, 1xQ and Qx1 cases
        // Corners
        gscreen_over[(dbuf->x1+0)*256 + dbuf->y1 + 0] = normal_tiles[((TICK % normal_frames) + 0*normal_frames) * bldw + 0];
        gscreen_over[(dbuf->x2+0)*256 + dbuf->y1 + 0] = normal_tiles[((TICK % normal_frames) + 0*normal_frames) * bldw + 2];
        gscreen_over[(dbuf->x1+0)*256 + dbuf->y2 + 0] = normal_tiles[((TICK % normal_frames) + 2*normal_frames) * bldw + 0];
        gscreen_over[(dbuf->x2+0)*256 + dbuf->y2 + 0] = normal_tiles[((TICK % normal_frames) + 2*normal_frames) * bldw + 2];

        // Top and bottom
        for (int x = dbuf->x1+1; x < dbuf->x2; x++)
            gscreen_over[x*256 + dbuf->y1] = normal_tiles[((TICK % normal_frames) + 0*normal_frames) * bldw + 1];
        for (int x = dbuf->x1+1; x < dbuf->x2; x++)
            gscreen_over[x*256 + dbuf->y2] = normal_tiles[((TICK % normal_frames) + 2*normal_frames) * bldw + 1];

        // Left and right
        for (int y = dbuf->y1+1; y < dbuf->y2; y++)
            gscreen_over[dbuf->x1*256 + y] = normal_tiles[((TICK % normal_frames) + 1*normal_frames) * bldw + 0];
        for (int y = dbuf->y1+1; y < dbuf->y2; y++)
            gscreen_over[dbuf->x2*256 + y] = normal_tiles[((TICK % normal_frames) + 1*normal_frames) * bldw + 2];

        // Middle
        for (int x = dbuf->x1+1; x < dbuf->x2; x++)
            for (int y = dbuf->y1+1; y < dbuf->y2; y++)
                gscreen_over[x*256 + y] = normal_tiles[((TICK % normal_frames) + 1*normal_frames) * bldw + 1];

        return;
    }

    INTERPOSE_NEXT(drawBuilding)(dbuf, smth);
})

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


#include <df/building_furnacest.h>

#define BLD_FURNACE_LOADIMG_SIMPLE(name) \
    name##_loaded = load_tiles("data/art/tiles/furnaces/" #name ".png", &name##_tiles, &name##_frames, bldw, bldh); \
    ok |= name##_loaded;

#define BLD_FURNACE_SIMPLE(type,name) \
    case df::furnace_type::type: \
        BLD_DRAW_IMAGE_AND_RETURN(name) \
        break;

BLD_OVR_BEGIN(building_furnacest,
{
    DEFINE_SIZE(3,3)
    DEFINE_VARS(wood)
    DEFINE_VARS(smelter)
    DEFINE_VARS(glass)
    DEFINE_VARS(kiln)
    DEFINE_VARS(magma_smelter)
    DEFINE_VARS(magma_glass)
    DEFINE_VARS(magma_kiln)

    LOAD_BEGIN
    BLD_FURNACE_LOADIMG_SIMPLE(wood)
    BLD_FURNACE_LOADIMG_SIMPLE(smelter)
    BLD_FURNACE_LOADIMG_SIMPLE(glass)
    BLD_FURNACE_LOADIMG_SIMPLE(kiln)
    BLD_FURNACE_LOADIMG_SIMPLE(magma_smelter)
    BLD_FURNACE_LOADIMG_SIMPLE(magma_glass)
    BLD_FURNACE_LOADIMG_SIMPLE(magma_kiln)
    LOAD_END

    FILL_PLACEHOLDER_1

    //TODO: can be done by deriving file name from type, but what if we will want custom states for some of them?
    /*std::string name = df::enum_traits<df::workshop_type>::key_table[this->type];
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);*/

    switch (this->type)
    {
        BLD_FURNACE_SIMPLE(WoodFurnace, wood)
        BLD_FURNACE_SIMPLE(Smelter, smelter)
        BLD_FURNACE_SIMPLE(GlassFurnace, glass)
        BLD_FURNACE_SIMPLE(Kiln, kiln)
        BLD_FURNACE_SIMPLE(MagmaSmelter, magma_smelter)
        BLD_FURNACE_SIMPLE(MagmaGlassFurnace, magma_glass)
        BLD_FURNACE_SIMPLE(MagmaKiln, magma_kiln)

        //Custom
    }

    BLD_DRAW_DEFAULT
})


#include <df/building_gear_assemblyst.h>
BLD_OVR_BEGIN(building_gear_assemblyst,
{
    DEFINE_SIZE(1,1)
    DEFINE_VARS(normal)
    DEFINE_VARS(active)

    LOAD_BEGIN
    LOAD_IMAGE(normal, "gear_assembly")
    LOAD_IMAGE(active, "gear_assembly-active")
    LOAD_END

    FILL_PLACEHOLDER_2

    //TODO: shouldn't animate if game paused
    df::machine *machine = df::machine::find(this->machine.machine_id);
    if (machine && machine->cur_power >= machine->min_power && machine->cur_power > 0)
        BLD_DRAW_IMAGE_AND_RETURN(active)

    BLD_DRAW_IMAGE_OR_DEFAULT(normal)
})

#include <df/building_grate_floorst.h>
BLD_OVR_SIMPLEST(grate_floor)

#include <df/building_grate_wallst.h>
BLD_OVR_SIMPLEST(grate_wall)

#include <df/building_hatchst.h>
BLD_OVR_SIMPLEST(hatch)

#include <df/building_hivest.h>
BLD_OVR_SIMPLEST(hive)

#include <df/building_nest_boxst.h>
BLD_OVR_SIMPLE(building_nest_boxst, "nestbox", 1, 1)

#include <df/building_rollersst.h>
BLD_OVR_BEGIN(building_rollersst,
{
    DEFINE_SIZE(1,1)
    DEFINE_VARS(any)
    DEFINE_VARS(any_active)
    DEFINE_VARS(we)
    DEFINE_VARS(we_active)
    DEFINE_VARS(ew)
    DEFINE_VARS(ew_active)
    DEFINE_VARS(ns)
    DEFINE_VARS(ns_active)
    DEFINE_VARS(sn)
    DEFINE_VARS(sn_active)

    LOAD_BEGIN
    LOAD_IMAGE(any, "rollers")
    LOAD_IMAGE(any_active, "rollers-active")
    LOAD_END

    {
        int bldw = dbuf->x2 - dbuf->x1 + 1;
        int bldh = dbuf->y2 - dbuf->y1 + 1;
        FILL_PLACEHOLDER_2
    }

    //TODO: shouldn't animate if game paused
    df::machine *machine = df::machine::find(this->machine.machine_id);
    if (machine && machine->cur_power >= machine->min_power && machine->cur_power > 0)
        BLD_DRAW_IMAGE_FILL_AND_RETURN(any_active)

    BLD_DRAW_IMAGE_FILL_AND_RETURN(any)

    INTERPOSE_NEXT(drawBuilding)(dbuf, smth);
})

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

//TODO: something is wrong with colour
//TODO: wagon itself also renders contained items
#include <df/building_wagonst.h>
BLD_OVR_SIMPLE(building_wagonst, "wagon", 3, 3)

#include <df/building_weaponrackst.h>
BLD_OVR_SIMPLEST(weaponrack)


#include <df/building_wellst.h>
BLD_OVR_SIMPLEST(well)

#include <df/building_windmillst.h>
BLD_OVR_BEGIN(building_windmillst,
{
    DEFINE_SIZE(3,3)
    DEFINE_VARS(normal)
    DEFINE_VARS(active)

    LOAD_BEGIN
    LOAD_IMAGE(normal, "windmill")
    LOAD_IMAGE(active, "windmill-active")
    LOAD_END

    FILL_PLACEHOLDER_2

    //TODO: shouldn't animate if game paused
    df::machine *machine = df::machine::find(this->machine.machine_id);
    if (machine && machine->cur_power > 0)
        BLD_DRAW_IMAGE_AND_RETURN(active)

    BLD_DRAW_IMAGE_OR_DEFAULT(normal)
})


#include <df/building_window_gemst.h>
BLD_OVR_SIMPLEST(window_gem)

#include <df/building_window_glassst.h>
BLD_OVR_SIMPLEST(window_glass)


#include <df/building_workshopst.h>

#define BLD_WORKSHOP_LOADIMG_SIMPLE(name) \
    name##_loaded = load_tiles("data/art/tiles/workshops/" #name ".png", &name##_tiles, &name##_frames, bldw, bldh); \
    ok |= name##_loaded;

#define BLD_WORKSHOP_SIMPLE(type,name) \
    case df::workshop_type::type: \
        BLD_DRAW_IMAGE_AND_RETURN(name) \
        break;

BLD_OVR_BEGIN(building_workshopst,
{
    DEFINE_SIZE(3,3)
    DEFINE_VARS(carpenters)
    DEFINE_VARS(masons)
    DEFINE_VARS(jewelers)
    DEFINE_VARS(farmers)
    DEFINE_VARS(craftsdwarf)
    DEFINE_VARS(forge)
    DEFINE_VARS(magma_forge)
    DEFINE_VARS(bowyers)
    DEFINE_VARS(mechanics)
    DEFINE_VARS(siege)
    DEFINE_VARS(butchers)
    DEFINE_VARS(leatherworks)
    DEFINE_VARS(tanners)
    DEFINE_VARS(clothiers)
    DEFINE_VARS(fishery)
    DEFINE_VARS(still)
    DEFINE_VARS(loom)
    DEFINE_VARS(quern)
    DEFINE_VARS(kennels)
    DEFINE_VARS(kitchen)
    DEFINE_VARS(ashery)
    DEFINE_VARS(dyers)
    DEFINE_VARS(millstone)
    DEFINE_VARS(tool)

    LOAD_BEGIN
    BLD_WORKSHOP_LOADIMG_SIMPLE(carpenters)
    BLD_WORKSHOP_LOADIMG_SIMPLE(masons)
    BLD_WORKSHOP_LOADIMG_SIMPLE(jewelers)
    BLD_WORKSHOP_LOADIMG_SIMPLE(farmers)
    BLD_WORKSHOP_LOADIMG_SIMPLE(craftsdwarf)
    BLD_WORKSHOP_LOADIMG_SIMPLE(forge)
    BLD_WORKSHOP_LOADIMG_SIMPLE(magma_forge)
    BLD_WORKSHOP_LOADIMG_SIMPLE(bowyers)
    BLD_WORKSHOP_LOADIMG_SIMPLE(mechanics)
    BLD_WORKSHOP_LOADIMG_SIMPLE(siege)
    BLD_WORKSHOP_LOADIMG_SIMPLE(butchers)
    BLD_WORKSHOP_LOADIMG_SIMPLE(leatherworks)
    BLD_WORKSHOP_LOADIMG_SIMPLE(tanners)
    BLD_WORKSHOP_LOADIMG_SIMPLE(clothiers)
    BLD_WORKSHOP_LOADIMG_SIMPLE(fishery)
    BLD_WORKSHOP_LOADIMG_SIMPLE(still)
    BLD_WORKSHOP_LOADIMG_SIMPLE(loom)
    BLD_WORKSHOP_LOADIMG_SIMPLE(quern)
    BLD_WORKSHOP_LOADIMG_SIMPLE(kennels)
    BLD_WORKSHOP_LOADIMG_SIMPLE(kitchen)
    BLD_WORKSHOP_LOADIMG_SIMPLE(ashery)
    BLD_WORKSHOP_LOADIMG_SIMPLE(dyers)
    BLD_WORKSHOP_LOADIMG_SIMPLE(millstone)
    BLD_WORKSHOP_LOADIMG_SIMPLE(tool)
    LOAD_END

    FILL_PLACEHOLDER_1

    //TODO: can be done by deriving file name from type, but what if we will want custom states for some of them?
    /*std::string name = df::enum_traits<df::workshop_type>::key_table[this->type];
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);*/

    switch (this->type)
    {
        BLD_WORKSHOP_SIMPLE(Carpenters, carpenters)
        BLD_WORKSHOP_SIMPLE(Masons, masons)
        BLD_WORKSHOP_SIMPLE(Jewelers, jewelers)
        BLD_WORKSHOP_SIMPLE(Farmers, farmers)
        BLD_WORKSHOP_SIMPLE(Craftsdwarfs, craftsdwarf)
        BLD_WORKSHOP_SIMPLE(MetalsmithsForge, forge)
        BLD_WORKSHOP_SIMPLE(MagmaForge, magma_forge)
        BLD_WORKSHOP_SIMPLE(Bowyers, bowyers)
        BLD_WORKSHOP_SIMPLE(Mechanics, mechanics)
        BLD_WORKSHOP_SIMPLE(Siege, siege)
        BLD_WORKSHOP_SIMPLE(Butchers, butchers)
        BLD_WORKSHOP_SIMPLE(Leatherworks, leatherworks)
        BLD_WORKSHOP_SIMPLE(Tanners, tanners)
        BLD_WORKSHOP_SIMPLE(Clothiers, clothiers)
        BLD_WORKSHOP_SIMPLE(Fishery, fishery)
        BLD_WORKSHOP_SIMPLE(Still, still)
        BLD_WORKSHOP_SIMPLE(Loom, loom)
        BLD_WORKSHOP_SIMPLE(Quern, quern)
        BLD_WORKSHOP_SIMPLE(Kennels, kennels)
        BLD_WORKSHOP_SIMPLE(Kitchen, kitchen)
        BLD_WORKSHOP_SIMPLE(Ashery, ashery)
        BLD_WORKSHOP_SIMPLE(Dyers, dyers)
        BLD_WORKSHOP_SIMPLE(Millstone, millstone)
        BLD_WORKSHOP_SIMPLE(Tool, tool)

        //Custom
    }

    BLD_DRAW_DEFAULT
})



void apply_building_hooks()
{
    BLD_HOOK(building_archerytargetst)
    BLD_HOOK(building_armorstandst)
    BLD_HOOK(building_axle_horizontalst)
    BLD_HOOK(building_axle_verticalst)
    BLD_HOOK(building_bars_floorst)
    BLD_HOOK(building_bars_verticalst)
    BLD_HOOK(building_bedst)
    BLD_HOOK(building_boxst)
    BLD_HOOK(building_bridgest)
    BLD_HOOK(building_cabinetst)
    BLD_HOOK(building_cagest)
    BLD_HOOK(building_chainst)
    BLD_HOOK(building_chairst)
    BLD_HOOK(building_coffinst)
    BLD_HOOK(building_doorst)
    BLD_HOOK(building_floodgatest)
    BLD_HOOK(building_furnacest)
    BLD_HOOK(building_gear_assemblyst)
    BLD_HOOK(building_grate_floorst)
    BLD_HOOK(building_grate_wallst)
    BLD_HOOK(building_hatchst)
    BLD_HOOK(building_hivest)
    BLD_HOOK(building_nest_boxst)
    BLD_HOOK(building_tablest)
    BLD_HOOK(building_rollersst)
    BLD_HOOK(building_slabst)
    BLD_HOOK(building_statuest)
    BLD_HOOK(building_supportst)
    BLD_HOOK(building_traction_benchst)
    BLD_HOOK(building_wagonst)
    BLD_HOOK(building_weaponrackst)
    BLD_HOOK(building_wellst)
    BLD_HOOK(building_windmillst)
    BLD_HOOK(building_window_gemst)
    BLD_HOOK(building_window_glassst)
    BLD_HOOK(building_workshopst)
}