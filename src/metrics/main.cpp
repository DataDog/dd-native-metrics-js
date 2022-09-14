#include "EventLoop.hpp"
#include "Object.hpp"

namespace datadog {
  namespace {
    static void start(const v8::FunctionCallbackInfo<v8::Value>& info) {
      EventLoop* data = reinterpret_cast<EventLoop*>(info.Data().As<v8::External>()->Value());

      data->enable();
    }

    static void stop(const v8::FunctionCallbackInfo<v8::Value>& info) {
      EventLoop* data = reinterpret_cast<EventLoop*>(info.Data().As<v8::External>()->Value());

      data->disable();
    }

    static void stats(const v8::FunctionCallbackInfo<v8::Value>& info) {
      EventLoop* data = reinterpret_cast<EventLoop*>(info.Data().As<v8::External>()->Value());
      Object obj;

      data->inject(obj);

      // Cast because V8 uses a template without type deduction support
      info.GetReturnValue().Set(static_cast<v8::Local<v8::Value>>(obj));
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
