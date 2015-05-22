#define ITEM_DRAW_IMAGE(id) \
    df::coord _pos = Items::getPosition(this); \
    for (int i = 0; i < 1; i++) \
        for (int j = 0; j < 1; j++) \
            gscreen_over[((_pos.x+i)*256 + _pos.y + j)] = id##_tiles[((TICK % id##_frames) + j*id##_frames) * bldw + i];

#define ITEM_DRAW_IMAGE2(id) \
    df::coord _pos = Items::getPosition((T*)this); \
    for (int i = 0; i < 1; i++) \
        for (int j = 0; j < 1; j++) \
            gscreen_over[((_pos.x+i)*256 + _pos.y + j)] = id##_tiles[((TICK % id##_frames) + j*id##_frames) * bldw + i];



#define ITEM_DRAW_IMAGE_OR_DEFAULT(id) { \
    if (IS_LOADED(id)) \
        ITEM_DRAW_IMAGE(id) \
    else \
        return INTERPOSE_NEXT(drawSelf)(); \
}

#define ITEM_DRAW_IMAGE_OR_DEFAULT2(id) { \
    if (IS_LOADED(id)) { \
        ITEM_DRAW_IMAGE2(id) \
        return true; \
    } else \
        return false; \
}




#define ITEM_DRAW_DEFAULT return INTERPOSE_NEXT(drawSelf)();

#define ITEM_DRAW_IMAGE_AND_RETURN(id) { \
    if (IS_LOADED(id)) { \
        ITEM_DRAW_IMAGE(id) \
        return 0xfe + (TICK%2); \
    } \
}

#define ITEM_LOAD_END \
    if (!ok) \
    { \
        return 0; \
    } \
    loaded = true; \
}

#define ITEM_HOOK(cls) INTERPOSE_HOOK(cls##_twbt, drawSelf).apply(true); 

template <typename T, typename Q>
struct ovr_custom 
{
    DEFINE_VARS_DYN()

    bool handle_custom(std::string folder)
    {
        DEFINE_TICK
        DEFINE_SIZE(1,1)

        if (!loaded)
        {
            int count = Q::get_vector().size();
            loaded = new bool[count]; bzero(loaded, count*sizeof(bool));
            tried = new bool[count]; bzero(tried, count*sizeof(bool));
            frames = new int[count]; bzero(frames, count*sizeof(int));
            tiles = new long*[count]; bzero(tiles, count*sizeof(long*));
        }        

        int subtype = ((T*)this)->getSubtype();
        if (!tried[subtype])
        {
            Q *itemdef = Q::get_vector()[subtype];
            std::string fn = "data/art/tiles/" + folder + "/" + itemdef->id + ".png";
            loaded[subtype] = load_tiles(fn.c_str(), &tiles[subtype], &frames[subtype], bldw, bldh);
            tried[subtype] = true;
        }
        if (loaded[subtype])
        {
            for (int i = 0; i < 1; i++)
                for (int j = 0; j < 1; j++)
                    gscreen_over[((((T*)this)->pos.x+i)*256 + ((T*)this)->pos.y + j)] = tiles[subtype][((TICK % frames[subtype]) + j*frames[subtype]) * bldw + i];
            return true;
        }

        return false;
    }
};
template <typename T, typename Q> bool *ovr_custom<T,Q>::loaded(0);
template <typename T, typename Q> bool *ovr_custom<T,Q>::tried(0);
template <typename T, typename Q> long **ovr_custom<T,Q>::tiles(0);
template <typename T, typename Q> int *ovr_custom<T,Q>::frames(0);

template <typename T>
struct ovr_simple
{
    bool handle_simple(std::string name)
    {
        DEFINE_TICK
        DEFINE_SIZE(1,1)
        DEFINE_VARS(normal)

        {
            LOAD_BEGIN
            normal_loaded = load_tiles(("data/art/tiles/" + name + ".png").c_str(), &normal_tiles, &normal_frames, bldw, bldh); \
            //LOAD_IMAGE(normal, fn)
            ok = true;
            ITEM_LOAD_END

            ITEM_DRAW_IMAGE_OR_DEFAULT2(normal)
        }                
    }
};

#define ITEM_OVR_BEGIN2(cls, itemdefcls) \
struct cls##_twbt : public df::cls, ovr_custom<df::cls, df::itemdefcls>, ovr_simple<df::cls> \
{ \
    typedef df::cls interpose_base; \
\
    void unhook() { \
        INTERPOSE_HOOK(cls##_twbt, drawSelf).apply(false); \
    } \

#define ITEM_OVR_END2(cls) \
}; \
IMPLEMENT_VMETHOD_INTERPOSE(cls##_twbt, drawSelf);

#define ITEM_OVR_BODY2(code) \
    DEFINE_VMETHOD_INTERPOSE(uint8_t, drawSelf, ()) \
    { \
        DEFINE_TICK \
        code \
    } \


#define ITEM_OVR_SUBTYPES_SIMPLEST(cls) \
ITEM_OVR_BEGIN2(item_##cls##st, itemdef_##cls##st) \
ITEM_OVR_BODY2( \
{ \
    if (handle_custom(#cls) || handle_simple(#cls)) \
        return 0xfe + (TICK%2); \
\
    return INTERPOSE_NEXT(drawSelf)(); \
}) \
ITEM_OVR_END2(item_##cls##st)


#define ITEM_OVR_SIMPLEST2(cls) \
struct item_##cls##st##_twbt : public df::item_##cls##st, ovr_simple<df::item_##cls##st> \
{ \
    typedef df::item_##cls##st interpose_base; \
\
    void unhook() { \
        INTERPOSE_HOOK(item_##cls##st##_twbt, drawSelf).apply(false); \
    } \
    ITEM_OVR_BODY2( \
    { \
        if (handle_simple(#cls)) \
            return 0xfe + (TICK%2); \
    \
        uint8_t t = INTERPOSE_NEXT(drawSelf)(); \
        unhook(); \
        return t; \
    }) \
ITEM_OVR_END2(item_##cls##st)





#include <df/item_ammost.h>
#include <df/itemdef_ammost.h>
ITEM_OVR_SUBTYPES_SIMPLEST(ammo)
#include <df/item_amuletst.h>
ITEM_OVR_SIMPLEST2(amulet)
#include <df/item_animaltrapst.h>
ITEM_OVR_SIMPLEST2(animaltrap)
#include <df/item_anvilst.h>
ITEM_OVR_SIMPLEST2(anvil)
#include <df/item_armorst.h>
#include <df/itemdef_armorst.h>
ITEM_OVR_SUBTYPES_SIMPLEST(armor)
#include <df/item_armorstandst.h>
ITEM_OVR_SIMPLEST2(armorstand)
#include <df/item_backpackst.h>
ITEM_OVR_SIMPLEST2(backpack)
#include <df/item_ballistaarrowheadst.h>
ITEM_OVR_SIMPLEST2(ballistaarrowhead)
#include <df/item_ballistapartsst.h>
ITEM_OVR_SIMPLEST2(ballistaparts)
#include <df/item_barrelst.h>
ITEM_OVR_SIMPLEST2(barrel)
#include <df/item_barst.h>
ITEM_OVR_SIMPLEST2(bar)
#include <df/item_bedst.h>
ITEM_OVR_SIMPLEST2(bed)
#include <df/item_binst.h>
ITEM_OVR_SIMPLEST2(bin)
#include <df/item_blocksst.h>
ITEM_OVR_SIMPLEST2(blocks)
#include <df/item_body_component.h>
#include <df/item_bookst.h>
ITEM_OVR_SIMPLEST2(book)
#include <df/item_boulderst.h>
ITEM_OVR_SIMPLEST2(boulder)
#include <df/item_boxst.h>
ITEM_OVR_SIMPLEST2(box)
#include <df/item_braceletst.h>
ITEM_OVR_SIMPLEST2(bracelet)
#include <df/item_bucketst.h>
ITEM_OVR_SIMPLEST2(bucket)
#include <df/item_cabinetst.h>
ITEM_OVR_SIMPLEST2(cabinet)
#include <df/item_cagest.h>
ITEM_OVR_SIMPLEST2(cage)
#include <df/item_catapultpartsst.h>
ITEM_OVR_SIMPLEST2(catapultparts)
#include <df/item_chainst.h>
ITEM_OVR_SIMPLEST2(chain)
#include <df/item_chairst.h>
ITEM_OVR_SIMPLEST2(chair)
#include <df/item_cheesest.h>
ITEM_OVR_SIMPLEST2(cheese)
#include <df/item_clothst.h>
ITEM_OVR_SIMPLEST2(cloth)
#include <df/item_coffinst.h>
ITEM_OVR_SIMPLEST2(coffin)
#include <df/item_coinst.h>
ITEM_OVR_SIMPLEST2(coin)
#include <df/item_corpsepiecest.h>
ITEM_OVR_SIMPLEST2(corpsepiece)
#include <df/item_corpsest.h>
ITEM_OVR_SIMPLEST2(corpse)
#include <df/item_critter.h>
#include <df/item_crownst.h>
ITEM_OVR_SIMPLEST2(crown)
#include <df/item_crutchst.h>
ITEM_OVR_SIMPLEST2(crutch)
#include <df/item_doorst.h>
ITEM_OVR_SIMPLEST2(door)
#include <df/item_drinkst.h>
ITEM_OVR_SIMPLEST2(drink)
#include <df/item_earringst.h>
ITEM_OVR_SIMPLEST2(earring)
#include <df/item_eggst.h>
ITEM_OVR_SIMPLEST2(egg)
#include <df/item_figurinest.h>
ITEM_OVR_SIMPLEST2(figurine)
#include <df/item_fish_rawst.h>
ITEM_OVR_SIMPLEST2(fish_raw)
#include <df/item_fishst.h>
ITEM_OVR_SIMPLEST2(fish)
#include <df/item_flaskst.h>
ITEM_OVR_SIMPLEST2(flask)
#include <df/item_floodgatest.h>
ITEM_OVR_SIMPLEST2(floodgate)
#include <df/item_foodst.h>
#include <df/itemdef_foodst.h>
ITEM_OVR_SUBTYPES_SIMPLEST(food)
#include <df/item_gemst.h>
ITEM_OVR_SIMPLEST2(gem)
#include <df/item_globst.h>
ITEM_OVR_SIMPLEST2(glob)
#include <df/item_glovesst.h>
#include <df/itemdef_glovesst.h>
ITEM_OVR_SUBTYPES_SIMPLEST(gloves)
#include <df/item_gobletst.h>
ITEM_OVR_SIMPLEST2(goblet)
#include <df/item_gratest.h>
ITEM_OVR_SIMPLEST2(grate)
#include <df/item_hatch_coverst.h>
ITEM_OVR_SIMPLEST2(hatch_cover)
#include <df/item_helmst.h>
#include <df/itemdef_helmst.h>
ITEM_OVR_SUBTYPES_SIMPLEST(helm)
#include <df/item_instrumentst.h>
#include <df/itemdef_instrumentst.h>
ITEM_OVR_SUBTYPES_SIMPLEST(instrument)
#include <df/item_liquid_miscst.h>
ITEM_OVR_SIMPLEST2(liquid_misc)
#include <df/item_meatst.h>
ITEM_OVR_SIMPLEST2(meat)
#include <df/item_millstonest.h>
ITEM_OVR_SIMPLEST2(millstone)
#include <df/item_orthopedic_castst.h>
ITEM_OVR_SIMPLEST2(orthopedic_cast)
#include <df/item_pantsst.h>
#include <df/itemdef_pantsst.h>
ITEM_OVR_SUBTYPES_SIMPLEST(pants)
#include <df/item_petst.h>
ITEM_OVR_SIMPLEST2(pet)
#include <df/item_pipe_sectionst.h>
ITEM_OVR_SIMPLEST2(pipe_section)
#include <df/item_plant_growthst.h>
ITEM_OVR_SIMPLEST2(plant_growth)
#include <df/item_plantst.h>
ITEM_OVR_SIMPLEST2(plant)
#include <df/item_powder_miscst.h>
ITEM_OVR_SIMPLEST2(powder_misc)
#include <df/item_quernst.h>
ITEM_OVR_SIMPLEST2(quern)
#include <df/item_quiverst.h>
ITEM_OVR_SIMPLEST2(quiver)
#include <df/item_remainsst.h>
ITEM_OVR_SIMPLEST2(remains)
#include <df/item_ringst.h>
ITEM_OVR_SIMPLEST2(ring)
#include <df/item_rockst.h>
ITEM_OVR_SIMPLEST2(rock)
#include <df/item_roughst.h>
ITEM_OVR_SIMPLEST2(rough)
#include <df/item_scepterst.h>
ITEM_OVR_SIMPLEST2(scepter)
#include <df/item_seedsst.h>
ITEM_OVR_SIMPLEST2(seeds)
#include <df/item_shieldst.h>
#include <df/itemdef_shieldst.h>
ITEM_OVR_SUBTYPES_SIMPLEST(shield)
#include <df/item_shoesst.h>
#include <df/itemdef_shoesst.h>
ITEM_OVR_SUBTYPES_SIMPLEST(shoes)
#include <df/item_siegeammost.h>
#include <df/itemdef_siegeammost.h>
ITEM_OVR_SUBTYPES_SIMPLEST(siegeammo)
#include <df/item_skin_tannedst.h>
ITEM_OVR_SIMPLEST2(skin_tanned)
#include <df/item_slabst.h>
ITEM_OVR_SIMPLEST2(slab)
#include <df/item_smallgemst.h>
ITEM_OVR_SIMPLEST2(smallgem)
#include <df/item_splintst.h>
ITEM_OVR_SIMPLEST2(splint)
#include <df/item_statuest.h>
ITEM_OVR_SIMPLEST2(statue)
#include <df/item_tablest.h>
ITEM_OVR_SIMPLEST2(table)
#include <df/item_threadst.h>
ITEM_OVR_SIMPLEST2(thread)
#include <df/item_toolst.h>
#include <df/itemdef_toolst.h>
ITEM_OVR_SUBTYPES_SIMPLEST(tool)
#include <df/item_totemst.h>
ITEM_OVR_SIMPLEST2(totem)
#include <df/item_toyst.h>
#include <df/itemdef_toyst.h>
ITEM_OVR_SUBTYPES_SIMPLEST(toy)
#include <df/item_traction_benchst.h>
ITEM_OVR_SIMPLEST2(traction_bench)
#include <df/item_trapcompst.h>
#include <df/itemdef_trapcompst.h>
ITEM_OVR_SUBTYPES_SIMPLEST(trapcomp)
#include <df/item_trappartsst.h>
ITEM_OVR_SIMPLEST2(trapparts)
#include <df/item_verminst.h>
ITEM_OVR_SIMPLEST2(vermin)
#include <df/item_weaponrackst.h>
ITEM_OVR_SIMPLEST2(weaponrack)
#include <df/item_weaponst.h>
#include <df/itemdef_weaponst.h>
ITEM_OVR_SUBTYPES_SIMPLEST(weapon)
#include <df/item_windowst.h>
ITEM_OVR_SIMPLEST2(window)
#include <df/item_woodst.h>
ITEM_OVR_SIMPLEST2(wood)

void apply_item_hooks()
{
    ITEM_HOOK(item_ammost)
    ITEM_HOOK(item_amuletst)
    ITEM_HOOK(item_animaltrapst)
    ITEM_HOOK(item_anvilst)
    ITEM_HOOK(item_armorst)
    ITEM_HOOK(item_armorstandst)
    ITEM_HOOK(item_backpackst)
    ITEM_HOOK(item_ballistaarrowheadst)
    ITEM_HOOK(item_ballistapartsst)
    ITEM_HOOK(item_barrelst)
    ITEM_HOOK(item_barst)
    ITEM_HOOK(item_bedst)
    ITEM_HOOK(item_binst)
    ITEM_HOOK(item_blocksst)
    ITEM_HOOK(item_bookst)
    ITEM_HOOK(item_boulderst)
    ITEM_HOOK(item_boxst)
    ITEM_HOOK(item_braceletst)
    ITEM_HOOK(item_bucketst)
    ITEM_HOOK(item_cabinetst)
    ITEM_HOOK(item_cagest)
    ITEM_HOOK(item_catapultpartsst)
    ITEM_HOOK(item_chainst)
    ITEM_HOOK(item_chairst)
    ITEM_HOOK(item_cheesest)
    ITEM_HOOK(item_clothst)
    ITEM_HOOK(item_coffinst)
    ITEM_HOOK(item_coinst)
    ITEM_HOOK(item_corpsepiecest)
    ITEM_HOOK(item_corpsest)
    ITEM_HOOK(item_crownst)
    ITEM_HOOK(item_crutchst)
    ITEM_HOOK(item_doorst)
    ITEM_HOOK(item_drinkst)
    ITEM_HOOK(item_earringst)
    ITEM_HOOK(item_eggst)
    ITEM_HOOK(item_figurinest)
    ITEM_HOOK(item_fish_rawst)
    ITEM_HOOK(item_fishst)
    ITEM_HOOK(item_flaskst)
    ITEM_HOOK(item_floodgatest)
    ITEM_HOOK(item_foodst)
    ITEM_HOOK(item_gemst)
    ITEM_HOOK(item_globst)
    ITEM_HOOK(item_glovesst)
    ITEM_HOOK(item_gobletst)
    ITEM_HOOK(item_gratest)
    ITEM_HOOK(item_hatch_coverst)
    ITEM_HOOK(item_helmst)
    ITEM_HOOK(item_instrumentst)
    ITEM_HOOK(item_liquid_miscst)
    ITEM_HOOK(item_meatst)
    ITEM_HOOK(item_millstonest)
    ITEM_HOOK(item_orthopedic_castst)
    ITEM_HOOK(item_pantsst)
    ITEM_HOOK(item_petst)
    ITEM_HOOK(item_pipe_sectionst)
    ITEM_HOOK(item_plant_growthst)
    ITEM_HOOK(item_plantst)
    ITEM_HOOK(item_powder_miscst)
    ITEM_HOOK(item_quernst)
    ITEM_HOOK(item_quiverst)
    ITEM_HOOK(item_remainsst)
    ITEM_HOOK(item_ringst)
    ITEM_HOOK(item_rockst)
    ITEM_HOOK(item_roughst)
    ITEM_HOOK(item_scepterst)
    ITEM_HOOK(item_seedsst)
    ITEM_HOOK(item_shieldst)
    ITEM_HOOK(item_shoesst)
    ITEM_HOOK(item_siegeammost)
    ITEM_HOOK(item_skin_tannedst)
    ITEM_HOOK(item_slabst)
    ITEM_HOOK(item_smallgemst)
    ITEM_HOOK(item_splintst)
    ITEM_HOOK(item_statuest)
    ITEM_HOOK(item_tablest)
    ITEM_HOOK(item_threadst)
    ITEM_HOOK(item_toolst)
    ITEM_HOOK(item_totemst)
    ITEM_HOOK(item_toyst)
    ITEM_HOOK(item_traction_benchst)
    ITEM_HOOK(item_trapcompst)
    ITEM_HOOK(item_trappartsst)
    ITEM_HOOK(item_verminst)
    ITEM_HOOK(item_weaponrackst)
    ITEM_HOOK(item_weaponst)
    ITEM_HOOK(item_windowst)
    ITEM_HOOK(item_woodst)
}