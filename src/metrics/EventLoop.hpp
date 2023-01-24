#pragma once

#include <stdint.h>

#include <node_api.h>
#include <napi.h>
#include <uv.h>

#include "Histogram.hpp"

// TODO: use UV_METRICS_IDLE_TIME

using Napi::Env;
using Napi::Object;
using Napi::Value;

namespace datadog {
  // http://docs.libuv.org/en/v1.x/design.html#the-i-o-loop
  class EventLoop {
    public:
      explicit EventLoop(Env env);
      virtual ~EventLoop() = default;
      EventLoop(const EventLoop&) = delete;
      void operator=(const EventLoop&) = delete;

      static void delete_instance(void* arg);

      void Enable();
      void Disable();
      Value ToJSON(Env env);
    protected:
      static void check_cb(uv_check_t* handle);
      static void prepare_cb(uv_prepare_t* handle);
      static void close_cb(uv_handle_t* handle);

      void Close();
    private:
      int handle_count_;
      uv_loop_t* loop;
      uv_check_t check_handle_;
      uv_prepare_t prepare_handle_;
      uint64_t check_time_;
      uint64_t prepare_time_;
      uint64_t timeout_;
      Histogram histogram_;
      bool enabled_;

      uint64_t usage();
  };

  EventLoop::EventLoop(Env env) {
    napi_get_uv_event_loop(env, &loop);

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
    enabled_ = false;

    env.AddCleanupHook(&EventLoop::delete_instance, this);
  }

  void EventLoop::check_cb(uv_check_t* handle) {
    EventLoop* self = static_cast<EventLoop*>(handle->data);
    if (!self->enabled_) return;

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

  void EventLoop::prepare_cb(uv_prepare_t* handle) {
    EventLoop* self = static_cast<EventLoop*>(handle->data);
    if (!self->enabled_) return;
    uv_loop_t* loop = self->loop;

    self->prepare_time_ = uv_hrtime();
    self->timeout_ = uv_backend_timeout(loop);
  }

  void EventLoop::close_cb(uv_handle_t* handle) {
    EventLoop* self = static_cast<EventLoop*>(handle->data);

    --self->handle_count_;

    if (self->handle_count_ == 0) {
      delete self;
    }
  }

  void EventLoop::Enable() {
    if (enabled_) return;
    enabled_ = true;
    uv_check_start(&check_handle_, &EventLoop::check_cb);
    uv_prepare_start(&prepare_handle_, &EventLoop::prepare_cb);
  }

  void EventLoop::Disable() {
    if (!enabled_) return;
    enabled_ = false;
    uv_check_stop(&check_handle_);
    uv_prepare_stop(&prepare_handle_);
    histogram_.reset();
  }

  Value EventLoop::ToJSON(Env env) {
    Value eventLoop = histogram_.ToJSON(env);
    histogram_.reset();
    return eventLoop;
  }

  void EventLoop::Close() {
    uv_close(reinterpret_cast<uv_handle_t*>(&check_handle_), &EventLoop::close_cb);
    uv_close(reinterpret_cast<uv_handle_t*>(&prepare_handle_), &EventLoop::close_cb);
  }

  void EventLoop::delete_instance(void* arg) {
    EventLoop* data = static_cast<EventLoop*>(arg);

    data->Disable();
    data->Close();
  }
}
