all: simple im_discrete

simple: src/*.cpp src/*.h
	g++ -DDISCRETE -g src/simple.cpp -Wall -std=c++17 -O3 src/sfmt/SFMT.c  -o simple

im_discrete: src/*.cpp src/*.h
	g++ -DDISCRETE -g src/main.cpp -Wall -std=c++17 -O3 src/sfmt/SFMT.c  -o im_discrete

im: src/*.cpp src/*.h
	g++ src/main.cpp -Wall -std=c++17 -O3 src/sfmt/SFMT.c  -o im
O0:
	g++ src/main.cpp -Wall -std=c++17 -O0 src/sfmt/SFMT.c  -o im
O1:
	g++ src/main.cpp -Wall -std=c++17 -O1 src/sfmt/SFMT.c  -o im
O2:
	g++ src/main.cpp -Wall -std=c++17 -O2 src/sfmt/SFMT.c  -o im
profile:
	g++ src/main.cpp -Wall -std=c++17 -O2 src/sfmt/SFMT.c  -pg -o im.profile

clean:
	rm -f *.o *~ simple im_discrete
