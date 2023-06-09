
require('.premake-qt/qt')

qt = premake.extensions.qt

newoption {
    trigger = 'to',
    value   = 'path',
    description = 'Set the output location for the generated files',
    default = 'Build'
}

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

    filter 'configurations:Debug32'
        defines { '_DEBUG' }
        qtsuffix 'd'
        targetdir 'Build/bin/Debug32'
        architecture "x86"
    filter 'configurations:Release32'
        defines { 'NDEBUG', 'QT_NO_DEBUG' }
        targetdir 'Build/bin/Release32'
        optimize 'On'
        disablewarnings {
            4127, -- warning C4127: 条件表达式是常量
        }
        architecture "x32"

    filter 'configurations:Debug64'
        defines { '_DEBUG' }
        qtsuffix 'd'
        targetdir 'Build/bin/Debug64'
        architecture "x86"
    filter 'configurations:Release64'
        defines { 'NDEBUG', 'QT_NO_DEBUG' }
        targetdir 'Build/bin/Release64'
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
