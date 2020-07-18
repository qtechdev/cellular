#include <cstdint>
#include <map>
#include <random>
#include <vector>

#include "elementary.hpp"

static const std::map<int, qca::cell> cells = {
  {0, {0, 255, 255, 255}},
  {1, {1, 0, 0, 0}},
};

qca::rule_set qca::wolfram(const uint8_t code) {
  rule_set r;

  r[0x010101] = cells.at((code & 0b10000000) >> 7);
  r[0x010100] = cells.at((code & 0b01000000) >> 6);
  r[0x010001] = cells.at((code & 0b00100000) >> 5);
  r[0x010000] = cells.at((code & 0b00010000) >> 4);
  r[0x000101] = cells.at((code & 0b00001000) >> 3);
  r[0x000100] = cells.at((code & 0b00000100) >> 2);
  r[0x000001] = cells.at((code & 0b00000010) >> 1);
  r[0x000000] = cells.at((code & 0b00000001));

  return r;
}

std::vector<uint8_t> qca::cells_to_colour(const qca::generation &g) {
  std::vector<uint8_t> colours;

  for (const auto &c : g) {
    colours.push_back(c.r);
    colours.push_back(c.g);
    colours.push_back(c.b);
  }

  return colours;
}

std::vector<uint8_t> qca::cells_to_colour(
  const qca::history &h, const int width, const int height
) {
  std::vector<uint8_t> colours;

  int counter = 0;
  for (const auto &g : h) {
    counter++;
    for (const auto &c : g) {
      colours.push_back(c.r);
      colours.push_back(c.g);
      colours.push_back(c.b);
    }
  }

  while (counter < height) {
    counter++;
    for (int i = 0; i < width; ++i) {
      colours.push_back(0);
      colours.push_back(0);
      colours.push_back(0);
    }
  }

  return colours;
}

qca::elementary::elementary(const int w, const int h, const rule_set &r)
: field_width(w), field_height(h), rules(r) {
  engine.seed(std::random_device{}());
  init_single_1();
}

void qca::elementary::init_single_0() {
  reset();
  for (int i = 0; i < field_width; ++i) {
    current_generation.push_back(cells.at(1));
  }

  current_generation[field_width / 2] = cells.at(0);
}

void qca::elementary::init_single_1() {
  reset();
  for (int i = 0; i < field_width; ++i) {
    current_generation.push_back(cells.at(0));
  }

  current_generation[field_width / 2] = cells.at(1);
}

void qca::elementary::init_alternate() {
  reset();
  for (int i = 0; i < field_width; ++i) {
    current_generation.push_back(cells.at(i % 2));
  }
}

void qca::elementary::init_random() {
  reset();
  std::uniform_int_distribution d{0, 1};

  for (int i = 0; i < field_width; ++i) {
    current_generation.push_back(cells.at(d(engine)));
  }
}

qca::generation qca::elementary::get() const {
  return current_generation;
}

void qca::elementary::next() {
  generation next_generation;
  next_generation.resize(current_generation.size());

  for (int i = 0; i < current_generation.size(); ++i) {
    generation parents = {
      cells.at(0),
      current_generation[i],
      cells.at(0)
    };

    if (i != 0) {
      parents[0] = current_generation[i - 1];
    }

    if (i != field_width - 1) {
      parents[2] = current_generation[i + 1];
    }

    uint64_t next_index = 0;
    next_index |= parents[0].state << 16;
    next_index |= parents[1].state << 8;
    next_index |= parents[2].state;

    next_generation[i] = rules.at(next_index);
  }

  current_generation = next_generation;
}

void qca::elementary::reset() {
  current_generation.clear();
}
