
project 'git_plugin'
    kind 'SharedLib'
    language 'C++'
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

        ['Common/common_h/*'] = {
            '../common/**.h',
        },
        ['Common/common_cpp/*'] = {
            '../common/**.cpp',
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

    libdirs {
        path.getabsolute('build/bin/', _MAIN_SCRIPT_DIR),
    }



    -- links {
    -- }

    linkoptions {
    }


    filter 'configurations:*32'
        targetname 'git_plugin'


    filter 'configurations:*64'
        targetname 'git_plugin_x64'
        
    filter 'configurations:Debug64'
        targetdir '../Build/bin/Debug32'

    filter 'configurations:Release64'
        targetdir '../Build/bin/Release32'
