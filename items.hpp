#define ITEM_DRAW_IMAGE(id) \
    for (int i = 0; i < 1; i++) \
        for (int j = 0; j < 1; j++) \
            gscreen_over[((pos.x+i)*256 + pos.y + j)] = id##_tiles[((TICK % id##_frames) + j*id##_frames) * bldw + i];

#define ITEM_DRAW_IMAGE_OR_DEFAULT(id) { \
    if (IS_LOADED(id)) \
        ITEM_DRAW_IMAGE(id) \
    else \
        INTERPOSE_NEXT(drawSelf)(); \
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
        unhook(); \
        return 0; \
    } \
    loaded = true; \
}

#define ITEM_HOOK(cls) INTERPOSE_HOOK(cls##_twbt, drawSelf).apply(true); 

#define ITEM_OVR_BEGIN(cls,code) \
struct cls##_twbt : public df::cls \
{ \
    typedef df::cls interpose_base; \
\
    void unhook() { \
        INTERPOSE_HOOK(cls##_twbt, drawSelf).apply(false); \
    } \
    DEFINE_VMETHOD_INTERPOSE(uint8_t, drawSelf, ()) \
    { \
        DEFINE_TICK \
        code \
    } \
}; \
IMPLEMENT_VMETHOD_INTERPOSE(cls##_twbt, drawSelf);

#define ITEM_OVR_END(cls) \
    } \
}; \
IMPLEMENT_VMETHOD_INTERPOSE(cls##_twbt, drawBuilding);

#define ITEM_OVR_SIMPLE(cls, fn) \
ITEM_OVR_BEGIN(cls, \
{ \
    DEFINE_SIZE(1,1) \
    DEFINE_VARS(normal) \
\
    LOAD_BEGIN \
    LOAD_IMAGE(normal, fn) \
    ITEM_LOAD_END \
\
    ITEM_DRAW_IMAGE(normal) \
    return 0xfe + (TICK%2); \
})

#define ITEM_OVR_SIMPLEST(name) \
    ITEM_OVR_SIMPLE(item_##name##st, #name)



#include <df/item_ammost.h>
ITEM_OVR_SIMPLEST(ammo)
#include <df/item_amuletst.h>
ITEM_OVR_SIMPLEST(amulet)
#include <df/item_animaltrapst.h>
ITEM_OVR_SIMPLEST(animaltrap)
#include <df/item_anvilst.h>
ITEM_OVR_SIMPLEST(anvil)
#include <df/item_armorst.h>
ITEM_OVR_SIMPLEST(armor)
#include <df/item_armorstandst.h>
ITEM_OVR_SIMPLEST(armorstand)
#include <df/item_backpackst.h>
ITEM_OVR_SIMPLEST(backpack)
#include <df/item_ballistaarrowheadst.h>
ITEM_OVR_SIMPLEST(ballistaarrowhead)
#include <df/item_ballistapartsst.h>
ITEM_OVR_SIMPLEST(ballistaparts)
#include <df/item_barrelst.h>
ITEM_OVR_SIMPLEST(barrel)
#include <df/item_barst.h>
ITEM_OVR_SIMPLEST(bar)
#include <df/item_bedst.h>
ITEM_OVR_SIMPLEST(bed)
#include <df/item_binst.h>
ITEM_OVR_SIMPLEST(bin)
#include <df/item_blocksst.h>
ITEM_OVR_SIMPLEST(blocks)
#include <df/item_body_component.h>
#include <df/item_bookst.h>
ITEM_OVR_SIMPLEST(book)
#include <df/item_boulderst.h>
ITEM_OVR_SIMPLEST(boulder)
#include <df/item_boxst.h>
ITEM_OVR_SIMPLEST(box)
#include <df/item_braceletst.h>
ITEM_OVR_SIMPLEST(bracelet)
#include <df/item_bucketst.h>
ITEM_OVR_SIMPLEST(bucket)
#include <df/item_cabinetst.h>
ITEM_OVR_SIMPLEST(cabinet)
#include <df/item_cagest.h>
ITEM_OVR_SIMPLEST(cage)
#include <df/item_catapultpartsst.h>
ITEM_OVR_SIMPLEST(catapultparts)
#include <df/item_chainst.h>
ITEM_OVR_SIMPLEST(chain)
#include <df/item_chairst.h>
ITEM_OVR_SIMPLEST(chair)
#include <df/item_cheesest.h>
ITEM_OVR_SIMPLEST(cheese)
#include <df/item_clothst.h>
ITEM_OVR_SIMPLEST(cloth)
#include <df/item_coffinst.h>
ITEM_OVR_SIMPLEST(coffin)
#include <df/item_coinst.h>
ITEM_OVR_SIMPLEST(coin)
#include <df/item_corpsepiecest.h>
ITEM_OVR_SIMPLEST(corpsepiece)
#include <df/item_corpsest.h>
ITEM_OVR_SIMPLEST(corpse)
#include <df/item_critter.h>
#include <df/item_crownst.h>
ITEM_OVR_SIMPLEST(crown)
#include <df/item_crutchst.h>
ITEM_OVR_SIMPLEST(crutch)
#include <df/item_doorst.h>
ITEM_OVR_SIMPLEST(door)
#include <df/item_drinkst.h>
ITEM_OVR_SIMPLEST(drink)
#include <df/item_earringst.h>
ITEM_OVR_SIMPLEST(earring)
#include <df/item_eggst.h>
ITEM_OVR_SIMPLEST(egg)
#include <df/item_figurinest.h>
ITEM_OVR_SIMPLEST(figurine)
#include <df/item_fish_rawst.h>
ITEM_OVR_SIMPLEST(fish_raw)
#include <df/item_fishst.h>
ITEM_OVR_SIMPLEST(fish)
#include <df/item_flaskst.h>
ITEM_OVR_SIMPLEST(flask)
#include <df/item_floodgatest.h>
ITEM_OVR_SIMPLEST(floodgate)
#include <df/item_foodst.h>
ITEM_OVR_SIMPLEST(food)
#include <df/item_gemst.h>
ITEM_OVR_SIMPLEST(gem)
#include <df/item_globst.h>
ITEM_OVR_SIMPLEST(glob)
#include <df/item_glovesst.h>
ITEM_OVR_SIMPLEST(gloves)
#include <df/item_gobletst.h>
ITEM_OVR_SIMPLEST(goblet)
#include <df/item_gratest.h>
ITEM_OVR_SIMPLEST(grate)
#include <df/item_hatch_coverst.h>
ITEM_OVR_SIMPLEST(hatch_cover)
#include <df/item_helmst.h>
ITEM_OVR_SIMPLEST(helm)
#include <df/item_instrumentst.h>
ITEM_OVR_SIMPLEST(instrument)
#include <df/item_liquid_miscst.h>
ITEM_OVR_SIMPLEST(liquid_misc)
#include <df/item_meatst.h>
ITEM_OVR_SIMPLEST(meat)
#include <df/item_millstonest.h>
ITEM_OVR_SIMPLEST(millstone)
#include <df/item_orthopedic_castst.h>
ITEM_OVR_SIMPLEST(orthopedic_cast)
#include <df/item_pantsst.h>
ITEM_OVR_SIMPLEST(pants)
#include <df/item_petst.h>
ITEM_OVR_SIMPLEST(pet)
#include <df/item_pipe_sectionst.h>
ITEM_OVR_SIMPLEST(pipe_section)
#include <df/item_plant_growthst.h>
ITEM_OVR_SIMPLEST(plant_growth)
#include <df/item_plantst.h>
ITEM_OVR_SIMPLEST(plant)
#include <df/item_powder_miscst.h>
ITEM_OVR_SIMPLEST(powder_misc)
#include <df/item_quernst.h>
ITEM_OVR_SIMPLEST(quern)
#include <df/item_quiverst.h>
ITEM_OVR_SIMPLEST(quiver)
#include <df/item_remainsst.h>
ITEM_OVR_SIMPLEST(remains)
#include <df/item_ringst.h>
ITEM_OVR_SIMPLEST(ring)
#include <df/item_rockst.h>
ITEM_OVR_SIMPLEST(rock)
#include <df/item_roughst.h>
ITEM_OVR_SIMPLEST(rough)
#include <df/item_scepterst.h>
ITEM_OVR_SIMPLEST(scepter)
#include <df/item_seedsst.h>
ITEM_OVR_SIMPLEST(seeds)
#include <df/item_shieldst.h>
ITEM_OVR_SIMPLEST(shield)
#include <df/item_shoesst.h>
ITEM_OVR_SIMPLEST(shoes)
#include <df/item_siegeammost.h>
ITEM_OVR_SIMPLEST(siegeammo)
#include <df/item_skin_tannedst.h>
ITEM_OVR_SIMPLEST(skin_tanned)
#include <df/item_slabst.h>
ITEM_OVR_SIMPLEST(slab)
#include <df/item_smallgemst.h>
ITEM_OVR_SIMPLEST(smallgem)
#include <df/item_splintst.h>
ITEM_OVR_SIMPLEST(splint)
#include <df/item_statuest.h>
ITEM_OVR_SIMPLEST(statue)
#include <df/item_tablest.h>
ITEM_OVR_SIMPLEST(table)
#include <df/item_threadst.h>
ITEM_OVR_SIMPLEST(thread)
#include <df/item_toolst.h>
ITEM_OVR_SIMPLEST(tool)
#include <df/item_totemst.h>
ITEM_OVR_SIMPLEST(totem)
#include <df/item_toyst.h>
ITEM_OVR_SIMPLEST(toy)
#include <df/item_traction_benchst.h>
ITEM_OVR_SIMPLEST(traction_bench)
#include <df/item_trapcompst.h>
ITEM_OVR_SIMPLEST(trapcomp)
#include <df/item_trappartsst.h>
ITEM_OVR_SIMPLEST(trapparts)
#include <df/item_verminst.h>
ITEM_OVR_SIMPLEST(vermin)
#include <df/item_weaponrackst.h>
ITEM_OVR_SIMPLEST(weaponrack)
#include <df/item_weaponst.h>
ITEM_OVR_SIMPLEST(weapon)
#include <df/item_windowst.h>
ITEM_OVR_SIMPLEST(window)
#include <df/item_woodst.h>
ITEM_OVR_SIMPLEST(wood)

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