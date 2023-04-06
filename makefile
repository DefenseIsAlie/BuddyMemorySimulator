
buddy.out:	buddy.cpp
	g++ -O3 $^ -o $@

debug: 
	g++ buddy.cpp -o buddy.out -g

clean:
	rm -rf *.out