#include <std.h>
#include "../invazio.h"
#include <damages.h>

inherit RUNA_ARMOR;

void create() {
    ::create();
    set_property("nev_mod", "%^BOLD%^%^RED%^Troll %^RESET%^%^GREEN%^harcos gyuruje%^RESET%^");
    set_property("runazhato", 1);
    
    if (!query_property("runazva")) {
        set_name("runazhato troll harcos gyuruje");
        set_short("Runazhato %^BOLD%^%^RED%^Troll %^RESET%^%^GREEN%^gyuruje%^RESET%^");
    }
    
    set_id(({"gyuru", "targy", "troll harcos gyuruje", "runazhato"}));
    
    set_long("Ez a gyuru egyetlen, sima, zoldes szinu kobol van kifaragva, ami a trollok "
             "termeszetes varazsat hordozza. Feluleten apro, voros erek futnak, "
             "mintha a Troll Harcos duhet es erejet jelkepeznek. Viselesekor "
             "massziv, szilard erzest ad.");
             
    set_type("gyuru");
    set_min_level(50);
    set_value(6500);
    set_mass(10);
    set_rarity(100);
    set_ac(30);
    set_invis(1);
    
  
    set_bonus("ero", 10);
    //set_bonus("kitartas", 15);
    set_property("extra_item", 1);
    
    set_only(({"harcosok", "barbarok", "semleges", "gonosz"}));
   // set_property("magic item", ({"szilardsag", "eros akarat"}));
}