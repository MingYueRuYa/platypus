

local to = _OPTIONS['to']

project 'ZERO_CHECK'
    kind 'Utility'

    files {
        '**.lua',
    }

    local f = { _MAIN_SCRIPT }
    for index, value in ipairs(os.matchdirs(_MAIN_SCRIPT_DIR .. '/*')) do
        local p = path.getname(value)
        if p ~= to and p ~= '.git' then
            table.insert(f, path.join(_MAIN_SCRIPT_DIR, p, '**premake5.lua'))
        end
    end

    files (f)

    local commands = { 'cd "$(SolutionDir)../"' }
    local subCommands = { 'call', '.\\premake5.exe', '--file=premake5.lua', '--to=' .. to }

    table.insert(subCommands, 'vs2019')
    table.insert(commands, table.concat(subCommands, ' '))

    checkerenable(true)
    checkerrootfile(_MAIN_SCRIPT)
    checkercommands (commands)

