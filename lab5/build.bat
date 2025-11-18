windres resource.rc -O coff -o resource.res
windres resourceClient.rc -O coff -o resourceClient.res
windres resourceClient3.rc -O coff -o resourceClient3.res
g++ -o serv.exe serv.cpp resource.res -mwindows -luser32 -lgdi32 -lws2_32 -municode
g++ -o client1.exe client1.cpp resourceClient.res -mwindows -luser32 -lgdi32 -lws2_32 -lwininet -municode
g++ -o client2.exe client2.cpp resourceClient.res -mwindows -luser32 -lgdi32 -lws2_32 -municode
g++ -o client3.exe client3.cpp resourceClient3.res -mwindows -luser32 -lgdi32 -lws2_32 -municode
