test_exe = executable('console_swizzler_test',
    'main.cpp',
    dependencies : [console_swizzler_dep, gtest_dep, gmock_dep],
    install : false)

test('console_swizzler_test', test_exe)

subdir('dds')
