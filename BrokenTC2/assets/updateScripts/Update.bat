@echo off
setlocal

set EXE_BASE_NAME=BrokenTC2
set UPDATE_TAG_FILENAME=UPDATED.TAG
set EXTENSION_EXE=.exe
set EXTENSION_UPDATE=.update
@REM How long it will take to fail update
set /A "TIMEOUT_TIME=25"


set EXE_NAME=%EXE_BASE_NAME%%EXTENSION_EXE%
set UPDATE_NAME=%EXE_BASE_NAME%%EXTENSION_UPDATE%

del %EXE_NAME%

set /A "cnt=0"
:loopRmExe
set /A "cnt=cnt+1"
PING localhost -n 2 >NUL
echo %cnt% ===== %TIMEOUT_TIME%

if %cnt% GEQ %TIMEOUT_TIME% goto endOfFile_error
@REM else
if exist %EXE_NAME% (
:retryDelete
    del %EXE_NAME%
    set errCode=errorlevel
    echo Error = %errCode%
    if not errorlevel 0 goto exitLoopRm
    @REM if error
    echo "ERROR FOUD ------------"
    goto loopRmExe
)
:exitLoopRm

rename %UPDATE_NAME% %EXE_NAME%
if errorlevel 1 goto endOfFile_error

echo .>%UPDATE_TAG_FILENAME%

start %EXE_NAME%

del "%~f0"
:endOfFile_error
