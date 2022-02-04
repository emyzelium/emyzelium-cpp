demo: demo.cpp ../emyzelium.hpp emyzelium.o 
	rm -f demo
	g++ -o $@ demo.cpp emyzelium.o -lncursesw -lzmq

emyzelium.o: ../emyzelium.hpp ../emyzelium.cpp
	rm -f emyzelium.o
	g++ -o $@ -c ../emyzelium.cpp

clean:
	rm -f demo emyzelium.o