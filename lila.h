#define TYPE_STRING 1      // a=<byte offset> d=<byte count>
#define TYPE_SYMBOL 2      // a=<byte offset> d=<byte count>
#define TYPE_PAIR 3        // a=<car index>   d=<cdr index>
#define TYPE_PAIR_SQUARE 4 // a=<car index>   d=<cdr index>
#define TYPE_PAIR_CURLY 5  // a=<car index>   d=<cdr index>

#define TOPLEVEL 8

#define MAXVALUES 1024

#define FLAG_DOUBLE_DASH 1

struct implementation {
    const char *name;
    const char *command;
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
extern int vflags;

void die(const char *msg);
void read_all(void);
void print_all(void);
void print_value_string(uint32_t index);

uint32_t value_type(uint32_t index);
uint32_t value_a(uint32_t index);
uint32_t value_d(uint32_t index);
int value_the_symbol_p(uint32_t index, const char *name);

int examine_toplevel_form(uint32_t index);
