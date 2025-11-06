parser: parser.cpp netlist.cpp lut.cpp
	g++ parser.cpp netlist.cpp lut.cpp -o parser -std=c++23 -O2
clean:
	rm parser
