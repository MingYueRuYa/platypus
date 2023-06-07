
require('.premake-qt/qt')

qt = premake.extensions.qt

newoption {
    trigger = 'to',
    value   = 'path',
    description = 'Set the output location for the generated files',
    default = 'Build'
}

workspace 'TestWorkSpace'
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

    filter 'configurations:Debug'
        defines { '_DEBUG' }
        qtsuffix 'd'
        targetdir 'bin/Debug'
    filter 'configurations:Release'
        defines { 'NDEBUG', 'QT_NO_DEBUG' }
        targetdir 'bin/Release'
        optimize 'On'
        disablewarnings {
            4127, -- warning C4127: 条件表达式是常量
        }

    -- projects
    -- include 'Tools/premake'
    -- include 'HearthstoneBox/HearthstoneBox'


    group 'Exes'
        include 'platypus'
        -- include 'dolphin'
        -- include 'git_register_exec'

    -- group 'Dlls'
    --     include 'git_plugin'
