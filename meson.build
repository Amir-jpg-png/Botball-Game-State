project(
    'botball-gamestate',
    'cpp',
    version : '1.0.0',
    default_options : ['cpp_std=c++20', 'buildtype=release']
)

cpp_compiler = meson.get_compiler('cpp')
spdlog_dep = cpp_compiler.find_library('spdlog', required : true)
fmt_dep    = cpp_compiler.find_library('fmt', required : true)

# Source files (using files() for all to keep it clean)
all_sources = files(
    'include/GameState.h',
    'impl/Client.cpp',
    'impl/GameState.cpp',
    'impl/Phase.cpp',
    'impl/PhaseState.cpp',
    'impl/Server.cpp',
    'impl/Socket.cpp',
    'impl/TableState.cpp',
    'impl/Util.cpp'
)

inc_dirs = include_directories('include', 'lib')

# Static library - ADDED 'install : true'
gamestate_lib = static_library(
    'botball-gamestate',
    all_sources,
    include_directories : inc_dirs,
    dependencies : [spdlog_dep, fmt_dep],
    install : true
)

# Install ALL headers to /usr/include/botball/
install_headers(
    'include/Client.h', 'include/GameState.h', 'include/Phase.h', 
    'include/PhaseState.h', 'include/Server.h', 'include/Socket.h', 
    'include/TableState.h', 'include/Util.h',
    subdir : 'botball'
)

# Generate the pkg-config file so other apps can find it
pkg = import('pkgconfig')
pkg.generate(
    gamestate_lib,
    name : 'botball-gamestate',
    description : 'Botball GameState Library',
    version : '1.0.0',
    subdirs : 'botball'
)

# Executables (If you want these in /usr/bin, add install : true)
executable('bot_a', 'bot_a.cpp', link_with : gamestate_lib, include_directories : inc_dirs, install : true)
executable('bot_b', 'bot_b.cpp', link_with : gamestate_lib, include_directories : inc_dirs, install : true)
