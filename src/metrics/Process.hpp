#pragma once

#include <napi.h>
#include <uv.h>

using Napi::Env;
using Napi::Object;
using Napi::Value;

namespace datadog {
  uint64_t time_to_micro(uv_timeval_t timeval) {
    return timeval.tv_sec * 1000 * 1000 + timeval.tv_usec;
  }

  class Process {
    private:
      uv_rusage_t usage_;

    public:
      Process();
      Value ToJSON(Env env);
  };

  Process::Process() {
    memset(&usage_, 0, sizeof(usage_));
  }

  Value Process::ToJSON(Env env) {
    uv_rusage_t usage;
    uv_getrusage(&usage);

    auto user = time_to_micro(usage.ru_utime) - time_to_micro(usage_.ru_utime);
    auto system = time_to_micro(usage.ru_stime) - time_to_micro(usage_.ru_stime);

    usage_ = usage;

    Object cpu = Object::New(env);
    cpu.Set("user", user);
    cpu.Set("system", system);
    return cpu;
  }
}
