demo: demo.cpp ../emyzelium.hpp emyzelium.o 
	rm -f demo
	g++ -o demo demo.cpp emyzelium.o -lncursesw -lzmq

demo-customlib: demo.cpp ../emyzelium.hpp emyzelium.o 
	rm -f demo-customlib
	g++ -o demo-customlib demo.cpp emyzelium.o -lncursesw -Wl,-rpath,./lib -L./lib -lzmq

emyzelium.o: ../emyzelium.hpp ../emyzelium.cpp
	rm -f emyzelium.o
	g++ -o $@ -c ../emyzelium.cpp

clean:
	rm -f demo demo-customlib emyzelium.o