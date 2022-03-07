@REM @echo off

@REM Rem copy .\build-SimpleUpdater-Desktop_Qt_6_2_3_MinGW_64_bit-Release\TESTv0.0.0.exe .\DeployDir
@REM Rem cd DeployDir


@REM set  /A matchCount=0
@REM for /f "usebackq delims=" %%f in (`dir /b .\ 2^>NUL ^| findstr /r "build-.*_Qt_6.*MinGW_64.*-Release"`) do (
@REM     echo Match: %%f
@REM     set /a matchCount= matchCount + 1

@REM )
@REM echo "Match count = %matchCount%"

@REM Rem C:\Qt\6.2.3\mingw_64\bin\windeployqt.exe . TESTv0.0.0.exe

@REM Rem FUNCTIONS -------------------------------------------------------------------

@REM :ErrorOccured
@REM echo "An error occured :"
@REM echo %1


@REM Rem OUTPUT PROG -----------------------------------------------------------------

python deploy.py > deployLog.txt