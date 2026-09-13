#include <string>
#include <vector>

#define NS_MACRO 100
#define MAX2(a, b) ((a) > (b) ? (a) : (b))

/* free functions: overloads, defaults, trailing return, noexcept, constexpr */
int free_add(int a, int b) { return a + b; }
int free_add(int a, int b, int c) { return a + b + c; }
int with_defaults(int a, int b = 2, const std::string &s = "x") {
  return a + b + (int)s.size();
}
auto trailing_ret(int x) -> int { return x * 2; }
int may_throw(int x) noexcept(false) { return x; }
int no_throw(int x) noexcept { return x; }
constexpr int square(int x) { return x * x; }
int *alloc_cpp(int n) { return new int[n]; }
void sink_fn(int (&arr)[4]) { (void)arr; }

template <typename T> T tmax(T a, T b) { return a > b ? a : b; }
template <typename T, typename... Rest> T vsum(T a, Rest... rest) {
  return a + vsum(rest...);
}
int vsum(int x) { return x; }
template <> const char *tmax<const char *>(const char *a, const char *b) {
  return a;
}

/* operator + conversion operator edge */
struct Comparable {
  int v;
  bool operator==(const Comparable &o) const { return v == o.v; }
  operator int() const { return v; }
};

/* classes / structs / unions */
class Empty {};

class Shape {
public:
  Shape();
  virtual ~Shape();
  virtual double area() const = 0;
  static int live_count;
  mutable int cache_hits = 0;
  const int id = 0;

protected:
  std::string name_;

private:
  int secret_ : 4;
  char *label_;
  int buf_[16];
  void (*cb_)(int);
};

int Shape::live_count = 0;
Shape::Shape() {}
Shape::~Shape() {}

class Circle : public Shape {
public:
  Circle(double r);
  double area() const override;
  double radius() const;

private:
  double radius_;
};

Circle::Circle(double r) : radius_(r) {}
double Circle::area() const { return 3.14 * radius_ * radius_; }
double Circle::radius() const { return radius_; }

class Final final {
public:
  void done() {}
};

template <typename T> class Box {
public:
  T get() const { return value_; }
  void set(const T &v) { value_; }

private:
  T value_;
  static int instances;
};

template <typename T> int Box<T>::instances = 0;

struct PlainStruct {
  int x;
  int y;
  void reset();
};

void PlainStruct::reset() { x = y = 0; }

union Word {
  int i;
  float f;
  char b[4];
};

typedef unsigned long size_alias_t;
using vec_int = std::vector<int>;
using str_vec_fn = std::string (*)(int);

enum Plain { PA, PB = 5, PC };
enum class Color : unsigned char { Red, Green = 10, Blue };
enum struct Direction { North, South, East, West };

namespace outer {
int ns_var = 1;
int ns_fn(int x) { return x + NS_MACRO; }

class InNs {
public:
  void hello();
};
void InNs::hello() {}

namespace inner {
int deep_var = 2;
}

inline namespace v2 {
int inline_ns_var = 3;
}

namespace {
int anon_ns_var = 4;
}
} // namespace outer

namespace alias_ns = outer::inner;

constexpr int KAnswer = 42;
inline int g_inline_var = 7;
thread_local int g_ns_tls = 0;

#ifdef WITH_EXTRA
int g_with_extra = 1;
#endif

#if defined(USE_CPP_A)
int g_use_cpp_a = 1;
#endif

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  Circle c(2.0);
  Box<int> b;
  b.set(3);
  auto lam = [](int x) { return x + 1; };
  (void)lam;
  return (int)c.area();
}
