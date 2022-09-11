#----------------------------------------------
#Name: David Onchuru
#SID: 1647809
#CCID: onchuru
#CMPUT 275, Winter 2022
#
#Assignment: Trivial Navigation System(Part 1)
#----------------------------------------------

server: dijkstra.o server.o digraph.o 
	g++ dijkstra.o server.o digraph.o -o server

dijkstra.o: dijkstra.cpp dijkstra.h wdigraph.h digraph.h
	g++ dijkstra.cpp -c

server.o: server.cpp dijkstra.h wdigraph.h digraph.h
	g++ server.cpp -c

digraph.o: digraph.cpp digraph.h
	g++ digraph.cpp -c

clean: 
	rm -f *.o
	rm -f server
	rm -f inpipe
	rm -f outpipe
