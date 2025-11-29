windres resource.rc -O coff -o resource.res
windres resourceClient.rc -O coff -o resourceClient.res
g++ -o serv.exe serv.cpp resource.res -mwindows -luser32 -lgdi32 -lws2_32 -municode
g++ -o client1.exe client1.cpp resourceClient.res -mwindows -luser32 -lgdi32 -lws2_32 -municode
g++ -o client2.exe client2.cpp resourceClient.res -mwindows -luser32 -lgdi32 -lws2_32 -municode
