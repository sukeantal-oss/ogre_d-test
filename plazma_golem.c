#include <std.h>
#include <daemons.h>
#include <damages.h>
#include "../invazio.h"

inherit MONSTER;

void create() {
    ::create();
    set_name("plazma golem");
    set_id(({"golem", "plazma golem", "szorny"}));
    set_short("Egy %^BOLD%^%^CYAN%^Plazma Golem%^RESET%^");
    set_long("Ez a golem egy pulzalo energia forma, amelyet sötét, "
             "fenyelnyelo anyagok tartanak egyben. Nincsenek szemei, de erezheto, "
             "hogy minden mozdulatod koveti. A testebol apro plazma szikrak "
             "szallnak a levegoben, es a kozelsegeben a levego is lassan felhevul.");
    set_race("golem");
    set_gender("ferfi");
    set_level(random(20) + 50);
    set_alignment(0);
    set_money(1000);
    set_max_hp(1200);
    set_hp(1200);
}
