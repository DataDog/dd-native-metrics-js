#pragma once

#include <stdint.h>
#include <uv.h>

#include "Collector.hpp"
#include "GarbageCollection.hpp"
#include "Heap.hpp"
#include "Histogram.hpp"
#include "Process.hpp"

// TODO: split AddonData from EventLoop
// TODO: use UV_METRICS_IDLE_TIME

namespace datadog {
  // http://docs.libuv.org/en/v1.x/design.html#the-i-o-loop
  class EventLoop : public Collector {
    public:
      explicit EventLoop(v8::Isolate* isolate);
      virtual ~EventLoop() = default;
      EventLoop(const EventLoop&) = delete;
      void operator=(const EventLoop&) = delete;

      GarbageCollection gc;
      Heap heap;
      Process process;

      static void delete_instance(void* arg);

      void enable();
      void disable();
      void inject(Object carrier);
    protected:
      static void check_cb (uv_check_t* handle);
      static void prepare_cb (uv_prepare_t* handle);
      static void close_cb (uv_handle_t* handle);

      void close();
    private:
      int handle_count_;
      uv_check_t check_handle_;
      uv_prepare_t prepare_handle_;
      uint64_t check_time_;
      uint64_t prepare_time_;
      uint64_t timeout_;
      Histogram histogram_;

      uint64_t usage();
  };
}
