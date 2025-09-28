#include <std.h>
#include "../invazio.h"

inherit ARMOUR;

void create() {
    ::create();
    set_name("kozmikus vert");
    set_id(({"vert", "pancel", "kozmikus vert"}));
    set_short("%^BOLD%^%^MAGENTA%^Kozmikus%^RESET%^ %^CYAN%^vért%^RESET%^");
    set_long("Ez a vert egy massziv, pulzalo, kristalyos anyagbol van kifaragva, "
             "amely mintha a kozmosz melyen levo energiat tarolna. Felulete apro, "
             "szikrazo fenyerekkel van tele, mintha egy csillagoktol teli egbolt lenne. "
             "Viselésekor massziv, tulvilagi erot sugaroz, es a benne levo energia "
             "megvedheti viselojet a legpusztitobb csapasoktol is.");
    set_rarity(80);
    set_min_level(50);
    set_weight(300);
    set_value(6000);
    set_type("mellvert");
    set_ac(20);
    set_property("magic item",({"energiapajzs"}));
}