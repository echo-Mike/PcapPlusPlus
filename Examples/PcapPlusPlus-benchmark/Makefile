include /usr/local/etc/PcapPlusPlus.mk

all:
	@$(CXX) $(PCAPPP_LIBRARY_WIDE_CXXFLAGS) $(PCAPPP_INCLUDES)  -O2 -std=c++0x -c -o benchmark.o benchmark.cpp
	@$(CXX) $(PCAPPP_LIBRARY_WIDE_CXXFLAGS) $(PCAPPP_INCLUDES)  -o benchmark benchmark.o $(PCAPPP_LIBS)

clean:
	rm benchmark.o
	rm benchmark
