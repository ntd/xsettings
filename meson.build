project('xsettings', 'c',
    version: '0.0.1',
    license: 'LGPLv2.1+'
)

open62541_dep = dependency('open62541')
cmocka_dep = dependency('cmocka', required: get_option('test'))

subdir('demo')
if cmocka_dep.found()
    subdir('test')
endif
