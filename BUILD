cc_library(
    name = 'xproto',
    srcs = [
        'json-message-codec.cpp',
        'object-reader.cpp',
        'protobuf-message-codec.cpp',
        'protobuf-message-factory.cpp',
        'text-message-codec.cpp',
        'xml-message-codec.cpp',
    ],
    deps = [
        '//thirdparty/protobuf:protobuf',
        '//thirdparty/boost:boost'
    ],
    warning = '-Wno-frame-larger-than'
)

proto_library(
    name = 'person_proto',
    srcs = 'person.proto'
)

cc_test(
    name = 'json-message-codec_test',
    srcs = ['json-message-codec_test.cpp'],
    deps = [
        ':xproto',
        ':person_proto'
    ],
    testdata = ['data.json']
)

cc_test(
    name = 'protobuf-message-factory_test',
    srcs = ['protobuf-message-factory_test.cpp'],
    deps = [
        ':xproto',
        ':person_proto'
    ],
    testdata = ['mock-server.proto']
)

cc_test(
    name = 'text-message-codec_test',
    srcs = ['text-message-codec_test.cpp'],
    deps = [
        ':xproto',
        ':person_proto'
    ],
    testdata = ['data.txt']
)

cc_test(
    name = 'xml-message-codec_test',
    srcs = ['xml-message-codec_test.cpp'],
    deps = [
        ':xproto',
        ':person_proto'
    ],
    testdata = ['data.xml']
)

