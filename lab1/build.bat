windres resource.rc -O coff -o resource.res
g++ -o lab1.exe lab1.cpp resource.res -mwindows -luser32 -lgdi32 -municode
