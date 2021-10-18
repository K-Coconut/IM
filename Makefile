all: im_continuous im_discrete

im_continuous: src/*.cpp src/*.h
	g++ -DCONTINUOUS -g src/main.cpp -Wall -std=c++0x -O3 src/sfmt/SFMT.c  -o im_continuous

im_discrete: src/*.cpp src/*.h
	g++ -DDISCRETE -g src/main.cpp -Wall -std=c++0x -O3 src/sfmt/SFMT.c  -o im_discrete

im: src/*.cpp src/*.h
	g++ src/main.cpp -Wall -std=c++0x -O3 src/sfmt/SFMT.c  -o im
O0:
	g++ src/main.cpp -Wall -std=c++0x -O0 src/sfmt/SFMT.c  -o im
O1:
	g++ src/main.cpp -Wall -std=c++0x -O1 src/sfmt/SFMT.c  -o im
O2:
	g++ src/main.cpp -Wall -std=c++0x -O2 src/sfmt/SFMT.c  -o im
profile:
	g++ src/main.cpp -Wall -std=c++0x -O2 src/sfmt/SFMT.c  -pg -o im.profile

cont_profile:
	g++ -DCONTINUOUS src/main.cpp -Wall -std=c++0x -O3 src/sfmt/SFMT.c  -o im_continuous -pg -o im_continuous.profile

clean:
	rm -f *.o *~ im_continuous im_discrete
