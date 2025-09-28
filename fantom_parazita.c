#include <std.h>
#include <daemons.h>
#include "../invazio.h"

inherit MONSTER;

void create() {
    ::create();
    set_name("fantom-parazita");
    set_id(({"parazita", "fantom-parazita", "leny"}));
    set_short("Egy %^BOLD%^%^MAGENTA%^Fantom-Parazita%^RESET%^");
    set_long("Ez a leny egy energia forma, amely lila es kek fenyekbol "
             "epul fel. Nincsenek kezei vagy labai, csak egy vibralo, "
             "humanoid forma, ami lassan lebeg a levegoben. A leny "
             "kozelesegeben fejfajas es zsibbadtsag erzi magat.");
    set_race("eteri");
    set_gender("semleges");
    set_level(random(25) + 55);
    set_alignment(0);
    set_money(1200);
}