#include <std.h>
#include "../invazio.h"
#include <damages.h>

inherit WEAPON;

void create() {
    ::create();
    set_name("buzogany");
    set_id(({"buzogany", "fegyver", "epic buzogany"}));
    
    set_short("%^BOLD%^%^BLACK%^Viharmart%^RESET%^ %^RED%^buzogany%^RESET%^");
    set_long("Ez a hatalmas buzogany egy massziv, hegyes szikladarabbol keszult, amelyet villamok es viharok martak. "
             "A nyele vastag es durva, mint egy troll harcos keze. "
             "Sulyos, brutalis fegyver, ami kepes csontot es kovet is osszetorni.");
             
    set_type("zuzo");
    set_property("two handed", 1);
    set_min_level(45);
    set_value(5000);
    set_weight(350);
    set_rarity(80);
    set_wc(15, 25);
    set_min_stat("ero", 25);
    set_min_stat("ugyesseg", 15);
    set_bonus("talalatbonusz", 10);
    set_bonus("sebzesbonusz", 12);
}