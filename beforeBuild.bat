cd /d %~dp0
cd.>HAL\COMMON\build_version.h
set h=%time:~0,2%

echo #define __CMIOT_VERSION__ "D5200 V1.0 Build%date:~0,4%%date:~5,2%%date:~8,2% Rel.%h: =0%%time:~3,2%%time:~6,2%">>HAL\COMMON\build_version.h

del Output\*.hex /s
del Output\*.bin /s

exit
