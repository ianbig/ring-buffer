CC = g++
CFLAGS = --std=c++14 -g

OBJS = CircularBuffer.o

ring_buffer: CircularBuffer.o api_test.cc
	$(CC) -o $@ $(CFLAGS) $(OBJS) api_test.cc
CircularBuffer.o: CircularBuffer.cc CircularBuffer.hpp
	$(CC) -c $(CFLAGS) $<