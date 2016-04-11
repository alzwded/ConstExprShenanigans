@echo off
SETLOCAL

IF #%1 == # GOTO :NOFILE
SET testName=%1

IF #%2 == # GOTO :EXEC
IF #%2 == #noinfo GOTO :SILENT

IF #%3 NEQ # GOTO :BADCANRY

SET ADDOPTS=-DPRINT_INFO
GOTO :EXEC

:SILENT
GOTO :EXEC

:EXEC

SET cppName=bits/test_rt.cpp

g++ --std=gnu++14 -g -I. -I./bits -DTEST=%testName% %ADDOPTS% %cppName% && a

GOTO :EOF

:NOFILE
ECHO Should give a test number, e.g. 1
EXIT /B 255

:BADCANRY
ECHO Don't know what to do with so many parameters
EXIT /B 255
