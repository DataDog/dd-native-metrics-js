#pragma once

#include <nan.h>
#include <stdint.h>
#include <string>
#include <uv.h>
#include <vector>

#include "Histogram.hpp"

namespace datadog {
  class Object {
    public:
      Object(v8::Local<v8::Object> target)
        : target_(target) {}

      Object()
        : Object(Nan::New<v8::Object>()) {}

      void set(const char* key, v8::Local<v8::Value> value) {
        Nan::Set(target_, Nan::New(key).ToLocalChecked(), value);
      }

      void set(const char* key, const char* value) {
        set(key, Nan::New(value).ToLocalChecked());
      }

      void set(const char* key, uint64_t value) {
        set(key, Nan::New<v8::Number>(static_cast<double>(value)));
      }

      void set(const char* key, std::vector<Object> value) {
        v8::Local<v8::Array> array = Nan::New<v8::Array>(value.size());

        for (unsigned int i = 0; i < array->Length(); i++) {
          Nan::Set(array, i, value.at(i));
        }

        set(key, array);
      }

      void set(const char* key, Histogram value) {
        Object obj;

        obj.set("min", value.min());
        obj.set("max", value.max());
        obj.set("sum", value.sum());
        obj.set("avg", value.avg());
        obj.set("count", value.count());
        obj.set("median", value.percentile(0.50));
        obj.set("p95", value.percentile(0.95));

        set(key, obj);
      }

      operator v8::Local<v8::Value>() {
        return target_;
      }
    private:
      v8::Local<v8::Object> target_;
  };
}
