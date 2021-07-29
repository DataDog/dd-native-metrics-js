#pragma once

// windows.h defines min and max macros.
#define NOMINMAX
#include <algorithm>
#undef min
#undef max
#undef NOMINMAX

#include <memory>

#include <stdint.h>
#include <tdigest/TDigest.h>

namespace datadog {
  class Histogram {
    public:
      Histogram();

      uint64_t min();
      uint64_t max();
      uint64_t sum();
      uint64_t avg();
      uint64_t count();
      uint64_t percentile(double percentile);

      void reset();
      void add(uint64_t value);
    private:
      uint64_t min_;
      uint64_t max_;
      uint64_t sum_;
      uint64_t count_;
      std::shared_ptr<tdigest::TDigest> digest_;
  };

  Histogram::Histogram() {
    reset();
  }

  uint64_t Histogram::min() { return min_; }
  uint64_t Histogram::max() { return max_; }
  uint64_t Histogram::sum() { return sum_; }
  uint64_t Histogram::avg() { return count_ == 0 ? 0 : sum_ / count_; }
  uint64_t Histogram::count() { return count_; }
  uint64_t Histogram::percentile(double value) {
    return count_ == 0 ? 0 : static_cast<uint64_t>(std::round(digest_->quantile(value)));
  }

  void Histogram::reset() {
    min_ = 0;
    max_ = 0;
    sum_ = 0;
    count_ = 0;

    digest_ = std::make_shared<tdigest::TDigest>(1000);
  }

  void Histogram::add(uint64_t value) {
    if (count_ == 0) {
      min_ = max_ = value;
    } else {
      min_ = (std::min)(min_, value);
      max_ = (std::max)(max_, value);
    }

    count_ += 1;
    sum_ += value;

    digest_->add(static_cast<tdigest::Value>(value));
  }
}
