project(
    'botball-gamestate',
    'cpp',
    version : '1.0.0',
    default_options : [
        'cpp_std=c++20',
        'cpp_extensions=false',
        'buildtype=release'
    ]
)

cpp_compiler = meson.get_compiler('cpp')

# Dependencies
spdlog_dep = cpp_compiler.find_library('spdlog', required : true)
fmt_dep    = cpp_compiler.find_library('fmt', required : true)

# Source files
src_files = files(
    'include/gamestate.h'
)

impl_sources = files(
    run_command('bash', '-c', 'ls impl/*.cpp', check : true).stdout().strip().split()
)

all_sources = src_files + impl_sources

# Include directories
inc_dirs = include_directories(
    'include',
    'lib'
)

# Static library
gamestate_lib = static_library(
    'botball-gamestate',
    all_sources,
    include_directories : inc_dirs,
    dependencies : [
        spdlog_dep,
        fmt_dep
    ],
)

# Executables
bot_a = executable(
    'bot_a',
    'bot_a.cpp',
    link_with : gamestate_lib,
    include_directories : inc_dirs
)

bot_b = executable(
    'bot_b',
    'bot_b.cpp',
    link_with : gamestate_lib,
    include_directories : inc_dirs
)