cd /d %~dp0
set /p a=<HAL\COMMON\build_version.h
COPY Objects\NB-IoT_TestTerminal.hex Output\
RENAME Output\NB-IoT_TestTerminal.hex %a:~26,36%.hex

exit
