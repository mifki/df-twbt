#include "df/item_ammost.h"
#include "df/item_amuletst.h"
#include "df/item_animaltrapst.h"
#include "df/item_anvilst.h"
#include "df/item_armorst.h"
#include "df/item_armorstandst.h"
#include "df/item_backpackst.h"
#include "df/item_ballistaarrowheadst.h"
#include "df/item_ballistapartsst.h"
#include "df/item_barrelst.h"
#include "df/item_barst.h"
#include "df/item_bedst.h"
#include "df/item_binst.h"
#include "df/item_blocksst.h"
#include "df/item_bookst.h"
#include "df/item_boulderst.h"
#include "df/item_boxst.h"
#include "df/item_braceletst.h"
#include "df/item_branchst.h"
#include "df/item_bucketst.h"
#include "df/item_cabinetst.h"
#include "df/item_cagest.h"
#include "df/item_catapultpartsst.h"
#include "df/item_chainst.h"
#include "df/item_chairst.h"
#include "df/item_cheesest.h"
#include "df/item_clothst.h"
#include "df/item_coffinst.h"
#include "df/item_coinst.h"
#include "df/item_corpsepiecest.h"
#include "df/item_corpsest.h"
#include "df/item_crownst.h"
#include "df/item_crutchst.h"
#include "df/item_doorst.h"
#include "df/item_drinkst.h"
#include "df/item_earringst.h"
#include "df/item_eggst.h"
#include "df/item_figurinest.h"
#include "df/item_fish_rawst.h"
#include "df/item_fishst.h"
#include "df/item_flaskst.h"
#include "df/item_floodgatest.h"
#include "df/item_foodst.h"
#include "df/item_gemst.h"
#include "df/item_globst.h"
#include "df/item_glovesst.h"
#include "df/item_gobletst.h"
#include "df/item_gratest.h"
#include "df/item_hatch_coverst.h"
#include "df/item_helmst.h"
#include "df/item_instrumentst.h"
#include "df/item_liquid.h"
#include "df/item_liquid_miscst.h"
#include "df/item_liquipowder.h"
#include "df/item_meatst.h"
#include "df/item_millstonest.h"
#include "df/item_orthopedic_castst.h"
#include "df/item_pantsst.h"
#include "df/item_petst.h"
#include "df/item_pipe_sectionst.h"
#include "df/item_plant_growthst.h"
#include "df/item_plantst.h"
#include "df/item_powder.h"
#include "df/item_powder_miscst.h"
#include "df/item_quernst.h"
#include "df/item_quiverst.h"
#include "df/item_remainsst.h"
#include "df/item_ringst.h"
#include "df/item_rockst.h"
#include "df/item_roughst.h"
#include "df/item_scepterst.h"
#include "df/item_seedsst.h"
#include "df/item_sheetst.h"
#include "df/item_shieldst.h"
#include "df/item_shoesst.h"
#include "df/item_siegeammost.h"
#include "df/item_skin_tannedst.h"
#include "df/item_slabst.h"
#include "df/item_smallgemst.h"
#include "df/item_splintst.h"
#include "df/item_statuest.h"
#include "df/item_tablest.h"
#include "df/item_threadst.h"
#include "df/item_toolst.h"
#include "df/item_totemst.h"
#include "df/item_toyst.h"
#include "df/item_traction_benchst.h"
#include "df/item_trapcompst.h"
#include "df/item_trappartsst.h"
#include "df/item_verminst.h"
#include "df/item_weaponrackst.h"
#include "df/item_weaponst.h"
#include "df/item_windowst.h"
#include "df/item_woodst.h"

//TODO: is Items::getPosition needed?

#define OVER2(cls) \
struct cls##_hook : public df::cls \
{ \
   typedef df::cls interpose_base; \
\
    DEFINE_VMETHOD_INTERPOSE(uint8_t, drawSelf, ()) \
    { \
        renderer_cool *r = (renderer_cool*)enabler->renderer; \
\
        df::coord _pos = Items::getPosition(this); \
                if (pos.x-gwindow_x < 0 || pos.y-gwindow_y < 0 || pos.x-gwindow_x >= r->gdimx || pos.y-gwindow_y >= r->gdimy) *out2 << "!!" << std::endl; \
        if (!((uint32_t*)screen_under_ptr)[(_pos.x-gwindow_x)*r->gdimy + _pos.y-gwindow_y]) \
            ((uint32_t*)screen_under_ptr)[(_pos.x-gwindow_x)*r->gdimy + _pos.y-gwindow_y] = ((uint32_t*)screen_ptr)[(_pos.x-gwindow_x)*r->gdimy + _pos.y-gwindow_y]; \
\
        return INTERPOSE_NEXT(drawSelf)(); \
\
    } \
}; \
IMPLEMENT_VMETHOD_INTERPOSE(cls##_hook, drawSelf);

#define OVER2_ENABLE(cls) INTERPOSE_HOOK(cls##_hook, drawSelf).apply(true);

OVER2(item_ammost);
OVER2(item_amuletst);
OVER2(item_animaltrapst);
OVER2(item_anvilst);
OVER2(item_armorst);
OVER2(item_armorstandst);
OVER2(item_backpackst);
OVER2(item_ballistaarrowheadst);
OVER2(item_ballistapartsst);
OVER2(item_barrelst);
OVER2(item_barst);
OVER2(item_bedst);
OVER2(item_binst);
OVER2(item_blocksst);
OVER2(item_bookst);
OVER2(item_boulderst);
OVER2(item_boxst);    
OVER2(item_braceletst);
OVER2(item_branchst);    
OVER2(item_bucketst);
OVER2(item_cabinetst);
OVER2(item_cagest);
OVER2(item_catapultpartsst);
OVER2(item_chainst);
OVER2(item_chairst);
OVER2(item_cheesest);
OVER2(item_clothst);
OVER2(item_coffinst);
OVER2(item_coinst);
OVER2(item_corpsepiecest);
OVER2(item_corpsest);
OVER2(item_crownst);
OVER2(item_crutchst);
OVER2(item_doorst);
OVER2(item_earringst);
OVER2(item_eggst);
OVER2(item_figurinest);
OVER2(item_fish_rawst);
OVER2(item_flaskst);
OVER2(item_floodgatest);
OVER2(item_fishst);
OVER2(item_foodst);
OVER2(item_gemst);
OVER2(item_globst);
OVER2(item_glovesst);
OVER2(item_gobletst);
OVER2(item_gratest);
OVER2(item_hatch_coverst);
OVER2(item_helmst);
OVER2(item_instrumentst);
OVER2(item_meatst);
OVER2(item_millstonest);
OVER2(item_orthopedic_castst);
OVER2(item_pantsst);
OVER2(item_petst);
OVER2(item_pipe_sectionst);
OVER2(item_plantst);
OVER2(item_plant_growthst);
OVER2(item_quernst);
OVER2(item_quiverst);
OVER2(item_remainsst);
OVER2(item_ringst);
OVER2(item_rockst);
OVER2(item_roughst);
OVER2(item_scepterst);
OVER2(item_seedsst);
OVER2(item_sheetst);
OVER2(item_shieldst);
OVER2(item_shoesst);
OVER2(item_siegeammost);
OVER2(item_skin_tannedst);
OVER2(item_slabst);
OVER2(item_smallgemst);
OVER2(item_splintst);
OVER2(item_statuest);
OVER2(item_tablest);
OVER2(item_threadst);
OVER2(item_toolst);
OVER2(item_totemst);
OVER2(item_toyst);
OVER2(item_traction_benchst);
OVER2(item_trapcompst);
OVER2(item_trappartsst);
OVER2(item_verminst);
OVER2(item_weaponrackst);
OVER2(item_weaponst);
OVER2(item_windowst);
OVER2(item_woodst);

void enable_item_hooks()
{
    OVER2_ENABLE(item_ammost);
    OVER2_ENABLE(item_amuletst);
    OVER2_ENABLE(item_animaltrapst);
    OVER2_ENABLE(item_anvilst);
    OVER2_ENABLE(item_armorst);        
    OVER2_ENABLE(item_armorstandst);
    OVER2_ENABLE(item_backpackst);
    OVER2_ENABLE(item_ballistaarrowheadst);
    OVER2_ENABLE(item_ballistapartsst);
    OVER2_ENABLE(item_barrelst);
    OVER2_ENABLE(item_barst);
    OVER2_ENABLE(item_bedst);
    OVER2_ENABLE(item_binst);
    OVER2_ENABLE(item_blocksst);
    OVER2_ENABLE(item_bookst);
    OVER2_ENABLE(item_boulderst);    
    OVER2_ENABLE(item_boxst);    
    OVER2_ENABLE(item_braceletst);
    OVER2_ENABLE(item_branchst);    
    OVER2_ENABLE(item_bucketst);
    OVER2_ENABLE(item_cabinetst);
    OVER2_ENABLE(item_cagest);
    OVER2_ENABLE(item_catapultpartsst);
    OVER2_ENABLE(item_chainst);
    OVER2_ENABLE(item_chairst);
    OVER2_ENABLE(item_cheesest);
    OVER2_ENABLE(item_clothst);
    OVER2_ENABLE(item_coffinst);
    OVER2_ENABLE(item_coinst);
    OVER2_ENABLE(item_corpsepiecest);
    OVER2_ENABLE(item_corpsest);
    OVER2_ENABLE(item_crownst);
    OVER2_ENABLE(item_crutchst);
    OVER2_ENABLE(item_doorst);
    OVER2_ENABLE(item_earringst);
    OVER2_ENABLE(item_eggst);
    OVER2_ENABLE(item_figurinest);
    OVER2_ENABLE(item_fishst);
    OVER2_ENABLE(item_fish_rawst);
    OVER2_ENABLE(item_flaskst);
    OVER2_ENABLE(item_floodgatest);
    OVER2_ENABLE(item_foodst);
    OVER2_ENABLE(item_gemst);
    OVER2_ENABLE(item_globst);
    OVER2_ENABLE(item_glovesst);        
    OVER2_ENABLE(item_gobletst);        
    OVER2_ENABLE(item_gratest);
    OVER2_ENABLE(item_hatch_coverst);
    OVER2_ENABLE(item_helmst);        
    OVER2_ENABLE(item_instrumentst);
    OVER2_ENABLE(item_meatst);
    OVER2_ENABLE(item_millstonest);
    OVER2_ENABLE(item_orthopedic_castst);
    OVER2_ENABLE(item_pantsst);
    OVER2_ENABLE(item_petst);
    OVER2_ENABLE(item_pipe_sectionst);
    OVER2_ENABLE(item_plantst);
    OVER2_ENABLE(item_plant_growthst);
    OVER2_ENABLE(item_quernst);
    OVER2_ENABLE(item_quiverst);
    OVER2_ENABLE(item_remainsst);
    OVER2_ENABLE(item_ringst);
    OVER2_ENABLE(item_rockst);
    OVER2_ENABLE(item_roughst);
    OVER2_ENABLE(item_scepterst);
    OVER2_ENABLE(item_seedsst);
    OVER2_ENABLE(item_sheetst);
    OVER2_ENABLE(item_shieldst);
    OVER2_ENABLE(item_shoesst);
    OVER2_ENABLE(item_siegeammost);
    OVER2_ENABLE(item_skin_tannedst);
    OVER2_ENABLE(item_slabst);
    OVER2_ENABLE(item_smallgemst);
    OVER2_ENABLE(item_splintst);
    OVER2_ENABLE(item_statuest);
    OVER2_ENABLE(item_tablest);  
    OVER2_ENABLE(item_threadst);  
    OVER2_ENABLE(item_toolst);
    OVER2_ENABLE(item_totemst);
    OVER2_ENABLE(item_toyst);
    OVER2_ENABLE(item_traction_benchst);
    OVER2_ENABLE(item_trapcompst);
    OVER2_ENABLE(item_trappartsst);
    OVER2_ENABLE(item_verminst);
    OVER2_ENABLE(item_weaponrackst);
    OVER2_ENABLE(item_weaponst);
    OVER2_ENABLE(item_windowst);
    OVER2_ENABLE(item_woodst);
}
