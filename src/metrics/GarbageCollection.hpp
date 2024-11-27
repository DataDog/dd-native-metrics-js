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
      explicit GarbageCollection(Env env);
      virtual ~GarbageCollection() = default;
      GarbageCollection(const GarbageCollection&) = delete;
      void operator=(const GarbageCollection&) = delete;

      static void delete_instance(napi_async_cleanup_hook_handle handle, void* arg);

      void Enable();
      void Disable();

      void before(v8::GCType type);
      void after(v8::GCType type);
      Value ToJSON(Env env);
    protected:
      void Close();
    private:
      std::map<v8::GCType, Histogram> pause_;
      Histogram pause_all_;
      std::map<unsigned char, std::string> types_;
      uint64_t start_time_;
      bool enabled_;
      napi_async_cleanup_hook_handle remove_handle_;

      const std::string ToType(Env env, v8::GCType);
  };

  GarbageCollection::GarbageCollection(Env env) {
    start_time_ = uv_hrtime();
    enabled_ = false;

    napi_add_async_cleanup_hook(env, &GarbageCollection::delete_instance, this, &remove_handle_);
  }

  void before_gc(v8::Isolate* isolate, v8::GCType type, v8::GCCallbackFlags flags, void* data) {
    reinterpret_cast<GarbageCollection*>(data)->before(type);
  }

  void after_gc(v8::Isolate* isolate, v8::GCType type, v8::GCCallbackFlags flags, void* data) {
    reinterpret_cast<GarbageCollection*>(data)->after(type);
  }

  void GarbageCollection::Enable() {
    if (enabled_) return;
    enabled_ = true;

    v8::Isolate* isolate = v8::Isolate::GetCurrent();

    isolate->AddGCPrologueCallback(before_gc, this);
    isolate->AddGCEpilogueCallback(after_gc, this);
  }

  void GarbageCollection::Disable() {
    if (!enabled_) return;
    enabled_ = false;

    v8::Isolate* isolate = v8::Isolate::GetCurrent();

    isolate->RemoveGCPrologueCallback(before_gc, this);
    isolate->RemoveGCEpilogueCallback(after_gc, this);

    pause_.clear();
    pause_all_.reset();
    types_.clear();
    start_time_ = 0;
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
      auto type = this->ToType(env, it.first);
      gc.Set(type, it.second.ToJSON(env));
      it.second.reset();
    }

    gc.Set("all", pause_all_.ToJSON(env));
    pause_all_.reset();

    return gc;
  }

  void GarbageCollection::delete_instance(napi_async_cleanup_hook_handle handle, void* arg) {
    GarbageCollection* data = static_cast<GarbageCollection*>(arg);

    data->Disable();
    data->Close();
  }

  void GarbageCollection::Close() {
    napi_remove_async_cleanup_hook(remove_handle_);

    delete this;
  }

  const std::string GarbageCollection::ToType(Env env, v8::GCType type) {
    auto version = VersionManagement::GetNodeVersion(env);
    auto type_bit = static_cast<char>(type);

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
