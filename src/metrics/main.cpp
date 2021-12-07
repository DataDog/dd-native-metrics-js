#include "EventLoop.hpp"
#include "Object.hpp"

namespace datadog {
  namespace {
    void before_gc(v8::Isolate* isolate, v8::GCType type, v8::GCCallbackFlags flags, void* data) {
      reinterpret_cast<EventLoop*>(data)->gc.before(type);
    }

    void after_gc(v8::Isolate* isolate, v8::GCType type, v8::GCCallbackFlags flags, void* data) {
      reinterpret_cast<EventLoop*>(data)->gc.after(type);
    }

    static void start(const v8::FunctionCallbackInfo<v8::Value>& info) {
      EventLoop* data = reinterpret_cast<EventLoop*>(info.Data().As<v8::External>()->Value());
      v8::Isolate* isolate = v8::Isolate::GetCurrent();

      data->enable();

      isolate->AddGCPrologueCallback(before_gc, data);
      isolate->AddGCEpilogueCallback(after_gc, data);
    }

    static void stop(const v8::FunctionCallbackInfo<v8::Value>& info) {
      EventLoop* data = reinterpret_cast<EventLoop*>(info.Data().As<v8::External>()->Value());
      v8::Isolate* isolate = v8::Isolate::GetCurrent();

      data->disable();

      isolate->RemoveGCPrologueCallback(before_gc, data);
      isolate->RemoveGCEpilogueCallback(after_gc, data);
    }

    static void stats(const v8::FunctionCallbackInfo<v8::Value>& info) {
      EventLoop* data = reinterpret_cast<EventLoop*>(info.Data().As<v8::External>()->Value());
      Object obj;

      data->inject(obj);
      data->gc.inject(obj);
      data->process.inject(obj);
      data->heap.inject(obj);

      info.GetReturnValue().Set(obj.to_json());
    }
  }

  NODE_MODULE_INIT() {
    v8::Isolate* isolate = context->GetIsolate();
    EventLoop* data = new EventLoop(isolate);
    v8::Local<v8::External> external = v8::External::New(isolate, data);

    exports->Set(
      context,
      Nan::New("start").ToLocalChecked(),
      v8::FunctionTemplate::New(isolate, start, external)->GetFunction(context).ToLocalChecked()
    ).FromJust();

    exports->Set(
      context,
      Nan::New("stop").ToLocalChecked(),
      v8::FunctionTemplate::New(isolate, stop, external)->GetFunction(context).ToLocalChecked()
    ).FromJust();

    exports->Set(
      context,
      Nan::New("stats").ToLocalChecked(),
      v8::FunctionTemplate::New(isolate, stats, external)->GetFunction(context).ToLocalChecked()
    ).FromJust();
  }
}
