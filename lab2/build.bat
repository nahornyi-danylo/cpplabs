windres resource.rc -O coff -o resource.res
g++ -o lab2.exe lab2.cpp graph.cpp -mwindows -luser32 -lgdi32 -municode
