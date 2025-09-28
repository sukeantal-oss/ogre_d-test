#define ALAP "/domains/invazio/"
#define SZOBA ALAP+"alaproom"
#define MONS ALAP+"mons/"
#define ROOMS ALAP+"rooms/"
#define OBJS ALAP+"items/"
#define PICS ALAP+"pics/"
#define DAEMONS ALAP+"daemons/"
#define FOVAROS "/domains/raknoor/rooms/"

// Az alapertelmezett esemeny idotartam 30 perc masodpercben
#define DEFAULT_DURATION 3600 // 1 ora
// Az alapertelmezett teruletek, ahol az idegenek megjelennek
#define DEFAULT_AREAS ({ "/domains/raknoor/rooms/" })
// Az idegen NPC fajljanak eleresi utja
#define ALIEN_NPC MONS+"idegen"
#define MIN_ACTIVE_ALIENS 1
// Az alapertelmezett minimum es maximum idegenek szama szobankent
#define MIN_ALIENS_PER_ROOM 1
//Maximum ennyi idegen lehet egy szobaban
#define MAX_ALIENS_PER_ROOM 10
// A szobak szama, amelyeket egy idoben spawnolunk.
#define CHUNK_SIZE 50 
// A mentes helye a killekert
#define SAVE_DIR ALAP+"save/"
// jo jellem
#define JO_ALIG 1500
//semleges jellem
#define SEMLEGES_ALIG 0
//gonosz jellem
#define GONOSZ_ALIG -1500

//Szinek 
#define B "%^BOLD%^"
#define R "%^RESET%^"
#define PIROS "%^RED%^"
#define KEK "%^BLUE%^"
#define ZOLD "%^GREEN%^"
#define SARGA "%^YELLOW%^"
#define FEHER "%^WHITE%^"
#define CYAN "%^CYAN%^"
#define MAGENTA "%^MAGENTA%^"
#define FEKETE "%^BLACK%^"
