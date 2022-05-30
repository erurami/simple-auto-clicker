
@echo off

rc resource.rc

cl main.cpp resource.res /W4 /std:c++17 /EHsc /Fe:main.exe /link /subsystem:windows

mt -manifest main.exe.manifest -outputresource:main.exe;1

