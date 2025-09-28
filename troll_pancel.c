#include <std.h>

inherit ARMOUR;

void create() {
    ::create();
    set_name("troll pancel");
    set_id(({"pancel","vert","troll pancel"}));
    set_short("%^BOLD%^%^GREEN%^Troll %^RESET%^%^RED%^pancel%^RESET%^");
    set_long("Ez egy massziv, durva pancel, amelyet a trollok vastag, "
             "bortelen borbol es a harcban elvesztett kovakbol keszitettek. "
             "A felulete karcoktol es vagásoktól tarkazott, ami a pancel viselojenek "
             "kemény, harcedzett eletmodjarol arulkodik.");
    set_rarity(80);
    set_min_level(50);
    set_weight(600);
    set_value(4500);
    set_type("pancel");
    set_ac(75);
    set_property("magic item",({"regeneracio"}));
    set_bonus("kitartas", 15);
    set_bonus("ero", 10);
    
    set_only(({"harcosok","barbarok","gladiatorok","gonosz", "semleges"}));
}