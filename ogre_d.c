// Idegen invazios esemenyt kezelo daemon whitelight 2025.09.13. (JAVÍTVA - TELJES)
#include <std.h>
#include <daemons.h>
#include <living.h>
#include "../invazio.h"

inherit DAEMON;

// A privat valtozok a daemon allapotanak tarolasara
private int event_running;
private string current_event_id;
private int event_callout_id;
private int event_end_time;
private string stat_file_path;
private int maintain_callout_id;
private object caller_player; // A hivo jatekos elmentese
private string *active_areas; // A tobb terulet tarolasara
private int periodic_shout_id;
private int original_duration;

// Static változók kontrollálása
private static int spawn_depth = 0;

// Az osztalyok es a hozza tartozo engedelyezett jellemek
private mapping class_alignments = ([
  "harcos": ({ JO_ALIG, SEMLEGES_ALIG, GONOSZ_ALIG }),
  "tolvaj": ({ JO_ALIG, SEMLEGES_ALIG, GONOSZ_ALIG }),
  "barbar": ({ SEMLEGES_ALIG, GONOSZ_ALIG }),
  "arnyjaro": ({ GONOSZ_ALIG }),
  "erdojaro": ({ JO_ALIG, SEMLEGES_ALIG }),
  "bard": ({ JO_ALIG, SEMLEGES_ALIG, GONOSZ_ALIG }),
  "gladiator": ({ GONOSZ_ALIG }),
  "fejvadasz": ({ SEMLEGES_ALIG, GONOSZ_ALIG }),
  "kardmuvesz": ({ JO_ALIG, SEMLEGES_ALIG, GONOSZ_ALIG }),
  "renegat": ({ JO_ALIG, SEMLEGES_ALIG }),
  "lovag": ({ JO_ALIG }),
  "utonallo": ({ GONOSZ_ALIG }),
  "pap": ({ JO_ALIG, SEMLEGES_ALIG, GONOSZ_ALIG }),
  "magus": ({ JO_ALIG, SEMLEGES_ALIG, GONOSZ_ALIG }),
  "druida": ({ SEMLEGES_ALIG }),
  "demonmagus": ({ SEMLEGES_ALIG, GONOSZ_ALIG }),
  "hirnok": ({ GONOSZ_ALIG }),
  "elementalista": ({ JO_ALIG, SEMLEGES_ALIG, GONOSZ_ALIG }),
  "paladin": ({ JO_ALIG }),
  "foldmagus": ({ JO_ALIG, SEMLEGES_ALIG, GONOSZ_ALIG }),
  "saman": ({ SEMLEGES_ALIG, GONOSZ_ALIG }),
  "levegomagus": ({ JO_ALIG, SEMLEGES_ALIG, GONOSZ_ALIG }),
  "szerzetes": ({ JO_ALIG }),
  "oselemmagus": ({ JO_ALIG, SEMLEGES_ALIG }),
  "tuzmagus": ({ SEMLEGES_ALIG, GONOSZ_ALIG }),
  "vizmagus": ({ JO_ALIG, SEMLEGES_ALIG }),
  "energiamagus": ({ JO_ALIG, SEMLEGES_ALIG }),
  "harcimagus": ({ JO_ALIG, SEMLEGES_ALIG, GONOSZ_ALIG }),
  "illuzionista": ({ SEMLEGES_ALIG, GONOSZ_ALIG }),
  "mentalista": ({ JO_ALIG, SEMLEGES_ALIG }),
  "nekromanta": ({ GONOSZ_ALIG }),
]);

// A fugggveny prototipusok
int check_invasion_status(object player);
void stop_invasion(object player);
void end_invasion();
void spawn_aliens_async(string area_path, string *rooms_to_spawn, object player);
void remove_aliens_async(object *aliens_to_remove, int index);
void remove_all_aliens(string event_id);
void process_end_of_invasion();
int sort_players(string a, string b, mapping stats);
void maintain_alien_count();
void periodic_shout();
void set_invasion_duration(int new_duration);
int query_total_kills(string player_name);
void drop_rare_item(object alien);
void display_alien_leaderboard(object player);
int query_event_running();
int query_current_event_kills(string player_name);
void start_invasion_multiple_areas(int duration, string *areas);
void add_player_kill(string player_name, string event_id);
void before_update();
void clean_up_static_vars();

void create() {
  ::create();
  // Tisztítás: ha voltak futó callout-ok a korábbi példányból, próbáljuk eltávolítani őket
  clean_up_callouts();
  clean_up_static_vars();
  // alap kezdeti állapot
  event_running = 0;
  current_event_id = 0;
  event_end_time = 0;
}

// Callout tisztítás
void clean_up_callouts() {
  if (maintain_callout_id) { remove_call_out(maintain_callout_id); maintain_callout_id = 0; }
  if (event_callout_id)  { remove_call_out(event_callout_id); event_callout_id  = 0; }
  if (periodic_shout_id) { remove_call_out(periodic_shout_id); periodic_shout_id = 0; }
}

// Static változók nullázása
void clean_up_static_vars() {
  spawn_depth = 0;
}

// Update előtti tisztítás
void before_update() {
  log_file("invaz_debug", sprintf("%s: before_update called\n", ctime(time())));
  clean_up_callouts();
  clean_up_static_vars();
  
  // Állítsuk le a futó eseményt biztonságosan
  if (event_running) {
    event_running = 0;
    current_event_id = 0;
    event_end_time = 0;
    log_file("invaz_debug", sprintf("%s: invasion stopped before update\n", ctime(time())));
  }
}

// Ha a driver meghívja a remove-ot (pl. update előtt), takarítsuk a calloutokat
void remove() {
  before_update();
  ::remove();
}

// A start_invasion_multiple_areas() fugggveny inditja el az invazios esemenyt tobb teruleten.
void start_invasion_multiple_areas(int duration, string *areas) {
    string *formatted_areas;
    string *corrected_areas;
    string *rooms;

    if (event_running) {
        tell_object(TP, "Az Idegen invazio mar folyamatban van.\n");
        return;
    }

    log_file("invaz_debug", sprintf("%s: Starting invasion with duration %d\n", ctime(time()), duration));
    event_running = 1;

    if (!duration) {
        duration = DEFAULT_DURATION;
    }
    
    if (!areas || !sizeof(areas)) {
        areas = DEFAULT_AREAS;
    }
    
    current_event_id = sprintf("Idegen_invazio_%d", time());
    caller_player = TP;
    
    corrected_areas = ({});
    foreach (string area_name_or_path in areas) {
        string final_path;
        
        if (strlen(area_name_or_path) >= strlen("/domains/") && area_name_or_path[0..8] == "/domains/") {
            if (area_name_or_path[<1] != '/') {
                final_path = area_name_or_path + "/";
            } else {
                final_path = area_name_or_path;
            }
        } else {
            final_path = "/domains/" + area_name_or_path + "/rooms/";
        }
        corrected_areas += ({ final_path });
    }
    active_areas = corrected_areas;
    original_duration = duration;

    formatted_areas = ({});
    foreach (string area_path in active_areas) {
        string *parts = explode(area_path, "/");
        if(sizeof(parts) >= 2) {
            formatted_areas += ({ capitalize(parts[1]) });
        }
    }
    
    if (objectp(TP)) tell_object(TP, "Az Idegen invazio elkezdodott a kovetkezo teruleteken:\n " + implode(formatted_areas, ", ") + "\n");
        
    foreach (string area_path in active_areas) {
        if (file_size(area_path) == -2) {
                mixed err = catch(alien->remove());
            if (err) {
                log_file("invaz_errors", sprintf("%s: failed to remove alien: %O\n", ctime(time()), err));
            }
        }
    }
    
    // Ha vannak további elemek, hívd újra késleltetve
    if ((index + chunk_size) < sizeof(aliens_to_remove)) {
        call_out("remove_aliens_async", 3, aliens_to_remove, index + chunk_size);
    } else {
        if (objectp(caller_player)) {
            tell_object(caller_player, "Az osszes Idegen sikeresen el lett tavolitva. Az invazio befejezodott.\n");
        }
    }
}

// A process_end_of_invasion() fugggveny befejezi az invaziot
void process_end_of_invasion() {
    string *files;
    string file;
    string player_name;
    mapping player_stats;
    mapping all_event_stats;
    mixed *leaderboard;
    mapping rewards;
    int i;
    string money_amount_str;
    int money_amount;
    object player_ob;
    object item;
    int kills;
    string current_file_path, old_file_path;
    
    all_event_stats = ([ ]);

    if (file_size(SAVE_DIR + "/old") != -2) {
        mkdir(SAVE_DIR + "/old");
    }

    mixed err = catch(files = get_dir(SAVE_DIR + "/*.o"));
    if (err) {
        log_file("invaz_errors", sprintf("%s: Error reading save directory: %O\n", ctime(time()), err));
        files = ({});
    }
    
    if (files) {
        foreach (file in files) {
            if (!file || strlen(file) < 3) continue;

            if (file == "leaderboard.o") {
                continue;
            }
            
            string raw = read_file(SAVE_DIR + "/" + file);
            if (!raw) continue;
            mapping restored = ([ ]);
            err = catch(restored = restore_variable(raw));
            if (err || !mapp(restored)) {
                log_file("invaz_errors", sprintf("%s: process_end_of_invasion: restore failed for %s\n", ctime(time()), SAVE_DIR + "/" + file));
                continue;
            }

            player_stats = restored;

            if (mapp(player_stats) && player_stats["event_kills"] && mapp(player_stats["event_kills"])) {
                player_name = file[0..<3];
                if (player_stats["event_kills"][current_event_id]) {
                    all_event_stats[player_name] = player_stats["event_kills"][current_event_id];
                }
            }
        }
    }

    // Ranglista rendezés
    leaderboard = sort_array(keys(all_event_stats), "sort_players", this_object(), all_event_stats);

    // Jutalmak kiosztása
    if (objectp(caller_player)) {
        tell_object(caller_player, sprintf("\n%%^BOLD%%^%%^YELLOW%%^--- Idegen Invazio Ranglista ---%%^RESET%%^\n"));
    } else {
        log_file("invaz_errors", sprintf("%s: process_end_of_invasion: caller_player not present\n", ctime(time())));
    }
    
    for (i = 0; i < sizeof(leaderboard) && i < 3; i++) {
        player_name = leaderboard[i];
        kills = all_event_stats[player_name];
        player_ob = find_player(player_name);
        rewards = ([
            0: ({ "10000000", OBJS+"dimenziodoboz.c" }),
            1: ({ "5000000", OBJS+"dimenziodoboz.c" }),
            2: ({ "2500000", OBJS+"dimenziodoboz.c" }),
        ]);

        money_amount_str = rewards[i][0];
        money_amount = to_int(money_amount_str);

        if (objectp(caller_player)) {
            tell_object(caller_player, sprintf("%%^BOLD%%^%%^CYAN%%^%d. helyezett: %s - %d gyilkossaggal!%%^RESET%%^\n", i + 1, capitalize(player_name), kills));
        }

        if (player_ob) {
            tell_object(player_ob, sprintf("\nGratulalok! A %d. helyen vegeztel az Idegen invazioban %d gyilkossaggal. Jutalom: %s arany.\n", i + 1, kills, money_amount_str));
            err = catch(player_ob->add_money(money_amount));
            if (err) {
                log_file("invaz_errors", sprintf("%s: Error adding money to player %s: %O\n", ctime(time()), player_name, err));
            }

            if (rewards[i][1]) {
                err = catch(item = new(rewards[i][1]));
                if (err) {
                    log_file("invaz_errors", sprintf("%s: Error creating reward item %s: %O\n", ctime(time()), rewards[i][1], err));
                } else if (item) {
                    err = catch(item->move(player_ob));
                    if (err) {
                        log_file("invaz_errors", sprintf("%s: Error moving reward to player %s: %O\n", ctime(time()), player_name, err));
                        if (item) item->remove();
                    } else {
                        tell_object(player_ob, "Kaptal egy specialis jutalmat!\n");
                    }
                }
            }
        } else {
            if (objectp(caller_player)) {
                tell_object(caller_player, sprintf("A %s jatekos, aki a %d. helyen vegzett, nincs bejelentkezve.\n", player_name, i + 1));
            }
        }
    }
    
    if (objectp(caller_player)) {
        tell_object(caller_player, sprintf("\n%%^BOLD%%^%%^YELLOW%%^-----------------------------%%^RESET%%^\n"));
    }

    // Fájlok archiválása
    err = catch(files = get_dir(SAVE_DIR + "/*.o"));
    if (err) files = ({});
    
    if(files && sizeof(files)) {
        foreach (file in files) {
            if (!file || strlen(file) < 3) continue;
            if (file == "leaderboard.o") {
                continue;
            }
            
            player_name = file[0..<3];
            current_file_path = SAVE_DIR + "/" + file;
            old_file_path = SAVE_DIR + "/old/" + file;
            
            string current_file_content = read_file(current_file_path);
            if (!current_file_content) continue;
            
            mapping current_stats = ([ ]);
            err = catch(current_stats = restore_variable(current_file_content));
            if (err || !mapp(current_stats)) {
                log_file("invaz_errors", sprintf("%s: process_end_of_invasion: restore failed for current %s\n", ctime(time()), current_file_path));
                continue;
            }
            
            if (file_size(old_file_path)) {
                string old_file_content = read_file(old_file_path);
                mapping old_stats = ([ ]);
                if (old_file_content) {
                    err = catch(old_stats = restore_variable(old_file_content));
                    if (err || !mapp(old_stats)) {
                        old_stats = ([ ]);
                    }
                }
                
                if (current_stats && mapp(current_stats) && current_stats["total_kills"]) {
                    if (!old_stats["total_kills"]) {
                        old_stats["total_kills"] = 0;
                    }
                    old_stats["total_kills"] += current_stats["total_kills"];
                }
                write_file(old_file_path, save_variable(old_stats), 1);
            } else {
                int ren = rename(current_file_path, old_file_path);
                if (!ren) {
                    write_file(old_file_path, save_variable(current_stats), 1);
                }
            }
        }
    }
}

// A ranglista megjelenítése
void display_alien_leaderboard(object player) {
    mapping all_time_leaderboard;
    mixed *sorted_players;
    string player_name;
    int kills;
    int i;
    string *files;
    string file;
    string stats_file_path;
    mapping stats;

    all_time_leaderboard = ([ ]);

    if (file_size(SAVE_DIR + "/old") != -2) {
        tell_object(player, "A ranglista jelenleg nem elerheto, mivel meg nem fejezodott be egy invazio sem.\n");
        return;
    }

    mixed err = catch(files = get_dir(SAVE_DIR + "/old/" + "*.o"));
    if (err || !files || !sizeof(files)) {
        tell_object(player, "A ranglista jelenleg nem elerheto, mivel meg nem fejezodott be egy invazio sem.\n");
        return;
    }

    foreach(file in files) {
        player_name = file[0..<3];
        stats_file_path = SAVE_DIR + "/old/" + file;
        
        string file_content = read_file(stats_file_path);
        if (!file_content) continue;
        mapping restored = ([ ]);
        err = catch(restored = restore_variable(file_content));
        if (err || !mapp(restored)) continue;
        stats = restored;
        
        if (mapp(stats) && stats["total_kills"]) {
            all_time_leaderboard[player_name] = stats["total_kills"];
        }
    }

    if (!sizeof(all_time_leaderboard)) {
        tell_object(player, "A ranglista meg ures.\n");
        return;
    }
    
    sorted_players = sort_array(keys(all_time_leaderboard), "sort_players", this_object(), all_time_leaderboard);

    tell_object(player, sprintf("\n%%^BOLD%%^%%^YELLOW%%^--- Osszesitett Ranglista (Top 5) ---%%^RESET%%^\n"));

    for (i = 0; i < sizeof(sorted_players) && i < 5; i++) {
        player_name = sorted_players[i];
        kills = all_time_leaderboard[player_name];
        tell_object(player, sprintf("%%^BOLD%%^%%^CYAN%%^%d. helyezett: %s - %d gyilkossaggal!%%^RESET%%^\n", i + 1, capitalize(player_name), kills));
    }
    tell_object(player, "--------------------------\n");
}

// Segédfüggvény a rendezéshez
int sort_players(string a, string b, mapping stats) {
    return stats[b] - stats[a];
}

void add_player_kill(string player_name, string event_id) {
    mapping stats;
    if (event_id != current_event_id) {
        return;
    }
    if (file_size(SAVE_DIR) != -2) {
        mkdir(SAVE_DIR);
    }
    stat_file_path = SAVE_DIR + "/" + lower_case(player_name) + ".o";
    
    mixed err = catch({
        if (file_exists(stat_file_path)) {
            string raw = read_file(stat_file_path);
            if (raw) {
                err = catch(stats = restore_variable(raw));
                if (err || !mapp(stats)) {
                    stats = ([ ]);
                }
            } else {
                stats = ([ ]);
            }
        } else {
            stats = ([ ]);
        }
    });
    
    if (err) {
        log_file("invaz_errors", sprintf("%s: Error in add_player_kill for %s: %O\n", ctime(time()), player_name, err));
        stats = ([ ]);
    }

    if (!stats || !mapp(stats)) {
        stats = ([ ]);
    }

    if (!stats["total_kills"]) {
        stats["total_kills"] = 0;
    }
    stats["total_kills"]++;

    if (!stats["event_kills"] || !mapp(stats["event_kills"])) {
        stats["event_kills"] = ([ ]);
    }
    if (!stats["event_kills"][current_event_id]) {
        stats["event_kills"][current_event_id] = 0;
    }
    stats["event_kills"][current_event_id]++;
    
    err = catch(write_file(stat_file_path, save_variable(stats), 1));
    if (err) {
        log_file("invaz_errors", sprintf("%s: Error writing stats for %s: %O\n", ctime(time()), player_name, err));
    }
}

// Periodikus shout-ok kezelése
void periodic_shout() {
    return; // Kikapcsolva
}

// Az invázió időtartamának beállítása
void set_invasion_duration(int new_duration) {
    int minutes;
    int seconds;

    if (!wizardp(TP)) {
        tell_object(TP, "Nincs meg a megfelelo jogod ehhez a parancshoz.\n");
        return;
    }

    if (!event_running) {
        tell_object(TP, "Az Idegen invazio jelenleg nem fut, igy az idotartama nem allithato.\n");
        return;
    }

    if (new_duration <= 0) {
        tell_object(TP, "Az uj idotartamnak nagyobbnak kell lennie nullanal. Az invazio leallitasahoz hasznald a stop_invasion() fugvenyt.\n");
        return;
    }

    // Calloutok törlése
    if (event_callout_id) {
        remove_call_out(event_callout_id);
        event_callout_id = 0;
    }
    if (periodic_shout_id) {
        remove_call_out(periodic_shout_id);
        periodic_shout_id = 0;
    }

    // Új időtartam beállítása
    event_end_time = time() + new_duration;
    original_duration = new_duration;

    // Új calloutok beállítása
    event_callout_id = call_out("end_invasion", new_duration);

    minutes = new_duration / 60;
    seconds = new_duration % 60;

    tell_object(TP, sprintf("Az invazio idotartama sikeresen beallitva %d perc %d masodpercre.\n", minutes, seconds));
}

// Total kills lekérdezése
int query_total_kills(string player_name) {
    mapping stats;
    string stat_file_path;

    if (!player_name || player_name == "") {
        return 0;
    }

    stat_file_path = SAVE_DIR + "/old/" + lower_case(player_name) + ".o";

    if (file_exists(stat_file_path)) {
        string raw = read_file(stat_file_path);
        if (!raw) return 0;
        mixed err = catch(stats = restore_variable(raw));
        if (err || !mapp(stats)) return 0;
        if (stats && stats["total_kills"]) {
            return stats["total_kills"];
        }
    }
    return 0;
}

// Jelenlegi esemény gyilkosságainak lekérdezése
int query_current_event_kills(string player_name) {
    mapping stats;
    string stat_file_path;
    
    if (!player_name || player_name == "" || !current_event_id) {
        return 0;
    }

    stat_file_path = SAVE_DIR + "/" + lower_case(player_name) + ".o";
    
    if (file_exists(stat_file_path)) {
        string raw = read_file(stat_file_path);
        if (!raw) return 0;
        mixed err = catch(stats = restore_variable(raw));
        if (err || !mapp(stats)) return 0;
        if (stats && stats["event_kills"] && stats["event_kills"][current_event_id]) {
            return stats["event_kills"][current_event_id];
        }
    }
    return 0;
}

// Invázió futásának ellenőrzése
int query_event_running() {
    return event_running;
}rooms = get_dir(area_path + "*.c"));
            if (err) {
                log_file("invaz_errors", sprintf("%s: Error reading directory %s: %O\n", ctime(time()), area_path, err));
                continue;
            }
            if (sizeof(rooms) > 0) {
                spawn_aliens_async(area_path, rooms, TP);
            }
        } else {
            if (objectp(TP)) tell_object(TP, sprintf("Hiba: %s nem egy ervenyes konyvtar, kihagyva.\n", area_path));
        }
    }

    // Fenntartó callout indítása (ellenőrzi aktív idegenek számát)
    maintain_callout_id = call_out("maintain_alien_count", 15); // 15 másodperc
    event_callout_id = call_out("end_invasion", duration);
    event_end_time = time() + duration;
}

// JAVÍTOTT - Az új függvény a folyamatos Idegen spawnolásért
void maintain_alien_count() {
    int active_aliens = 0;
    string room_path;
    string *rooms;
    string area_to_spawn;
    int aliens_to_spawn;
    int max_iterations = 10; // BIZTONSÁGI KORLÁT
    int current_iteration = 0;

    // Guardok
    if (!this_object() || !event_running) {
        if (maintain_callout_id) {
            remove_call_out(maintain_callout_id);
            maintain_callout_id = 0;
        }
        return;
    }

    // Aktív idegenek számolása - hibakezeléssel
    mixed err = catch({
        foreach(object ob in livings()) {
            if (ob && living(ob) && !userp(ob) && ob->query_property("event_id") == current_event_id) {
                active_aliens++;
            }
        }
    });
    
    if (err) {
        log_file("invaz_errors", sprintf("%s: Error counting active aliens: %O\n", ctime(time()), err));
        active_aliens = MIN_ACTIVE_ALIENS; // Biztonságos érték
    }

    if (active_aliens < MIN_ACTIVE_ALIENS) {
        aliens_to_spawn = MIN_ACTIVE_ALIENS - active_aliens;
        
        // JAVÍTOTT CIKLUS - végtelen ciklus megelőzése
        while (aliens_to_spawn > 0 && sizeof(active_areas) > 0 && current_iteration < max_iterations) {
            current_iteration++;
            
            area_to_spawn = active_areas[random(sizeof(active_areas))];
            
            // File művelet hibakezeléssel
            err = catch({
                if (file_size(area_to_spawn) == -2) {
                    rooms = get_dir(area_to_spawn + "*.c");
                } else {
                    rooms = ({});
                }
            });
            
            if (err) {
                log_file("invaz_errors", sprintf("%s: Error reading directory in maintain: %s: %O\n", ctime(time()), area_to_spawn, err));
                rooms = ({});
            }
            
            if (sizeof(rooms) > 0) {
                spawn_aliens_async(area_to_spawn, rooms, caller_player);
                aliens_to_spawn = 0; // Kilép
            } else {
                aliens_to_spawn--;
                // Ha minden area üres, ne próbálkozzunk tovább
                if (current_iteration >= sizeof(active_areas)) {
                    break;
                }
            }
        }
        
        // Ha elértük a maximum iterációt, logoljuk
        if (current_iteration >= max_iterations) {
            log_file("invaz_errors", sprintf("%s: maintain_alien_count: maximum iterations reached\n", ctime(time())));
        }
    }

    // Újrahívás csak akkor, ha még mindig fut az esemény
    if (event_running && this_object()) {
        maintain_callout_id = call_out("maintain_alien_count", 20); // 20 másodperc
    } else {
        maintain_callout_id = 0;
    }
}

// JAVÍTOTT spawn_aliens_async - hibakezeléssel és korlátokkal
void spawn_aliens_async(string area_path, string *rooms_to_spawn, object player) {
    string room_file;
    string room_path;
    object room;
    int num_aliens;
    int i;
    object alien;
    int rooms_processed = 0;
    int aliens_processed = 0;
    int alien_level;
    int roll;
    string color_code;
    string *all_classes;
    string selected_class;
    string *secondary_classes;
    string *tertiary_classes;
    string original_short;
    string *strong_races = ({"kekangyal", "zoldangyal", "celervis", "ianus", "osianus"});
    
    int *allowed_alignments;
    int random_alignment;
    
    // KRITIKUS ELLENŐRZÉS - spawn depth kontroll
    spawn_depth++;
    
    if (spawn_depth > 30) { // Maximum 30 egyidejű spawn callout
        spawn_depth--;
        log_file("invaz_errors", sprintf("%s: spawn_aliens_async: too many concurrent spawns (%d), aborting\n", ctime(time()), spawn_depth));
        return;
    }

    // Guardok
    if (!this_object() || !event_running || !current_event_id) {
        spawn_depth--;
        return;
    }

    if (!sizeof(rooms_to_spawn)) {
        spawn_depth--;
        if (objectp(player)) tell_object(player, sprintf("A szornyek spawnolasa a(z) %s teruleten befejezodott.\n", area_path));
        return;
    }

    // CSÖKKENTETT CHUNK_SIZE a lag csökkentése érdekében
    int reduced_chunk_size = 20; // 50 helyett 20
    
    while (rooms_processed < reduced_chunk_size && sizeof(rooms_to_spawn)) {
        room_file = rooms_to_spawn[0];
        rooms_to_spawn = rooms_to_spawn[1..];
        rooms_processed++;

        room_path = area_path + room_file;

        // Ellenőrizze, hogy a fájl létezik-e
        if (file_size(room_path) < 0) {
            continue;
        }

        if (!find_object(room_path)) {
            mixed err = catch(load_object(room_path));
            if (err) {
                log_file("invaz_errors", sprintf("%s: failed to load room %s: %O\n", ctime(time()), room_path, err));
                continue;
            }
        }
        
        room = find_object(room_path);

        if (room && !room->query_property("no attack", 1)) {
            // CSÖKKENTETT alien szám
            num_aliens = random(3) + 1; // Maximum 3

            for(i = 0; i < num_aliens; i++) {
                // Guardok minden iterációban
                if (!this_object() || !event_running || !current_event_id) {
                    spawn_depth--;
                    return;
                }

                // Monster létrehozás hibakezeléssel
                string monster_dir = MONS;
                string *monster_files = ({});
                
                mixed err = catch(monster_files = get_dir(monster_dir + "*.c"));
                if (err || !sizeof(monster_files)) {
                    err = catch(alien = new(ALIEN_NPC));
                    if (err) {
                        log_file("invaz_errors", sprintf("%s: failed to create default alien: %O\n", ctime(time()), err));
                        continue;
                    }
                } else {
                    string monster_to_create = MONS + monster_files[random(sizeof(monster_files))];
                    if (file_size(monster_to_create) >= 0) {
                        err = catch(alien = new(monster_to_create));
                        if (err) {
                            err = catch(alien = new(ALIEN_NPC));
                            if (err) {
                                log_file("invaz_errors", sprintf("%s: failed to create any alien: %O\n", ctime(time()), err));
                                continue;
                            }
                        }
                    } else {
                        err = catch(alien = new(ALIEN_NPC));
                        if (err) {
                            continue;
                        }
                    }
                }

                if (alien) {
                    // Alien beállítások hibakezeléssel
                    mixed config_err = catch({
                        roll = random(100) + 1;

                        if (roll >= 95) {
                            alien_level = 100;
                        } else if (roll >= 80) {
                            alien_level = random(20) + 80;
                        } else {
                            alien_level = random(40) + 40;
                        }

                        alien->set_level(alien_level);
                        alien->set_hp(alien_level * 30);
                        alien->set_max_hp(alien_level * 30);

                        all_classes = keys(class_alignments);
                        if (sizeof(all_classes)) {
                            selected_class = all_classes[random(sizeof(all_classes))];
                            alien->set_class(selected_class);
                        }

                        if (class_alignments[selected_class]) {
                            allowed_alignments = class_alignments[selected_class];
                            random_alignment = allowed_alignments[random(sizeof(allowed_alignments))];
                            alien->set_alignment(random_alignment);
                        } else {
                            alien->set_alignment(GONOSZ_ALIG);
                        }

                        // Speciális képességek
                        if (alien_level == 100) {
                            alien->set_race("phomanor");
                            alien->add_skill("kritikus utes");
                            alien->set_skill("kritikus utes", 6);
                            alien->add_skill("piszkos harc");
                            alien->set_skill("piszkos harc", 6);
                            alien->add_skill("harcok mestere");
                            alien->set_skill("harcok mestere", 6);
                            alien->add_skill("nehezfegyverzet");
                            alien->set_skill("nehezfegyverzet", 6);
                            alien->add_magic_affected("gyemantbor", -1);
                            alien->add_magic_affected("szelpajzs", -1);
                            alien->add_magic_affected("legtest", -1);
                            alien->add_magic_affected("tukorpancel", -1);
                            alien->add_magic_affected("stabilizalas", -1);
                            alien->add_magic_affected("fegyverrogzites", -1);
                            alien->add_magic_affected("gyokerbilincs", -1);
                            alien->add_bonus("talalatbonusz", 50);
                            alien->add_bonus("sebzesbonusz", 50);
                            alien->add_bonus("magiaimmunitas", 20);
                            alien->set_property("adhat_warrunat", 95);
                            alien->set_added_stats((["ero":3,"ugyesseg":3,"allokepesseg":3,"intelligencia":3,"karizma":3,"bolcsesseg":3]));
                            color_code = "%^BOLD%^%^BLACK%^";
                        } else if (alien_level >= 95) {
                            alien->set_race("kylvhieud");
                            color_code = "%^BOLD%^%^RED%^";
                        } else if (alien_level >= 80) {
                            alien->set_race(strong_races[random(sizeof(strong_races))]);
                            alien->add_skill("kritikus utes");
                            alien->set_skill("kritikus utes", 5);
                            color_code = "%^BOLD%^%^GREEN%^";
                        } else if (alien_level >= 60) {
                            color_code = "%^BOLD%^%^YELLOW%^";
                        } else {
                            color_code = "%^BOLD%^%^WHITE%^";
                        }
                        
                        original_short = alien->query_short();
                        if (!original_short) {
                            original_short = "Idegen Harcos";
                        }
                        alien->set_short(color_code + original_short + "%^RESET%^");
                        
                        if (alien_level >= 50) {
                            secondary_classes = keys(class_alignments);
                            if (sizeof(secondary_classes)) alien->set_class2(secondary_classes[random(sizeof(secondary_classes))]);
                            if (alien_level >= 75) {
                                tertiary_classes = keys(class_alignments);
                                if (sizeof(tertiary_classes)) alien->set_class3(tertiary_classes[random(sizeof(tertiary_classes))]);
                            }
                        }
                        
                        alien->set_property("event_id", current_event_id);
                        alien->set_property("spawn_time", time());
                        alien->set_property("spawn_room", room_path);
                    });
                    
                    if (config_err) {
                        log_file("invaz_errors", sprintf("%s: failed to configure alien: %O\n", ctime(time()), config_err));
                        if (alien) alien->remove();
                        continue;
                    }
                    
                    // Mozgatás hibakezeléssel
                    mixed move_err = catch(alien->move(room));
                    if (move_err) {
                        log_file("invaz_errors", sprintf("%s: failed to move alien to room %s: %O\n", ctime(time()), room_path, move_err));
                        if (alien) alien->remove();
                        continue;
                    }

                    aliens_processed++;
                    if (aliens_processed >= reduced_chunk_size) {
                        if (sizeof(rooms_to_spawn) > 0) {
                            call_out("spawn_aliens_async", 3, area_path, rooms_to_spawn, player); // 3 másodperc
                        }
                        spawn_depth--;
                        return;
                    }
                }
            }
        }
    }

    if (sizeof(rooms_to_spawn) > 0) {
        call_out("spawn_aliens_async", 3, area_path, rooms_to_spawn, player); // 3 másodperc késleltetés
    } else {
        if (objectp(player)) tell_object(player, sprintf("A szornyek spawnolasa a(z) %s teruleten befejezodott.\n", area_path));
    }
    
    spawn_depth--;
}

// A check_invasion_status() fugggveny kiirja az aktualis invazio allapotat.
int check_invasion_status(object player) {
    int remaining_time;
    int minutes;
    int seconds;
    string area_path;
    string *files;
    string file;
    string room_path;
    object room;
    int count;
    object item;  
    string *room_status;
    string line;
    int i;
    int num_rooms;
    int current_line_length;
    string room_name;
    string formatted_entry;

    room_status = ({ });

    if (!event_running) {
        tell_object(player, "Az Idegen invazio jelenleg nem fut.\n");
        return 1;
    }

    tell_object(player, "Idegen invazio allapota:\n");
    tell_object(player, "-----------------------\n");

    remaining_time = event_end_time - time();
    if (remaining_time > 0) {
        minutes = remaining_time / 60;
        seconds = remaining_time % 60;
        tell_object(player, sprintf("Hatralevo ido az invazio vegeig: %d perc, %d masodperc.\n\n", minutes, seconds));
    } else {
        tell_object(player, "Az invazio hamarosan veget er.\n\n");
    }

    if (wizardp(player))  {
        tell_object(player, "Szoba eleresi utja:\n" + implode(active_areas, ", ") + "\n\n");
    
        foreach (area_path in active_areas) {
            mixed err = catch(files = get_dir(area_path + "*.c"));
            if (err || !files || !sizeof(files)) {
                continue;
            }

            foreach (file in files) {
                room_path = area_path + file;
                room = find_object(room_path);
                count = 0;

                if (room) {
                    err = catch({
                        foreach (item in all_inventory(room)) {
                            if (item && living(item) && !userp(item) && item->query_property("event_id") == current_event_id) {
                                count++;
                            }
                        }
                    });
                    if (err) count = 0;
                    
                    if (count > 0) {
                        if (!file || strlen(file) < 3) continue;
                        room_name = file[0..<3];
                        formatted_entry = sprintf("%%^BOLD%%^%%^YELLOW%%^%s %%^RESET%%^%%^BOLD%%^%%^RED%%^[%d]%%^RESET%%^ ", room_name, count);
                        room_status += ({ formatted_entry });
                    }
                }
            }
        }

        num_rooms = sizeof(room_status);
        current_line_length = 0;
        line = "";

        if (num_rooms > 0) {
            for (i = 0; i < num_rooms; i++) {
                int display_length = strlen(room_status[i]) - (strlen("%%^BOLD%%^%%^YELLOW%%^") + strlen("%%^RESET%%^%%^BOLD%%^%%^RED%%^") + strlen("%%^RESET%^"));
                if (current_line_length + display_length + (i > 0 ? 3 : 0) > 75) {
                    tell_object(player, line + "\n");
                    line = "";
                    current_line_length = 0;
                }
                if (current_line_length > 0) {
                    line += " | ";
                    current_line_length += 3;
                }
                line += room_status[i];
                current_line_length += display_length;
            }
            if (strlen(line) > 0) {
                tell_object(player, line + "\n");
            }
        } else {
            tell_object(player, "Nincsenek aktiv Idegenek a teruleten.\n");
        }
    }
    tell_object(player, "-----------------------\n");
    return 1;
}

// A stop_invasion() fugggveny a kezi leallitast vegzi.
void stop_invasion(object player) {
    if (!wizardp(player)) {
        tell_object(player, "Nincs meg a megfelelo jogod ehhez a parancshoz.\n");
        return;
    }
    if (!event_running) {
        tell_object(player, "Az Idegen invazio jelenleg nem fut.\n");
        return;
    }
    
    if (objectp(TP)) tell_object(TP, "Az Idegen invazio manualisan leallitva.\n");
    if (event_callout_id) {
        remove_call_out(event_callout_id);
        event_callout_id = 0;
    }
    end_invasion();
}

// Az end_invasion() fugggveny tavolitja el az Idegeneket az esemeny vegen.
void end_invasion() {
    string event_to_clean_up;
    object *aliens;

    if (!event_running) {
        return;
    }
    
    log_file("invaz_debug", sprintf("%s: end_invasion called\n", ctime(time())));
    event_to_clean_up = current_event_id;
    
    // A process_end_of_invasion() hívása, mielőtt az Idegenek eltávolításra kerülnek
    process_end_of_invasion();

    // Szerezzük meg az összes Idegent
    aliens = ({});
    mixed err = catch({
        foreach (object ob in livings()) {
            if (ob && living(ob) && !userp(ob) && ob->query_property("event_id") == event_to_clean_up) {
                aliens += ({ ob });
            }
        }
    });
    
    if (err) {
        log_file("invaz_errors", sprintf("%s: Error collecting aliens for removal: %O\n", ctime(time()), err));
        aliens = ({});
    }

    // Indítsuk el az aszinkron törlést
    if (sizeof(aliens) > 0) {
        remove_aliens_async(aliens, 0);
    }

    clean_up_callouts();
    current_event_id = 0;
    event_running = 0;
    event_end_time = 0;
}

// JAVÍTOTT remove_aliens_async - hibakezeléssel
void remove_aliens_async(object *aliens_to_remove, int index) {
    int i;
    int chunk_size = 15; // Csökkentett chunk size
    
    if (!this_object()) return;
    if (!aliens_to_remove || index >= sizeof(aliens_to_remove)) {
        if (objectp(caller_player)) {
            tell_object(caller_player, "Az osszes Idegen sikeresen el lett tavolitva. Az invazio befejezodott.\n");
        }
        return;
    }

    for (i = 0; i < chunk_size && (index + i) < sizeof(aliens_to_remove); i++) {
        object alien = aliens_to_remove[index + i];
        if (alien && environment(alien)) {
            mixed err = catch(alien->remove());
            if (err) {
                log_file("invaz_errors", sprintf("%s: failed to remove alien: %O\n", ctime(time()), err));
            }
        }
    }
    
    // Ha vannak további elemek, hívd újra késleltetve
    if ((index + chunk_size) < sizeof(aliens_to_remove)) {
        call_out("remove_aliens_async", 3, aliens_to_remove, index + chunk_size);
    } else {
        if (objectp(caller_player)) {
            tell_object(caller_player, "Az osszes Idegen sikeresen el lett tavolitva. Az invazio befejezodott.\n");
        }
    }
}

// A process_end_of_invasion() fugggveny befejezi az invaziot
void process_end_of_invasion() {
    string *files;
    string file;
    string player_name;
    mapping player_stats;
    mapping all_event_stats;
    mixed *leaderboard;
    mapping rewards;
    int i;
    string money_amount_str;
    int money_amount;
    object player_ob;
    object item;
    int kills;
    string current_file_path, old_file_path;
    
    all_event_stats = ([ ]);

    if (file_size(SAVE_DIR + "/old") != -2) {
        mkdir(SAVE_DIR + "/old");
    }

    mixed err = catch(files = get_dir(SAVE_DIR + "/*.o"));
    if (err) {
        log_file("invaz_errors", sprintf("%s: Error reading save directory: %O\n", ctime(time()), err));
        files = ({});
    }
    
    if (files) {
        foreach (file in files) {
            if (!file || strlen(file) < 3) continue;

            if (file == "leaderboard.o") {
                continue;
            }
            
            string raw = read_file(SAVE_DIR + "/" + file);
            if (!raw) continue;
            mapping restored = ([ ]);
            err = catch(restored = restore_variable(raw));
            if (err || !mapp(restored)) {
                log_file("invaz_errors", sprintf("%s: process_end_of_invasion: restore failed for %s\n", ctime(time()), SAVE_DIR + "/" + file));
                continue;
            }

            player_stats = restored;

            if (mapp(player_stats) && player_stats["event_kills"] && mapp(player_stats["event_kills"])) {
                player_name = file[0..<3];
                if (player_stats["event_kills"][current_event_id]) {
                    all_event_stats[player_name] = player_stats["event_kills"][current_event_id];
                }
            }
        }
    }

    // Ranglista rendezés
    leaderboard = sort_array(keys(all_event_stats), "sort_players", this_object(), all_event_stats);

    // Jutalmak kiosztása
    if (objectp(caller_player)) {
        tell_object(caller_player, sprintf("\n%%^BOLD%%^%%^YELLOW%%^--- Idegen Invazio Ranglista ---%%^RESET%%^\n"));
    } else {
        log_file("invaz_errors", sprintf("%s: process_end_of_invasion: caller_player not present\n", ctime(time())));
    }
    
    for (i = 0; i < sizeof(leaderboard) && i < 3; i++) {
        player_name = leaderboard[i];
        kills = all_event_stats[player_name];
        player_ob = find_player(player_name);
        rewards = ([
            0: ({ "10000000", OBJS+"dimenziodoboz.c" }),
            1: ({ "5000000", OBJS+"dimenziodoboz.c" }),
            2: ({ "2500000", OBJS+"dimenziodoboz.c" }),
        ]);

        money_amount_str = rewards[i][0];
        money_amount = to_int(money_amount_str);

        if (objectp(caller_player)) {
            tell_object(caller_player, sprintf("%%^BOLD%%^%%^CYAN%%^%d. helyezett: %s - %d gyilkossaggal!%%^RESET%%^\n", i + 1, capitalize(player_name), kills));
        }

        if (player_ob) {
            tell_object(player_ob, sprintf("\nGratulalok! A %d. helyen vegeztel az Idegen invazioban %d gyilkossaggal. Jutalom: %s arany.\n", i + 1, kills, money_amount_str));
            err = catch(player_ob->add_money(money_amount));
            if (err) {
                log_file("invaz_errors", sprintf("%s: Error adding money to player %s: %O\n", ctime(time()), player_name, err));
            }

            if (rewards[i][1]) {
                err = catch(item = new(rewards[i][1]));
                if (err) {
                    log_file("invaz_errors", sprintf("%s: Error creating reward item %s: %O\n", ctime(time()), rewards[i][1], err));
                } else if (item) {
                    err = catch(item->move(player_ob));
                    if (err) {
                        log_file("invaz_errors", sprintf("%s: Error moving reward to player %s: %O\n", ctime(time()), player_name, err));
                        if (item) item->remove();
                    } else {
                        tell_object(player_ob, "Kaptal egy specialis jutalmat!\n");
                    }
                }
            }
        } else {
            if (objectp(caller_player)) {
                tell_object(caller_player, sprintf("A %s jatekos, aki a %d. helyen vegzett, nincs bejelentkezve.\n", player_name, i + 1));
            }
        }
    }
    
    if (objectp(caller_player)) {
        tell_object(caller_player, sprintf("\n%%^BOLD%%^%%^YELLOW%%^-----------------------------%%^RESET%%^\n"));
    }

    // Fájlok archiválása
    err = catch(files = get_dir(SAVE_DIR + "/*.o"));
    if (err) files = ({});
    
    if(files && sizeof(files)) {
        foreach (file in files) {
            if (!file || strlen(file) < 3) continue;
            if (file == "leaderboard.o") {
                continue;
            }
            
            player_name = file[0..<3];
            current_file_path = SAVE_DIR + "/" + file;
            old_file_path = SAVE_DIR + "/old/" + file;
            
            string current_file_content = read_file(current_file_path);
            if (!current_file_content) continue;
            
            mapping current_stats = ([ ]);
            err = catch(current_stats = restore_variable(current_file_content));
            if (err || !mapp(current_stats)) {
                log_file("invaz_errors", sprintf("%s: process_end_of_invasion: restore failed for current %s\n", ctime(time()), current_file_path));
                continue;
            }
            
            if (file_size(old_file_path)) {
                string old_file_content = read_file(old_file_path);
                mapping old_stats = ([ ]);
                if (old_file_content) {
                    err = catch(old_stats = restore_variable(old_file_content));
                    if (err || !mapp(old_stats)) {
                        old_stats = ([ ]);
                    }
                }
                
                if (current_stats && mapp(current_stats) && current_stats["total_kills"]) {
                    if (!old_stats["total_kills"]) {
                        old_stats["total_kills"] = 0;
                    }
                    old_stats["total_kills"] += current_stats["total_kills"];
                }
                write_file(old_file_path, save_variable(old_stats), 1);
            } else {
                int ren = rename(current_file_path, old_file_path);
                if (!ren) {
                    write_file(old_file_path, save_variable(current_stats), 1);
                }
            }
        }
    }
}

// A ranglista megjelenítése
void display_alien_leaderboard(object player) {
    mapping all_time_leaderboard;
    mixed *sorted_players;
    string player_name;
    int kills;
    int i;
    string *files;
    string file;
    string stats_file_path;
    mapping stats;

    all_time_leaderboard = ([ ]);

    if (file_size(SAVE_DIR + "/old") != -2) {
        tell_object(player, "A ranglista jelenleg nem elerheto, mivel meg nem fejezodott be egy invazio sem.\n");
        return;
    }

    mixed err = catch(files = get_dir(SAVE_DIR + "/old/" + "*.o"));
    if (err || !files || !sizeof(files)) {
        tell_object(player, "A ranglista jelenleg nem elerheto, mivel meg nem fejezodott be egy invazio sem.\n");
        return;
    }

    foreach(file in files) {
        player_name = file[0..<3];
        stats_file_path = SAVE_DIR + "/old/" + file;
        
        string file_content = read_file(stats_file_path);
        if (!file_content) continue;
        mapping restored = ([ ]);
        err = catch(restored = restore_variable(file_content));
        if (err || !mapp(restored)) continue;
        stats = restored;
        
        if (mapp(stats) && stats["total_kills"]) {
            all_time_leaderboard[player_name] = stats["total_kills"];
        }
    }

    if (!sizeof(all_time_leaderboard)) {
        tell_object(player, "A ranglista meg ures.\n");
        return;
    }
    
    sorted_players = sort_array(keys(all_time_leaderboard), "sort_players", this_object(), all_time_leaderboard);

    tell_object(player, sprintf("\n%%^BOLD%%^%%^YELLOW%%^--- Osszesitett Ranglista (Top 5) ---%%^RESET%%^\n"));

    for (i = 0; i < sizeof(sorted_players) && i < 5; i++) {
        player_name = sorted_players[i];
        kills = all_time_leaderboard[player_name];
        tell_object(player, sprintf("%%^BOLD%%^%%^CYAN%%^%d. helyezett: %s - %d gyilkossaggal!%%^RESET%%^\n", i + 1, capitalize(player_name), kills));
    }
    tell_object(player, "--------------------------\n");
}

// Segédfüggvény a rendezéshez
int sort_players(string a, string b, mapping stats) {
    return stats[b] - stats[a];
}

void add_player_kill(string player_name, string event_id) {
    mapping stats;
    if (event_id != current_event_id) {
        return;
    }
    if (file_size(SAVE_DIR) != -2) {
        mkdir(SAVE_DIR);
    }
    stat_file_path = SAVE_DIR + "/" + lower_case(player_name) + ".o";
    
    mixed err = catch({
        if (file_exists(stat_file_path)) {
            string raw = read_file(stat_file_path);
            if (raw) {
                err = catch(stats = restore_variable(raw));
                if (err || !mapp(stats)) {
                    stats = ([ ]);
                }
            } else {
                stats = ([ ]);
            }
        } else {
            stats = ([ ]);
        }
    });
    
    if (err) {
        log_file("invaz_errors", sprintf("%s: Error in add_player_kill for %s: %O\n", ctime(time()), player_name, err));
        stats = ([ ]);
    }

    if (!stats || !mapp(stats)) {
        stats = ([ ]);
    }

    if (!stats["total_kills"]) {
        stats["total_kills"] = 0;
    }
    stats["total_kills"]++;

    if (!stats["event_kills"] || !mapp(stats["event_kills"])) {
        stats["event_kills"] = ([ ]);
    }
    if (!stats["event_kills"][current_event_id]) {
        stats["event_kills"][current_event_id] = 0;
    }
    stats["event_kills"][current_event_id]++;
    
    err = catch(write_file(stat_file_path, save_variable(stats), 1));
    if (err) {
        log_file("invaz_errors", sprintf("%s: Error writing stats for %s: %O\n", ctime(time()), player_name, err));
    }
}

// Periodikus shout-ok kezelése
void periodic_shout() {
    return; // Kikapcsolva
}

// Az invázió időtartamának beállítása
void set_invasion_duration(int new_duration) {
    int minutes;
    int seconds;

    if (!wizardp(TP)) {
        tell_object(TP, "Nincs meg a megfelelo jogod ehhez a parancshoz.\n");
        return;
    }

    if (!event_running) {
        tell_object(TP, "Az Idegen invazio jelenleg nem fut, igy az idotartama nem allithato.\n");
        return;
    }

    if (new_duration <= 0) {
        tell_object(TP, "Az uj idotartamnak nagyobbnak kell lennie nullanal. Az invazio leallitasahoz hasznald a stop_invasion() fugvenyt.\n");
        return;
    }

    // Calloutok törlése
    if (event_callout_id) {
        remove_call_out(event_callout_id);
        event_callout_id = 0;
    }
    if (periodic_shout_id) {
        remove_call_out(periodic_shout_id);
        periodic_shout_id = 0;
    }

    // Új időtartam beállítása
    event_end_time = time() + new_duration;
    original_duration = new_duration;

    // Új calloutok beállítása
    event_callout_id = call_out("end_invasion", new_duration);

    minutes = new_duration / 60;
    seconds = new_duration % 60;

    tell_object(TP, sprintf("Az invazio idotartama sikeresen beallitva %d perc %d masodpercre.\n", minutes, seconds));
}

// Total kills lekérdezése
int query_total_kills(string player_name) {
    mapping stats;
    string stat_file_path;

    if (!player_name || player_name == "") {
        return 0;
    }

    stat_file_path = SAVE_DIR + "/old/" + lower_case(player_name) + ".o";

    if (file_exists(stat_file_path)) {
        string raw = read_file(stat_file_path);
        if (!raw) return 0;
        mixed err = catch(stats = restore_variable(raw));
        if (err || !mapp(stats)) return 0;
        if (stats && stats["total_kills"]) {
            return stats["total_kills"];
        }
    }
    return 0;
}

// Jelenlegi esemény gyilkosságainak lekérdezése
int query_current_event_kills(string player_name) {
    mapping stats;
    string stat_file_path;
    
    if (!player_name || player_name == "" || !current_event_id) {
        return 0;
    }

    stat_file_path = SAVE_DIR + "/" + lower_case(player_name) + ".o";
    
    if (file_exists(stat_file_path)) {
        string raw = read_file(stat_file_path);
        if (!raw) return 0;
        mixed err = catch(stats = restore_variable(raw));
        if (err || !mapp(stats)) return 0;
        if (stats && stats["event_kills"] && stats["event_kills"][current_event_id]) {
            return stats["event_kills"][current_event_id];
        }
    }
    return 0;
}

// Invázió futásának ellenőrzése
int query_event_running() {
    return event_running;
}