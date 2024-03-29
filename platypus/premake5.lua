
project 'Platypus'
    kind 'WindowedApp'
    language 'C++'
    targetname 'Platypus'
    -- largeaddressaware 'on'
    icon './res/64.ico'

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
            'Platypus.rc',
            'resource.h'
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

    -- 依赖于x64架构下的 git_register_exec
    dependson { 'git_register_exec' }

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
        path.getabsolute('3rdparty/breakpad/install_windows/include', _MAIN_SCRIPT_DIR),
    }



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
        libdirs {
            path.getabsolute('3rdparty/breakpad/install_windows/lib/Debug/$(Platform)', _MAIN_SCRIPT_DIR),
        }
        links { 'qtmaind.lib','exception_handler.lib','common.lib','crash_generation_client.lib'}
        targetdir ('../'..top_debug_target_dir)


    filter 'configurations:Release*'
        defines { 'QT_NO_DEBUG' }
        libdirs {
            path.getabsolute('3rdparty/breakpad/install_windows/lib/Release/$(Platform)', _MAIN_SCRIPT_DIR),
        }

        links { 'qtmaind.lib','exception_handler.lib','common.lib','crash_generation_client.lib'}
        targetdir ('../'..top_release_target_dir)

