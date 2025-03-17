#pragma once

#include <napi.h>
#include <v8.h>

#include "general-regs-only.hpp"

using Napi::Array;
using Napi::Env;
using Napi::Object;
using Napi::Value;

namespace datadog {
  class Heap {
    public:
      Value ToJSON(Env env);
  };

  // Extracted as it needs FP operations
  void setLong(Object& obj, const char* key, uint64_t value) {
    obj.Set(key, value);
  }

  GENERAL_REGS_ONLY Value Heap::ToJSON(Env env) {
    v8::Isolate* isolate = v8::Isolate::GetCurrent();

    Array spaces = Array::New(env);
    for (uint32_t i = 0; i < isolate->NumberOfHeapSpaces(); i++) {
      v8::HeapSpaceStatistics stats;
      if (isolate->GetHeapSpaceStatistics(&stats, i)) {
        Object space = Object::New(env);
        space.Set("space_name", stats.space_name());
        setLong(space, "space_size", stats.space_size());
        setLong(space, "space_used_size", stats.space_used_size());
        setLong(space, "space_available_size", stats.space_available_size());
        setLong(space, "physical_space_size", stats.physical_space_size());
        spaces.Set(i, space);
      }
    }

    Object heap = Object::New(env);
    heap.Set("spaces", spaces);
    return heap;
  }
}
