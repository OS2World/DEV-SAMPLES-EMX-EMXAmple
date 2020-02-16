/* flag.cmd */

parse arg s1 s2 rest

call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs

select
  when s1 = 'install' then
    do
      if s2 \= '' then
        call usage
      name = stream('flag.dll', 'c', 'query exists')
      if name == '' then
        do
          say '*** flag.dll not in current working directory.'
          exit 2
        end
      call SysRegisterObjectClass 'Flag', name
    end

  when s1 = 'uninstall' then
    do
      if s2 \= '' then
        call usage
      if SysDeregisterObjectClass('Flag') then
        say 'Flag class uninstalled successfully.'
      else
        do
          say '*** Uninstallation of Flag class failed.'
          exit 2
        end
    end

  when s1 = 'create' then
    do
      if s2 == '' | rest \= '' then
        call usage
      if SysCreateObject('Flag', s2, '<WP_DESKTOP>') then
        say 'Flag object' s2 'created successfully.'
      else
        do
          say '*** Creation of Flag object' s2 'failed.'
          exit 2
        end
    end

  otherwise
    call usage
end
exit 0

usage:
  say 'Usage:'
  say ''
  say 'flag install                 Install Flag class'
  say 'flag uninstall               Remove Flag class'
  say 'flag create <name>           Create Flag object NAME'
  exit 1
