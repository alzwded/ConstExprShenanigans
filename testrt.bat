@echo off
SETLOCAL

IF #%1 == # GOTO :NOFILE
SET testName=%1

IF #%2 == # GOTO :LOUD
IF #%2 == #noinfo GOTO :SILENT

GOTO :BADCANRY
IF #%3 NEQ # GOTO :BADCANRY

:LOUD
SET ADDOPTS=-DPRINT_INFO
GOTO :EXEC

:SILENT
GOTO :EXEC

:EXEC

SET cppName=bits/test_rt.cpp

g++ --std=gnu++14 -g -I. -I./bits -DTEST=%testName% %ADDOPTS% %cppName% && a

GOTO :EOF

:NOFILE
ECHO testrt ^<num^> [noinfo]
EXIT /B 255

:BADCANRY
ECHO testrt ^<num^> [noinfo]
EXIT /B 255
