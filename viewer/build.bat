@echo off
echo viewer Build Script

REM wxWidgetsパスの設定
set "WXWIDGETS_ROOT=C:\wxWidgets"

if not exist "%WXWIDGETS_ROOT%" (
    echo wxWidgets not found. Please install wxWidgets.
    pause
    exit /b 1
)

echo Found wxWidgets at: %WXWIDGETS_ROOT%

REM MSBuildでビルド
echo Building with MSBuild...

REM Developer Command Promptの環境を設定
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
if errorlevel 1 (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
)

msbuild viewer.sln /p:Configuration=Release /p:Platform=x64 /p:WXWIDGETS_ROOT="%WXWIDGETS_ROOT%"

if %ERRORLEVEL% neq 0 (
    echo Build failed. Check wxWidgets installation and Visual Studio setup.
    pause
    exit /b 1
)

echo Build completed successfully!
echo Executable: x64\Release\viewer.exe
pause