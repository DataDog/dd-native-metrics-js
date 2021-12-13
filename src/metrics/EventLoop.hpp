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

  EventLoop::EventLoop(v8::Isolate* isolate) {
    uv_loop_t* loop = Nan::GetCurrentEventLoop();

    uv_check_init(loop, &check_handle_);
    uv_prepare_init(loop, &prepare_handle_);
    uv_unref(reinterpret_cast<uv_handle_t*>(&check_handle_));
    uv_unref(reinterpret_cast<uv_handle_t*>(&prepare_handle_));

    handle_count_ = 2;
    check_handle_.data = (void*)this;
    prepare_handle_.data = (void*)this;

    check_time_ = uv_hrtime();
    prepare_time_ = check_time_;
    timeout_ = 0;

    node::AddEnvironmentCleanupHook(isolate, delete_instance, this);
  }

  void EventLoop::check_cb (uv_check_t* handle) {
    EventLoop* self = (EventLoop*)handle->data;

    uint64_t check_time = uv_hrtime();
    uint64_t poll_time = check_time - self->prepare_time_;
    uint64_t latency = self->prepare_time_ - self->check_time_;
    uint64_t timeout = self->timeout_ * 1000 * 1000;

    if (poll_time > timeout) {
      latency += poll_time - timeout;
    }

    self->histogram_.add(latency);
    self->check_time_ = check_time;
  }

  void EventLoop::prepare_cb (uv_prepare_t* handle) {
    uv_loop_t* loop = Nan::GetCurrentEventLoop();
    EventLoop* self = (EventLoop*)handle->data;

    self->prepare_time_ = uv_hrtime();
    self->timeout_ = uv_backend_timeout(loop);
  }

  void EventLoop::close_cb (uv_handle_t* handle) {
    EventLoop* self = (EventLoop*)handle->data;

    --self->handle_count_;

    if (self->handle_count_ == 0) {
      delete self;
    }
  }

  void EventLoop::enable() {
    uv_check_start(&check_handle_, &EventLoop::check_cb);
    uv_prepare_start(&prepare_handle_, &EventLoop::prepare_cb);
  }

  void EventLoop::disable() {
    uv_check_stop(&check_handle_);
    uv_prepare_stop(&prepare_handle_);
    histogram_.reset();
  }

  void EventLoop::inject(Object carrier) {
    carrier.set("eventLoop", histogram_);
    histogram_.reset();
  }

  void EventLoop::close() {
    uv_close(reinterpret_cast<uv_handle_t*>(&check_handle_), &EventLoop::close_cb);
    uv_close(reinterpret_cast<uv_handle_t*>(&prepare_handle_), &EventLoop::close_cb);
  }

  void EventLoop::delete_instance(void* arg) {
    EventLoop* data = (static_cast<EventLoop*>(arg));

    data->close();
  }
}
