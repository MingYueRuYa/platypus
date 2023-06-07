
require('.premake-qt/qt')

qt = premake.extensions.qt

workspace 'Platypus'
    configurations { 'Debug', 'Release' }
    location ( _OPTIONS['to'] )
    toolset 'v142'
    symbols 'Full'
    architecture 'x86'
    systemversion "10.0.19041.0"
    cppdialect 'C++20'

    dependson { 'ZERO_CHECK' }
    startproject 'platypus'

    -- Qt config
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

    filter 'configurations:Debug'
        defines { '_DEBUG' }
        qtsuffix 'd'
        targetdir 'Build/bin/Debug'
    filter 'configurations:Release'
        defines { 'NDEBUG', 'QT_NO_DEBUG' }
        targetdir 'Build/bin/Release'
        optimize 'On'
        disablewarnings {
            4127, -- warning C4127: 条件表达式是常量
        }

    group 'App'
        include 'platypus'
        include 'dolphin'
        include 'git_register_exec'

    group 'dlls'
        include 'git_plugin'
