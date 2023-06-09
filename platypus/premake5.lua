
project 'Platypus'
    kind 'WindowedApp'
    language 'C++'
    targetname 'Platypus'
    -- largeaddressaware 'on'

    warnings 'Default'
    removeflags 'FatalWarnings'

    disablewarnings {
    }

    -- warning LNK4099: 未找到 PDB“vc120.pdb”
    linkoptions { '/ignore:4099' }

    vpaths {
        ['Kernel/*'] = {
            'Kernel/**.*',
        },
        ['Resources/Language/*'] = {
            'Resources/Language/**.*',
        },
        ['Resources/UI/*'] = {
            'Resources/UI/**.*',
        },
        ['Resources/*'] = {
            'Resources/*.*',
        },
        ['Source Files/*'] = {
            'stdafx.*',
            'main.*',
        },
        ['qxtglobalshortcut5/shortcut_h/*'] = {
            '../3rdparty/qxtglobalshortcut5/core/**.h',
            '../3rdparty/qxtglobalshortcut5/gui/*.h',
            '../3rdparty/qxtglobalshortcut5/gui/win/**.h',
        },
        ['qxtglobalshortcut5/shortcut_cpp/*'] = {
            '../3rdparty/qxtglobalshortcut5/core/**.cpp',
            '../3rdparty/qxtglobalshortcut5/gui/*.cpp',
            '../3rdparty/qxtglobalshortcut5/gui/win/**.cpp',
        },
        ['Common/common_h/*'] = {
            '../common/**.h',
        },
        ['Common/common_cpp/*'] = {
            '../common/**.cpp',
        },

        -- 移除默认筛选器
        ['Header Files/*'] = {},
        ['UI Files/*'] = {},
    }

    files {
        '../common/**.*',
        '../3rdparty/qxtglobalshortcut5/core/**.*',
        '../3rdparty/qxtglobalshortcut5/gui/*.h',
        '../3rdparty/qxtglobalshortcut5/gui/*.cpp',
        '../3rdparty/qxtglobalshortcut5/gui/win/**.*',
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
        'BUILD_QXT_CORE',
        'BUILD_QXT_GUI',
        'WIN32'
    }

    includedirs {
        path.getabsolute('3rdparty/spdlog/include', _MAIN_SCRIPT_DIR),
        path.getabsolute('3rdparty/json/', _MAIN_SCRIPT_DIR),
        path.getabsolute('common/', _MAIN_SCRIPT_DIR),
        path.getabsolute('include/', _MAIN_SCRIPT_DIR),
        path.getabsolute('3rdparty/qxtglobalshortcut5/core/', _MAIN_SCRIPT_DIR),
        path.getabsolute('3rdparty/qxtglobalshortcut5/gui/', _MAIN_SCRIPT_DIR),
    }

    -- libdirs {
    -- }

    -- links {
    -- }

    linkoptions {
        '/MANIFESTUAC:"level=\'requireAdministrator\' uiAccess=\'false\'"'
    }

    qt.enable()
    qtmodules {
        'core',
        'gui',
        'xml',
        'widgets',
    }

    filter 'configurations:Debug*'
        links { 'qtmaind.lib' }

    filter 'configurations:Release*'
        defines { 'QT_NO_DEBUG' }
        links { 'qtmain.lib' }

