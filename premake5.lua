
require('.premake-qt/qt')

qt = premake.extensions.qt

newoption {
    trigger = 'to',
    value   = 'path',
    description = 'Set the output location for the generated files',
    default = 'Build'
}

top_debug_target_dir='Build/bin/Debug'
top_release_target_dir='Build/bin/Release'

print(top_debug_target_dir)

workspace 'Platypus'
    configurations { "Debug32", "Release32", "Debug64", "Release64" }
    location ( _OPTIONS['to'] )
    toolset 'v142'
    symbols 'Full'
    -- architecture 'x86'
    systemversion "10.0.19041.0"
    cppdialect 'C++20'

    -- dependson { 'ZERO_CHECK' }
    startproject 'Platypus'

    disablewarnings {
        4006, -- __NULL_IMPORT_DESCRIPTOR already defined
        4091, -- ignored on left of 'typedef struct' when no variable is declared
        4127, -- conditional expression is constant
        4221, -- This object file does not define any previously undefined public symbols, so it will not be used by any link operation that consumes this library
        4239, -- Warning C4239   nonstandard extension used: 'argument': conversion from 'QString' to 'QString &'
        4996, -- The POSIX name for this item is deprecated. Instead, use the ISO C and C++ conformant name: xxx
        5240, -- 'nodiscard': attribute is ignored in this syntactic position
    }

    -- Qt config
    -- qtpath (abs 'f:/qt5.15.2')
    qtpath ('f:/qt5.15.2/bin')
    qtprefix 'Qt5'

    -- global macros
    defines {
        'WIN32',
        '_WINDOWS',
    }

    flags {
        'MultiProcessorCompile',
    }

    qtmocargs {
        -- '-I"../playpus/3rd/include"'
    }

    filter 'configurations:Debug32'
        defines { '_DEBUG' }
        qtsuffix 'd'
        targetdir ("${top_debug_target_dir}")
        architecture "x86"
    filter 'configurations:Release32'
        defines { 'NDEBUG', 'QT_NO_DEBUG' }
        targetdir ("${top_release_target_dir}")
        optimize 'On'
        disablewarnings {
            4127, -- warning C4127: 条件表达式是常量
        }
        architecture "x86"

    filter 'configurations:Debug64'
        defines { '_DEBUG' }
        qtsuffix 'd'
        targetdir ("${top_debug_target_dir}")
        architecture "x64"
    filter 'configurations:Release64'
        defines { 'NDEBUG', 'QT_NO_DEBUG' }
        targetdir ("${top_release_target_dir}")
        optimize 'On'
        disablewarnings {
            4127, -- warning C4127: 条件表达式是常量
        }
        architecture "x64"

    group 'App'
        include 'platypus'
        include 'dolphin'
        include 'git_register_exec'

    group 'dlls'
        include 'git_plugin'
