#ifndef CORPUS_WIDGET_H
#define CORPUS_WIDGET_H

#define WIDGET_VERSION_MAJOR 2
#define WIDGET_CLAMP(v, lo, hi) ((v) < (lo) ? (lo) : ((v) > (hi) ? (hi) : (v)))

#include <string>

namespace widget {

enum class Kind { Button, Slider, Canvas = 10 };

struct Config {
  int width;
  int height;
  const char *title;
  int flags : 3;
};

class Widget {
public:
  Widget();
  explicit Widget(const Config &cfg);
  virtual ~Widget();
  virtual void draw() const = 0;
  void resize(int w, int h);
  static Widget *create(Kind k);
  static int instance_count;

protected:
  Config cfg_;

private:
  int id_;
};

class Button : public Widget {
public:
  void draw() const override;
  void on_click();
};

typedef Widget *widget_ptr_t;
using widget_creator = Widget *(*)(Kind);

extern int g_widget_debug;
extern Widget *default_widget();

} // namespace widget

#endif
