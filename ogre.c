#include <std.h>
#include <damages.h>
#include "../invazio.h"
inherit MONSTER;

#define lvl 40


void klassz() {
 int szam,i,C1, C2, C3;
 object ob = TO;
 string *EXTRA;
 string *GONOSZOK = ({"barbar","gladiator","kardmuvesz","hirnok","saman","arnyjaro","bard","fejvadasz","utonallo","demonmagus","foldmagus","levegomagus","tuzmagus","harcimagus","illuzionista","nekromanta"});
 string *SEMLEGESEK = ({"barbar","erdojaro","kardmuvesz","druida","saman","bard","fejvadasz","renegat","demonmagus","foldmagus","levegomagus","oselemmagus","tuzmagus","vizmagus","energiamagus","harcimagus","illuzionista","mentalista"});
  
 ob->set_level(lvl+D(1,60));
 ob->set_level2(lvl+D(1,60));
 ob->set_level3(lvl+D(1,60));
 
 //random 2 eselyel lesz gonosz vagy semleges
if(!random(2)) {szam=1;} else {szam=2;}  
  if (szam==1) {
    for(i = 0;i<sizeof(GONOSZOK);i++) {
    C1 = random(sizeof(GONOSZOK[i]));
    C2 = random(sizeof(GONOSZOK[i]));
    C3 = random(sizeof(GONOSZOK[i]));
  }
  ob->set_class(GONOSZOK[C1]);
  ob->set_class2(GONOSZOK[C2]);
  ob->set_class3(GONOSZOK[C3]);
  EXTRA =({"tuzmagus", "arnyjaro", "demonmagus","bard","utonallo","gladiator","kardmuvesz","saman","foldmagus","levegomagus"});
  if (ob->query_level() == 100 || ob->query_level2() == 100 || ob->query_level3() == 100) {
    ob->set_extra_classes(EXTRA);
  }
  ob->set_alignment(-1500);
  }
  else {
  //semleges
      for(i = 0;i<sizeof(SEMLEGESEK);i++) {
    C1 = random(sizeof(SEMLEGESEK[i]));
    C2 = random(sizeof(SEMLEGESEK[i]));
    C3 = random(sizeof(SEMLEGESEK[i]));
  }
  ob->set_class(SEMLEGESEK[C1]);
  ob->set_class2(SEMLEGESEK[C2]);
  ob->set_class3(SEMLEGESEK[C3]);
  EXTRA =({"tuzmagus", "bard", "demonmagus","renegat","fejvadasz",
  "druida","kardmuvesz","saman","foldmagus","levegomagus"});
  if(ob->query_level() == 100 || ob->query_level2() == 100 || ob->query_level3() == 100) {
    ob->set_extra_classes(EXTRA);
  }
  ob->set_alignment(0);
  } 
}



void create() {
  ::create();
  klassz();
  set_name("Ogre");
  set_id( ({ "ogre", "csunyasag"}) );
  set_short("%^BOLD%^%^RED%^Csunya, gonosz%^BOLD%^%^YELLOW%^ Ogre%^RESET%^");
  set_long(  "Egy csunya es gonosz ogre, aki rettegessel tolti el az "
      "embereket a kozelebe keruloik szamara.");
  set_race("ogre");
  set_gender("ferfi"); 

  //set_moving(1);
  //set_speed(5);
  set_alignment(-1500);  
  set_money(25000);
  set_speech(15, ({
    "Elfutni sem fogtok tolem, kicsik!",
    "Az a celom, hogy mindenkit leigazzak es uralkodjak rajtuk!",
    "Azt hittem, hogy az embereknek tobb batorsaguk van, de tevedtem."
     }), 0 );  
 set_emotes(6, ({
    "Ogre mormolja: Bosszuvagy!",
    "Ogre mormolja: Gyulolet!",
  "Ogre kuncogja: Orom a masok szenvedeseben!",
  "Ogre kantalja: Felelemkeltes!"
 }), 0 );
  set_equipment( ({
  //OBJS+"fullank"
  }) );
  if (TO->query_spell("halalaura")>0) {TO->delete_spell("halalaura");}
  
 // add_action("repkedes", "repkedes");
//  add_action("zummoges", "zummoges");
//  add_activities(({"lefegyverez","repkedes","zummoges"}));
}