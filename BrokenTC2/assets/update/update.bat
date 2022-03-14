@REM  Broken The Crew 2 sequential clutch assist
@REM  Copyright (C) 2022 BrokenGameNoob <brokengamenoob@gmail.com>

@REM  This program is free software: you can redistribute it and/or modify
@REM  it under the terms of the GNU General Public License as published by
@REM  the Free Software Foundation, either version 3 of the License, or
@REM  (at your option) any later version.

@REM  This program is distributed in the hope that it will be useful,
@REM  but WITHOUT ANY WARRANTY; without even the implied warranty of
@REM  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
@REM  GNU General Public License for more details.

@REM  You should have received a copy of the GNU General Public License
@REM  along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
