/* rgrep.cmd -- List all lines of a file matching a regexp, with highligting
   Copyright (c) 1994 by Eberhard Mattes */

call RxFuncAdd 'RegexxLoadFuncs', 'regexx', 'RegexxLoadFuncs'
call RegexxLoadFuncs

/* Control sequences for starting and ending hilighting, respectively. */

hs = '1b'x||'[1m'
he = '1b'x||'[0m'

parse arg regexp fname rest

if (regexp == '') | (fname == '') | (rest \== '') then do
  say 'Usage: rgrep <regexp> <file>'
  exit 1
end

if stream(fname, 'C', 'QUERY EXISTS') == '' then do
  say fname||': not found'
  exit 2
end

do while lines(fname)
  line = linein(fname)
  if RegexxStart(regexp, line) \== 0 then
    say RegexxReplaceAll(regexp, hs||'\0'||he, line)
end
