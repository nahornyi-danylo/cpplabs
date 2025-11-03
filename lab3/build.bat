windres resource.rc -O coff -o resource.res
g++ -o lab3.exe lab3.cpp resource.res -mwindows -luser32 -lgdi32 -municode
