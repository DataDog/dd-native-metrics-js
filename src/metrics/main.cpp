#include <napi.h>

#include "EventLoop.hpp"
#include "GarbageCollection.hpp"
#include "Heap.hpp"
#include "Process.hpp"
#include "general-regs-only.hpp"

using Napi::Addon;
using Napi::CallbackInfo;
using Napi::Env;
using Napi::Object;
using Napi::Value;

namespace datadog {
  class NativeMetrics : public Addon<NativeMetrics> {
    private:
      Process processInfo;
      GarbageCollection gcInfo;
      Heap heapInfo;
      // This needs to be a pointer so it can live longer than env for uv_close
      EventLoop* loopInfo;

      GENERAL_REGS_ONLY Value Start(const CallbackInfo& info) {
        if (info.Length() > 0) { // Enable only selected watchers
          for (size_t i = 0; i < info.Length(); i++) {
            std::string watcher = info[i].As<Napi::String>().Utf8Value();

            if (watcher == "loop") {
              loopInfo->Enable();
            } else if (watcher == "gc") {
              gcInfo.Enable();
            }
          }
        } else { // Enable all watchers
          gcInfo.Enable();
          loopInfo->Enable();
        }
        return info.Env().Undefined();
      }

      Value Stop(const CallbackInfo& info) {
        gcInfo.Disable();
        loopInfo->Disable();
        return info.Env().Undefined();
      }

      Value Stats(const CallbackInfo& info) {
        Env env = info.Env();
        Object obj = Object::New(env);
        obj.Set("cpu", processInfo.ToJSON(env));
        obj.Set("heap", heapInfo.ToJSON(env));
        obj.Set("gc", gcInfo.ToJSON(env));
        obj.Set("eventLoop", loopInfo->ToJSON(env));
        return obj;
      }

    public:
      NativeMetrics(Env env, Object exports)
        : loopInfo(new EventLoop(env)) {
        DefineAddon(exports, {
          InstanceMethod("start", &NativeMetrics::Start),
          InstanceMethod("stop", &NativeMetrics::Stop),
          InstanceMethod("stats", &NativeMetrics::Stats)
        });
      }
  };

  NODE_API_ADDON(NativeMetrics)
}
