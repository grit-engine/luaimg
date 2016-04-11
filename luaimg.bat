:: Copyright (c) 2013 Dave Cunningham and the Grit Game Engine project, licensed under the MIT license.

:: The purpose of this script is to provide something that can be associated with the lua files
:: in windows, for running them in luaimg.

@cd "%~dp1"
@"%~dp0luaimg.exe" -F "%1"

IF ERRORLEVEL 1 pause
