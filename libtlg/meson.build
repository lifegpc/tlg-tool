sources = files(
    'handlestream.cpp',
    'handlestream.h',
    'LoadTLG.cpp',
    'memstream.cpp',
    'SaveTLG.cpp',
    'slide.cpp',
    'slide.h',
    'stream.cpp',
    'stream.h',
    'tjs.h',
    'TLG.h',
    'TLG5Saver.cpp',
    'TLG6BS.h',
    'TLG6Saver.cpp',
    'tvpgl.c',
    'tvpgl.h',
)

tlg = library('tlg',
    sources,
    include_directories: include_directories('.'),
)
tlg_dep = declare_dependency(
    link_with: tlg,
    include_directories: include_directories('.'),
)
