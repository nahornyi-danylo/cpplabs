#! /bin/env bash
x86_64-w64-mingw32-windres resource.rc -O coff -o resource.res
x86_64-w64-mingw32-windres resourceClient.rc -O coff -o resourceClient.res
x86_64-w64-mingw32-windres resourceClient3.rc -O coff -o resourceClient3.res
x86_64-w64-mingw32-g++ -o serv.exe serv.cpp resource.res -mwindows -luser32 -lgdi32 -lws2_32 -municode
x86_64-w64-mingw32-g++ -o client1.exe client1.cpp resourceClient.res -mwindows -luser32 -lgdi32 -lws2_32 -municode
x86_64-w64-mingw32-g++ -o client2.exe client2.cpp resourceClient.res -mwindows -luser32 -lgdi32 -lws2_32 -municode
x86_64-w64-mingw32-g++ -o client3.exe client3.cpp resourceClient3.res -mwindows -luser32 -lgdi32 -lws2_32 -municode
