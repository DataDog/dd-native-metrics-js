#pragma once

#include <stdint.h>
#include <uv.h>

#include "Object.hpp"

namespace datadog {
  class Collector {
    public:
      virtual void inject(Object carrier) = 0;
    protected:
      virtual uint64_t time_to_micro(uv_timeval_t timeval);
  };

  uint64_t Collector::time_to_micro(uv_timeval_t timeval) {
    return timeval.tv_sec * 1000 * 1000 + timeval.tv_usec;
  }
}
