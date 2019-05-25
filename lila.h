#define TYPE_STRING 1      // string  <firstbyte> <bytelimit>
#define TYPE_SYMBOL 2      // symbol  <firstbyte> <bytelimit>
#define TYPE_PAIR 3        // ( pair <car idx> <cdr idx>
#define TYPE_PAIR_SQUARE 4 // [ pair <car idx> <cdr idx>
#define TYPE_PAIR_CURLY 5  // { pair <car idx> <cdr idx>

#define TOPLEVEL 8

#define MAXVALUES 1024

#define FLAG_DOUBLE_DASH 1

struct implementation {
    const char *name;
    const char *command;
    const char *imageflag;
    const char *scriptflag;
    size_t flags;
};

extern const struct implementation *implementations[];
extern const struct implementation *implementation;
extern const char progname[];
extern char *script;

extern char bytes[4096];
extern uint32_t values[MAXVALUES];
extern size_t nvalue;
extern size_t nbyte;
extern size_t bytepos;
extern unsigned int verbosity;

void die(const char *msg);
void read_all(void);
void print_all(void);
void print_value_string(uint32_t index);

uint32_t value_type(uint32_t index);
uint32_t value_a(uint32_t index);
uint32_t value_d(uint32_t index);
int value_the_symbol_p(uint32_t index, const char *name);

void examine_all(void);
