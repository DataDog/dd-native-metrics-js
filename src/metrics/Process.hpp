#pragma once

#include <uv.h>

#include "Collector.hpp"
#include "Object.hpp"

namespace datadog {
  class Process : public Collector {
    public:
      void enable();
      void disable();
      void inject(Object carrier);
    private:
      bool enabled_;
      uv_rusage_t usage_;
  };

  void Process::inject(Object carrier) {
    uv_rusage_t usage;
    uv_getrusage(&usage);

    Object cpu;

    cpu.set("user", time_to_micro(usage.ru_utime) - time_to_micro(usage_.ru_utime));
    cpu.set("system", time_to_micro(usage.ru_stime) - time_to_micro(usage_.ru_stime));

    carrier.set("cpu", cpu);

    usage_ = usage;
  }
}
