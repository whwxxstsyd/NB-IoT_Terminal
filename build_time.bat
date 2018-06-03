cd /d %~dp0
cd.>HAL\COMMON\build_time.h
echo #define CMIOT_BUILD_DATE	"%date:~0,4%%date:~5,2%%date:~8,2%">>HAL\COMMON\build_time.h
echo.
set h=%time:~0,2%
echo #define CMIOT_BUILD_TIME	"%h: =0%%time:~3,2%%time:~6,2%">>HAL\COMMON\build_time.h


