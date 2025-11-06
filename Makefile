parser: parser.cpp netlist.cpp lut.cpp
	g++ parser.cpp netlist.cpp lut.cpp -o parser -std=c++20
clean:
	rm parser
