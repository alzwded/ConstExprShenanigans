@echo off
SETLOCAL

IF #%1 == # GOTO :NOFILE
SET testName=%1

IF #%2 NEQ # GOTO :BADCANRY

SET cppName=test_rt.cpp

g++ --std=gnu++14 -g -DTEST=%testName% %cppName% && a

GOTO :EOF

:NOFILE
ECHO Should give a test number, e.g. 1
EXIT /B 255

:BADCANRY
ECHO Don't know what to do with so many parameters
EXIT /B 255
