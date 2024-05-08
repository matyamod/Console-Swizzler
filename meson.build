project('console-swizzler', 'c',
    default_options: [
        'c_std=c99',
        'cpp_std=c++11',
    ],
    meson_version: '>=0.48.0',
    version: '0.3.1')

# Set dependencies for macOS
if host_machine.system() == 'darwin' and not meson.is_subproject()
    macosx_version_min = '-mmacosx-version-min=' + get_option('macosx_version_min')
    add_global_arguments(macosx_version_min, language: 'c')
    add_global_link_arguments(macosx_version_min, language: 'c')

    # Check if SDKs support universal binaries or not.
    arch = ['-arch', 'x86_64', '-arch', 'arm64']
    c_compiler = meson.get_compiler('c')
    result = c_compiler.run(
        'int main(void) { return 0; }',
        name : 'universal binary test',
        args: arch)
    if result.compiled()
        add_global_arguments(arch, language: 'c')
        add_global_link_arguments(arch, language: 'c')
    else
        warning('Universal build is disabled since your SDKs do not support it.')
    endif
endif

# Build the library
swiz_sources = [
    'src/context.c',
    'src/swizfunc.c',
    'src/util.c',
]

console_swizzler = library('console-swizzler',
    swiz_sources,
    install: true,
    include_directories: include_directories('./include'),
    gnu_symbol_visibility: 'hidden')
install_headers('include/console-swizzler.h')

console_swizzler_dep = declare_dependency(
    include_directories: include_directories('./include'),
    link_with : console_swizzler)

# Build swizzler-cli
if get_option('cli')
    cli_sources = [
        'swizzler-cli/main.c',
        'swizzler-cli/dds.c',
    ]
    executable('swizzler-cli',
        cli_sources,
        dependencies: console_swizzler_dep,
        install : true)
endif

# Build unit tests
if get_option('tests')
    add_languages('cpp', required: true)

    # get gtest
    gtest_proj = subproject('gtest')
    gtest_dep = gtest_proj.get_variable('gtest_dep')
    gmock_dep = gtest_proj.get_variable('gmock_dep')

    # build tests
    subdir('tests')
endif
