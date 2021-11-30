{
  "targets": [{
    "target_name": "metrics",
    "sources": [
      "src/metrics/main.cpp"
    ],
    "include_dirs": [
      "src",
      "<!(node -e \"require('nan')\")"
    ],
    "xcode_settings": {
      "MACOSX_DEPLOYMENT_TARGET": "11.0",
      "OTHER_CFLAGS": [
        "-std=c++14",
        "-stdlib=libc++",
        "-Wall",
        "-Werror"
      ]
    },
    "conditions": [
      ["OS == 'linux'", {
        "cflags": [
          "-std=c++11",
          "-Wall",
          "-Werror"
        ],
        "cflags_cc": [
          "-Wno-cast-function-type"
        ]
      }],
      ["OS == 'win'", {
        "cflags": [
          "/WX"
        ]
      }]
    ]
  }]
}
