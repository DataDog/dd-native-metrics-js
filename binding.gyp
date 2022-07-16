{
  "targets": [{
    "target_name": "metrics",
    "sources": [
      "src/metrics/Process.cpp",
      "src/metrics/Heap.cpp",
      "src/metrics/GarbageCollection.cpp",
      "src/metrics/Collector.cpp",
      "src/metrics/EventLoop.cpp",
      "src/metrics/Histogram.cpp",
      "src/metrics/Object.cpp",
      "src/metrics/main.cpp",
    ],
    "include_dirs": [
      "src",
      "<!(node -e \"require('nan')\")"
    ],
    "xcode_settings": {
      "MACOSX_DEPLOYMENT_TARGET": "10.10",
      "OTHER_CFLAGS": [
        "-std=c++14",
        "-stdlib=libc++",
        "-Wall"
      ]
    },
    "conditions": [
      ["OS == 'linux'", {
        "cflags": [
          "-std=c++11",
          "-Wall"
        ],
        "cflags_cc": [
          "-Wno-cast-function-type"
        ]
      }],
      ["OS == 'win'", {
        "cflags": []
      }]
    ]
  }]
}
