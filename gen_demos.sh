DIR="demo"
if [ -d  "$DIR" ]; then
	rm -r demo
fi
mkdir demo
echo "delays time:" > timed.txt
(time ./parser read_nldm delays Benchmarks/NLDM_lib_max2Inp.lib) 2>> timed.txt
echo "slews time:" >> timed.txt
(time ./parser read_nldm slews Benchmarks/NLDM_lib_max2Inp.lib) 2>> timed.txt
mv slew_LUT.txt delay_LUT.txt timed.txt demo
for path in Benchmarks/Benchmarks/*.isc; do
	FILE=$(basename "$path")
	echo "details time:" > timed.txt
	(time ./parser "read_ckt" "Benchmarks/Benchmarks/${FILE}") 2>> timed.txt
	echo "traversal time:" >> timed.txt
	(time ./parser "Benchmarks/NLDM_lib_max2Inp.lib" "Benchmarks/Benchmarks/${FILE}") 2>> timed.txt
	mkdir "demo/${FILE}_demo"
	mv ckt_details.txt ckt_traversal.txt timed.txt "demo/${FILE}_demo"
done

