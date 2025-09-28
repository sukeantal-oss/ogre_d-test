#include <std.h>
#include <daemons.h>
#include "../invazio.h"

inherit MONSTER;

void create() {
    ::create();
    set_name("komagi golem");
    set_id(({"golem", "komagi golem", "leny"}));
    set_short("Egy %^BOLD%^%^BLACK%^Komagi Golem%^RESET%^");
    set_long("Ez a golem egy idegen, sotét kovetbol van kifaragva, "
             "amelynek feluletebol apro, pulzalo, lila energiakristalyok allnak ki. "
             "A leny hatalmas es lassu, de minden egyes lepesenel a talaj "
             "megremeg. Lathatoan a bolygo energiaja tartja eletben.");
    set_race("kogoem");
    set_gender("ferfi");
    set_level(random(20) + 50);
    set_alignment(0);
    set_money(1100);
    set_aggressive(1);
}