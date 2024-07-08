test_sources = [
    'test.c',
    '../xsettings.c',
    '../xsettings.h',
]
ldflags = [
    '-Wl,--wrap=UA_Server_addDataSourceVariableNode',
    '-Wl,--wrap=UA_Variant_setScalar',
]

exe = executable('xsettings-test',
                 sources: test_sources,
                 link_args: ldflags,
                 dependencies: cmocka_dep,
                 build_by_default: false,
                 install: false)
test('xsettings', exe)
