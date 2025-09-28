#include <std.h>
#include "../invazio.h"
#include <damages.h>

inherit RUNA_WEAPON;

void create() {
    ::create();
    set_property("nev_mod", "%^BOLD%^%^CYAN%^Troll %^RESET%^%^RED%^harcos buzogánya%^RESET%^");
    set_property("runazhato", 1);
    
    if (!query_property("runazva")) {
        set_name("runazhato troll harcos buzogany");
        set_short("Runazhato %^BOLD%^%^CYAN%^Troll %^RESET%^%^RED%^buzoganya%^RESET%^");
    }
    
    set_id(({"buzogany", "fegyver", "runazhato", "troll harcos buzogany"}));
    
    set_long("Ez a buzogány egy szornyeteg altal keszitett, hatalmas, kotoro fegyver. "
             "Feje egy eles, fekete szikla, amelyet troll verrel edzettek. "
             "A nyele vastag, durva fa, amelyet szilardan lehet fogni. "
             "A fegyver pusztito erot sugaroz.");
             
    set_type("zuzo");
    set_property("two handed", 1);
    set_min_level(50);
    set_value(7500);
    set_weight(400); 
    set_rarity(100);
    set_wc(18, 28); 
    set_min_stat("ero", 30); 
    set_min_stat("ugyesseg", 20); 
    set_property("poisoned", -1);
    set_property("poison", "idegmereg");
    //set_property("magic item", ({"demoni aura", "pusztito csapas"}));
    set_property("extra_item", 1);
    
    set_only(({"harcosok", "barbarok", "semleges", "gonosz"}));
}