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
      static void DeleteInstance(void* data);
  };

  AddonData::AddonData(v8::Isolate* isolate) {
    node::AddEnvironmentCleanupHook(isolate, DeleteInstance, this);
  }

  void AddonData::DeleteInstance(void* data) {
    (static_cast<AddonData*>(data))->eventLoop.disable();
    delete static_cast<AddonData*>(data);
  }
}
