parser: parser.cpp netlist.cpp
	g++ parser.cpp netlist.cpp -o parser -std=c++20
clean:
	rm parser
