project('tlg-tools', 'c', 'cpp')

MSVC = meson.get_compiler('c').get_id() == 'msvc'
if MSVC
    add_project_arguments('/utf-8', language: 'c')
    add_project_arguments('/utf-8', language: 'cpp')
endif

subdir('libtlg')

zlib = subproject('zlib')
libpng = subproject('libpng')
utils = subproject('utils')

zlib_dep = zlib.get_variable('zlib_dep')
libpng_dep = libpng.get_variable('libpng_dep')
utils_dep = utils.get_variable('utils_dep')

deps = [zlib_dep, libpng_dep, utils_dep, tlg_dep]

if MSVC
    getopt = subproject('getopt')
    getopt_dep = getopt.get_variable('getopt_dep')
    deps += getopt_dep
endif

executable('tlg',
    files(
        'src/main.cpp',
        'src/file_stream.cpp',
        'src/file_stream.h',
        'src/dict_file.cpp',
        'src/dict_file.h',
    ),
    dependencies: deps,
)
