#include "EventLoop.hpp"
#include "GarbageCollection.hpp"
#include "Heap.hpp"
#include "Process.hpp"

namespace datadog {
  class AddonData {
    public:
      EventLoop eventLoop;
      GarbageCollection gc;
      Heap heap;
      Process process;

      explicit AddonData(v8::Isolate* isolate);
      static void delete_instance(void* arg, void (*cb)(void*), void* cbarg);
  };

  AddonData::AddonData(v8::Isolate* isolate) {
    node::AddEnvironmentCleanupHook(isolate, delete_instance, this);
  }

  void AddonData::delete_instance(void* arg, void (*cb)(void*), void* cbarg) {
    AddonData* data = (static_cast<AddonData*>(arg));

    data->eventLoop.close([=]() -> void {
      delete data;
      cb(cbarg);
    });
  }
}
