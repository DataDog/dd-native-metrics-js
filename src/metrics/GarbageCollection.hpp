#pragma once

#include <map>
#include <string>
#include <stdint.h>

#include <napi.h>
#include <uv.h>
#include <v8.h>

#include "Histogram.hpp"

using Napi::Env;
using Napi::Object;
using Napi::Value;
using Napi::VersionManagement;

namespace datadog {
  class GarbageCollection {
    public:
      GarbageCollection();

      void Enable();
      void Disable();

      void before(v8::GCType type);
      void after(v8::GCType type);
      Value ToJSON(Env env);
    private:
      std::map<v8::GCType, Histogram> pause_;
      Histogram pause_all_;
      std::map<unsigned char, const char*> types_;
      uint64_t start_time_;

      const char* ToType(Env env, v8::GCType);
  };

  GarbageCollection::GarbageCollection() {
    start_time_ = uv_hrtime();
  }

  void before_gc(v8::Isolate* isolate, v8::GCType type, v8::GCCallbackFlags flags, void* data) {
    reinterpret_cast<GarbageCollection*>(data)->before(type);
  }

  void after_gc(v8::Isolate* isolate, v8::GCType type, v8::GCCallbackFlags flags, void* data) {
    reinterpret_cast<GarbageCollection*>(data)->after(type);
  }

  void GarbageCollection::Enable() {
    v8::Isolate* isolate = v8::Isolate::GetCurrent();

    isolate->AddGCPrologueCallback(before_gc, this);
    isolate->AddGCEpilogueCallback(after_gc, this);
  }

  void GarbageCollection::Disable() {
    v8::Isolate* isolate = v8::Isolate::GetCurrent();

    isolate->RemoveGCPrologueCallback(before_gc, this);
    isolate->RemoveGCEpilogueCallback(after_gc, this);
  }

  void GarbageCollection::before(v8::GCType type) {
    start_time_ = uv_hrtime();
  }

  void GarbageCollection::after(v8::GCType type) {
    uint64_t usage = uv_hrtime() - start_time_;

    if (pause_.find(type) == pause_.end()) {
      pause_[type] = Histogram();
    }

    pause_[type].add(usage);
    pause_all_.add(usage);
  }

  Value GarbageCollection::ToJSON(Env env) {
    Object gc = Object::New(env);

    for (auto &it : pause_) {
      printf("%u\n", it.first);
      auto type = this->ToType(env, it.first);
      gc.Set(type, it.second.ToJSON(env));
      it.second.reset();
    }

    gc.Set("all", pause_all_.ToJSON(env));
    pause_all_.reset();

    return gc;
  }

  const char* GarbageCollection::ToType(Env env, v8::GCType type) {
    auto version = VersionManagement::GetNodeVersion(env);
    auto type_bit = static_cast<char>(type);

    printf("major: %u\n", version->major);

    if (version->major >= 22) {
      switch (type_bit) {
        case 1: return "scavenge";
        case 2: return "minor_mark_sweep";
        case 4: return "mark_sweep_compact"; // Deprecated, might be removed soon.
        case 8: return "incremental_marking";
        case 16: return "process_weak_callbacks";
        case 31: return "all";
      }
    } else if (version->major >= 18) {
      switch (type_bit) {
        case 1: return "scavenge";
        case 2: return "minor_mark_compact";
        case 4: return "mark_sweep_compact";
        case 8: return "incremental_marking";
        case 16: return "process_weak_callbacks";
        case 31: return "all";
      }
    } else {
      switch (type_bit) {
        case 1: return "scavenge";
        case 2: return "mark_sweep_compact";
        case 4: return "incremental_marking";
        case 8: return "process_weak_callbacks";
        case 15: return "all";
      }
    }

    return "unknown";
  }
}
