#pragma once

#include <array>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <span>
#include <vector>

namespace {
using std::array;
using std::int16_t;
using std::int8_t;
using std::uint16_t;
using std::uint8_t;
}; // namespace

template <int inputs, int hidden, int outputs> struct SNN {
  array<int8_t, hidden * inputs> w_hidden_input;
  array<int8_t, hidden * hidden> w_hidden_hidden;
  array<int8_t, outputs * hidden> w_output_hidden;
  array<uint8_t, hidden> b_hidden;
  array<uint8_t, hidden> s_hidden;
  array<bool, hidden> act_hidden;

  void init(std::mt19937 &rng) {
    // He/Xavier-inspired initialization scaled for uint8_t/int8_t ranges

    // Input->Hidden weights: scale by sqrt(2/fan_in) for ReLU-like activation
    float input_scale = std::sqrt(2.0f / inputs);
    int input_range = static_cast<int>(64 * input_scale);
    input_range = std::clamp(input_range, 1, 127);
    std::uniform_int_distribution<int> dist_input(-input_range, input_range);

    // Hidden->Hidden weights: scale by sqrt(1/fan_in) for recurrent connections
    float hidden_scale = std::sqrt(1.0f / hidden);
    int hidden_range = static_cast<int>(128 * hidden_scale);
    hidden_range = std::clamp(hidden_range, 1, 127);
    std::uniform_int_distribution<int> dist_hidden(-hidden_range, hidden_range);

    // Output weights: Xavier initialization sqrt(1/fan_in) for linear output
    float output_scale = std::sqrt(1.0f / hidden);
    int output_range = static_cast<int>(128 * output_scale);
    output_range = std::clamp(output_range, 1, 127);
    std::uniform_int_distribution<int> dist_output(-output_range, output_range);

    // Bias: small positive values
    std::uniform_int_distribution<int> dist_bias(input_range / 2, input_range);

    for (auto &w : w_hidden_input)
      w = static_cast<uint8_t>(dist_input(rng));
    for (auto &w : w_hidden_hidden)
      w = static_cast<uint8_t>(dist_hidden(rng));
    for (auto &w : w_output_hidden)
      w = static_cast<int8_t>(dist_output(rng));
    for (auto &b : b_hidden)
      b = static_cast<uint8_t>(dist_bias(rng));

    clear();
  }

  void clear() {
    std::fill(s_hidden.begin(), s_hidden.end(), 0);
    std::fill(act_hidden.begin(), act_hidden.end(), false);
  }

  void update(std::span<int8_t const> input) {
    static constexpr uint8_t LEAK_SHIFT = 3; // leak rate (divide by 8)
    static constexpr int THRESHOLD = std::numeric_limits<uint8_t>::max();
    array<bool, hidden> act_hidden_next;

    for (auto i = 0; i < hidden; ++i) {
      // start with current accumulated hidden state
      int16_t acc = s_hidden[i];
      // apply leak
      acc -= static_cast<uint8_t>(acc) >> LEAK_SHIFT;
      // add bias
      acc += b_hidden[i];
      // add inputs
      for (auto j = 0; j < inputs; ++j) {
        acc += (static_cast<int16_t>(w_hidden_input[i * inputs + j]) *
                    static_cast<int16_t>(input[j]) >>
                8);
      }

      // std::cout << "after inputs: " << (int)acc << std::endl;
      // add recurrent connections
      for (auto j = 0; j < hidden; ++j) {
        if (act_hidden[j]) {
          acc += w_hidden_hidden[i * hidden + j];
        }
      }

      // check if the neuron spikes
      act_hidden_next[i] = acc >= THRESHOLD;
      // std::cout << (int)acc << std::endl;

      // update the state
      s_hidden[i] = static_cast<uint8_t>(acc);
      if (acc >= THRESHOLD || acc < 0) {
        s_hidden[i] = 0;
      }
    }
    // copy over new activations
    act_hidden = act_hidden_next;

    // for (auto i = 0; i < hidden; ++i) {
    //   std::cout << (int)s_hidden[i] << " ";
    // }
    std::cout << std::endl;
  }

  void get_output(std::vector<int16_t> &output) {
    output.resize(outputs);
    // init to all zeros
    std::fill(output.begin(), output.end(), 0);
    for (auto j = 0; j < outputs; ++j) {
      for (auto i = 0; i < hidden; ++i) {
        // after a spike, the s_hidden is 0
        if (act_hidden[i]) {
          output[j] += w_output_hidden[j * hidden + i];
        }
      }
    }
  }
};
