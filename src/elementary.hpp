#ifndef __ELEMENTARY_HPP__
#define __ELEMENTARY_HPP__
#include <cstdint>
#include <map>
#include <random>
#include <vector>

namespace qca {
  struct cell {
    uint8_t state;
    uint8_t r;
    uint8_t g;
    uint8_t b;
  };

  using generation = std::vector<cell>;
  using history = std::vector<generation>;
  using rule_set = std::map<uint64_t, cell>;

  rule_set wolfram(const uint8_t code);

  std::vector<uint8_t> cells_to_colour(const generation &g);
  std::vector<uint8_t> cells_to_colour(
    const history &h, const int width, const int height
  );

  class elementary {
  public:
    elementary() = default;
    elementary(const int w, const int h, const rule_set &r);

    void init_single_0();
    void init_single_1();
    void init_alternate();
    void init_random();
    void set_rules(const rule_set &r);

    generation get() const;
    void next();
    void reset();

    int field_width;
    int field_height;
  private:
    generation current_generation;
    rule_set rules;
    rule_set working_rules;

    std::mt19937 engine;
  };
}

#endif // __ELEMENTARY_HPP__
