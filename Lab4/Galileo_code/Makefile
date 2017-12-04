
main: net.c net.h common.h main.c sensor.cpp sensor.h
	g++ -g main.c net.c sensor.cpp `pkg-config opencv --cflags --libs` -o main -lmraa -pthread -lcurl

