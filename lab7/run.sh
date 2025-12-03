#! /bin/env bash

WINEDEBUG=-all WINEPATH="Z:\\usr\\x86_64-w64-mingw32\\bin" wine ./client2.exe &
WINEDEBUG=-all WINEPATH="Z:\\usr\\x86_64-w64-mingw32\\bin" wine ./client1.exe &
WINEDEBUG=-all WINEPATH="Z:\\usr\\x86_64-w64-mingw32\\bin" wine ./serv.exe
