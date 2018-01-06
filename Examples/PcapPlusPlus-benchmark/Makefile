include /usr/local/etc/PcapPlusPlus.mk

all:
	@$(CXX) $(PCAPPP_INCLUDES) -std=$(CXX_STANDARD) -O2 -std=c++0x -c -o benchmark.o benchmark.cpp
	@$(CXX) $(PCAPPP_INCLUDES) -std=$(CXX_STANDARD) -o benchmark benchmark.o $(PCAPPP_LIBS)

clean:
	rm benchmark.o
	rm benchmark
