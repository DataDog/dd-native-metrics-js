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
      GarbageCollection(Env env);

      void Enable();
      void Disable();

      void before(v8::GCType type);
      void after(v8::GCType type);
      Value ToJSON(Env env);
    private:
      std::map<v8::GCType, Histogram> pause_;
      std::map<unsigned char, const char*> types_;
      uint64_t start_time_;
  };

  GarbageCollection::GarbageCollection(Env env) {
    auto version = VersionManagement::GetNodeVersion(env);
    if (version->major >= 18) {
      types_[1] = "scavenge";
      types_[2] = "minor_mark_compact";
      types_[4] = "mark_sweep_compact";
      types_[8] = "incremental_marking";
      types_[16] = "process_weak_callbacks";
      types_[31] = "all";
    } else {
      types_[1] = "scavenge";
      types_[2] = "mark_sweep_compact";
      types_[3] = "all";
      types_[4] = "incremental_marking";
      types_[8] = "process_weak_callbacks";
      types_[15] = "all";
    }

    pause_[v8::GCType::kGCTypeAll] = Histogram();
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
    pause_[v8::GCType::kGCTypeAll].add(usage);
  }

  Value GarbageCollection::ToJSON(Env env) {
    Object gc = Object::New(env);
    for (auto &it : pause_) {
      auto type = types_[it.first];
      gc.Set(type ? type : "unknown", it.second.ToJSON(env));
      it.second.reset();
    }
    return gc;
  }
}
