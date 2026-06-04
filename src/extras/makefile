PORT = /dev/cu.usbmodem1401 

PrintRoom: printRoom.cpp
	g++ -Wno-writable-strings -o PrintRoom printRoom.cpp

run: PrintRoom
	./PrintRoom $(PORT)
clean:
	${RM} PrintRoom
