project('console-swizzler', 'c',
    default_options: [
        'c_std=c99',
    ],
    meson_version: '>=0.48.0',
    version: '0.1.0')

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

console_swizzler_dep = declare_dependency(include_directories: include_directories('./include'),
	link_with : console_swizzler)

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
