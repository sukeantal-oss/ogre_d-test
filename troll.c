// Troll harcos NPC kodja.
#include <std.h>
#include <damages.h>
#include "../invazio.h"
#include <std.h>

inherit MONSTER;

#define BASE_LVL 60 

void klassz() {
    int class_index;
    object ob = TO;
    string *CLASSES;


    if (!random(2)) {
        CLASSES = ({ "barbar", "gladiator", "kardmuvesz", "hirnok", "saman", "arnyjaro", "bard", "fejvadasz", "utonallo", "demonmagus" });
        ob->set_alignment(-1500);
    } else {
        CLASSES = ({ "barbar", "erdojaro", "kardmuvesz", "druida", "saman", "bard", "fejvadasz", "renegat", "demonmagus" });
        ob->set_alignment(0);
    }

    class_index = random(sizeof(CLASSES));
    ob->set_class(CLASSES[class_index]);
    
    // A szint beallitasa az alap szint es egy veletlenszeru ertek osszegekent.
    ob->set_level(BASE_LVL + random(20)); // Peldaul, a szint 60 es 79 kozott lesz
    
    // Opcionalis: a tobbi szint beallitasa ugyanigy, a tobbsztalyos rendszerhez
    ob->set_level2(BASE_LVL + random(20));
    ob->set_level3(BASE_LVL + random(20));

    // Opcionalis: extra osztalyok, ha a szint eler egy bizonyos erteket
    if (ob->query_level() >= 75) {
        ob->set_extra_classes(({"tuzmagus", "harcimagus", "kardmuvesz"}));
    }
}


void create() {
    ::create(); 
    klassz(); 
    set_name("Troll Harcos");
    set_id( ({ "troll", "harcos", "trollharcos", "szorny" }) );
    set_short("%^BOLD%^%^GREEN%^Egy massziv Troll Harcos%^RESET%^");
    set_long("Ez a troll hatalmas es izmos, a teste hegektol boritott. "
             "Egy kotoro buzoganyt hord a kezeben, ami arra utal, hogy "
             "hosszu es brutalis harcokon ment keresztul. A szemei "
             "gonoszsagot sugaroznak.");
    set_race("troll");
    set_gender("ferfi");  
    set_money(50000 + random(25000));
    set_max_hp(query_level() * 30);
    set_hp(query_max_hp());   
    set_speech(10, ({
        "Megtepem a fejedet!",
        "Nem menekulsz a buzoganyomtol!",
        "A csontjaidat porra zuzom!",
        "Hahaha, gyenge vagy, mint egy ember!"
    }), 0 );
    
    set_emotes(5, ({
        "A Troll Harcos duhosen morog.",
        "A Troll Harcos a mellkasat veri a hatalmas oklevel.",
        "A Troll Harcos hangosan kuncog a pusztitas gondolatan."
    }), 0 );
    set_equipment( ({
        OBJS+"epic_buzogany",
        OBJS+"troll_pancel" 
    }) );
}

void attack_die(object killer) {
    object cumo;
    if(!TO) return;
    if(!ENV(TO)) return;
    if (!random(200)) { // 1/200 esely egy epic lootra
        cumo = new(OBJS+"epic_troll_ring.c");
        if (cumo) {
            cumo->move(TO);   
            write("A Troll Harcos egy %^BOLD%^%^MAGENTA%^ritka gyurut%^RESET%^ dobott!");
        }
    } else {
        cumo = new(OBJS+"epic_troll_buzogany.c");
        if (cumo) {
            cumo->move(TO);   
            write("A Troll Harcos egy %^BOLD%^%^CYAN%^csodalatos buzoganyt%^RESET%^ dobott!");
        }
    }
    ::die(killer);
}
