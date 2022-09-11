#----------------------------------------------
#Name: David Onchuru
#SID: 1647809
#CCID: onchuru
#CMPUT 275, Winter 2022
#
#Assignment: Trivial Navigation System(Part 2)
#----------------------------------------------

run:
	make -C server
	gnome-terminal -- ./server/server
	gnome-terminal -- python3 client/client.py
clean:
	rm -f *.o
	rm -f inpipe
	rm -f outpipe
	rm -f client/inpipe
	make -C server clean