parser: parser.cpp netlist.cpp lut.cpp
	g++ parser.cpp netlist.cpp lut.cpp -o parser -std=c++23 -O2

demo: parser
	./gen_demos.sh
zip: parser demo
	tar -czf macle119_mp1_phase2.tar.gz parser.cpp netlist.cpp lut.cpp netlist.h lut.h gen_demos.sh Makefile README.md
clean:
	rm parser
	rm -rf demo
	rm macle119_mp1_phase2.tar.gz
