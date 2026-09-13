#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

/* ============ macros: preproc_def ============ */
#define SIMPLE_CONST 42
#define NEGATIVE (-1)
#define STR_VERSION "1.2.3"
#define EMPTY
#define EXPR(a, b) ((a) + (b))
#define MULTILINE_SUM(a, b) \
  ((a) +                    \
   (b))
#define STRINGIFY(x) #x
#define CONCAT(a, b) a##b
#define VARIADIC(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__)
#define VA_OPT_SUM(...) sum_all(__VA_ARGS__, 0)
#define GUARD_VAR 1

/* preproc_function_def variants */
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN3(a, b, c) (MAX(MIN(a, b), MIN(b, c)))
#define UNUSED(x) (void)(x)

/* ============ typedefs: kind.typedef ============ */
typedef unsigned long ulong_t;
typedef const char *cstr_t;
typedef int *my_intptr_t;
typedef int (*cmp_fn_t)(const void *, const void *);
typedef void (*signal_handler_t)(int);
typedef int arr4_t[4];
typedef struct Point Point_alias;
typedef enum Color Color_alias;

/* ============ enums: kind.enum + kind.enumerator ============ */
enum Color {
  COLOR_RED,
  COLOR_GREEN = 10,
  COLOR_BLUE,
  COLOR_ALPHA = 1 << 7,
  COLOR_MASK = COLOR_RED | COLOR_BLUE,
  COLOR_TRAILING,
};

enum Empty_sentinel { EMPTY_DUMMY };
enum { ANON_A, ANON_B = 100 };

enum __attribute__((packed)) Packed {
  PACKED_A,
  PACKED_B = 255,
};

/* ============ structs/unions: kind.struct/union/member ============ */
struct Forward; /* no body: must NOT tag */

struct Point {
  int x;
  int y;
};

struct WithMembers {
  int plain;
  char *name;
  const char *const fixed;
  char **argv;
  char ***triple;
  int buf[256];
  int matrix[4][4];
  unsigned flags : 3;
  unsigned : 5; /* anonymous bitfield: must NOT tag */
  volatile int vcount;
  _Atomic int acount;
  void (*on_event)(int code, void *ctx);
  int (*ops[4])(int);
  struct Point nested;
  struct {
    int anon_a;
    int anon_b;
  } anon; /* anonymous struct field */
  union {
    int i;
    float f;
  } data; /* anonymous union field */
};

union Value {
  int i;
  double d;
  char bytes[8];
  void *ptr;
};

union NoBody; /* no body: must NOT tag */

struct Outer {
  struct Inner {
    int deep;
  } inner;
  union Value u;
};

/* ============ globals: kind.variable ============ */
int g_counter;
static int s_state = 1;
const char *g_version = "1.0";
volatile int g_ticks;
_Atomic int g_atomic = 0;
extern int g_errno_like;
int g_table[4] = {1, 2, 3, 4};
static const char *const g_names[] = {"a", "bb", NULL};
int g_a = 1, g_b = 2; /* multi-declarator edge: query may catch only some */
thread_local int g_tls = 0;

/* ============ preproc-conditional variables ============ */
#ifdef FEATURE_X
int g_feature_x = 1;
#endif

#if defined(USE_A)
int g_use_a = 1;
#elif defined(USE_B)
int g_use_b = 2;
#else
int g_use_fallback = 3;
#endif

#ifndef GUARD_VAR
int g_should_not_appear = 0;
#endif

/* ============ functions: kind.function ============ */
void plain_void(void) {}
int add(int a, int b) { return a + b; }
static int helper_static(int x) { return x * 2; }
inline int fast_inc(int x) { return x + 1; }
static inline int both(int x) { return x; }
extern int extern_fn(int x);
int variadic_sum(int n, ...) {
  va_list ap;
  va_start(ap, n);
  int s = 0;
  for (int i = 0; i < n; i++) s += va_arg(ap, int);
  va_end(ap);
  return s;
}
int *alloc_int(void) {
  static int v;
  return &v;
}
char *const fixed_str(void) {
  static char buf[] = "hi";
  return buf;
}
void takes_fnptr(cmp_fn_t cmp, void *base) { UNUSED(cmp); UNUSED(base); }
void takes_array(int arr[4], int m[4][4]) { UNUSED(arr); UNUSED(m); }
__attribute__((noreturn)) void die(const char *msg) {
  while (1) { UNUSED(msg); }
}
int K_and_R(a, b) int a; int b; { return a + b; }

/* function with body using static locals (locals must NOT tag) */
int with_locals(int n) {
  int local_var = n + 1;
  static int call_count = 0;
  call_count++;
  {
    int block_scoped = local_var * 2;
    local_var += block_scoped;
  }
  for (int i = 0; i < n; i++) local_var += i;
  return local_var + call_count;
}

/* prototype without body: must NOT tag as function */
int prototype_only(int a, int b);
struct Point make_point(int x, int y);
