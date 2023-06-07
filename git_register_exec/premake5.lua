
project 'git_register_exec'
    kind 'WindowedApp'
    language 'C++'
    targetname 'git_register_exec'
    -- largeaddressaware 'on'

    warnings 'Default'
    removeflags 'FatalWarnings'

    disablewarnings {
    }

    -- warning LNK4099: 未找到 PDB“vc120.pdb”
    linkoptions { '/ignore:4099' }

    vpaths {
        ['Resources/Language/*'] = {
            'Resources/Language/**.*',
        },
        ['Resources/UI/*'] = {
            'Resources/UI/**.*',
        },
        ['Resources/*'] = {
            'Resources/*.*',
            '*.ico',
            '*.icon',
            '*.rc'
        },

        ['Source Files/*'] = {
            'stdafx.*',
            '*.cpp',
            '*.h',
        },
        -- 移除默认筛选器
        ['Header Files/*'] = {},
        ['UI Files/*'] = {},
    }

    files {
        '../common/**.*',
        '**.h',
        '**.cpp',
        '**.ui',
        '**.ts',
        '**.manifest',
        '**.ico',
        '**.pro',
        '**.qrc',
        '**.rc',
    }
    -- pchheader 'stdafx.h'
    -- pchsource 'stdafx.cpp'

    -- removefiles {
    -- }

    defines {
        '_SCL_SECURE_NO_WARNINGS',
        'SPDLOG_WCHAR_TO_UTF8_SUPPORT',
        'SPDLOG_TRACE_ON',
        'WIN32'
    }

    includedirs {
        path.getabsolute('3rdparty/json/', _MAIN_SCRIPT_DIR),
        path.getabsolute('common/', _MAIN_SCRIPT_DIR),
        path.getabsolute('3rdparty/spdlog/include', _MAIN_SCRIPT_DIR),
    }

    -- libdirs {
    -- }

    -- links {
    -- }

    linkoptions {
    }

    dependson { 'git_plugin' }

    filter 'configurations:Debug'
        links { 'git_plugin.lib' }
        libdirs {
            path.getabsolute('build/bin/Debug', _MAIN_SCRIPT_DIR),
        }

    filter 'configurations:Release'
        links { 'git_plugin.lib' }
        libdirs {
            path.getabsolute('build/bin/Release', _MAIN_SCRIPT_DIR),
        }
