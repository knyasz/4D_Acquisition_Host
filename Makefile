all: test

CFLAGS=-fPIC -g -Wall `pkg-config --cflags opencv`
LIBS = `pkg-config --libs opencv`
INCLUDE = -I/usr/include/libfreenect
FREE_LIBS = -L/usr/lib -lfreenect
CUFFT_LIB = -L/usr/local/cuda/lib64/ -lcufft
OBJS = udpShow.o udpSocket.o

test:  test.cpp
	g++-4.9 $(CFLAGS) $? -o $@  $(LIBS) $(FREE_LIBS) $(CUFFT_LIB)

%.o: %.cpp
	g++-4.9 -c $(CFLAGS) $< -o $@
	
clean:
	rm -rf *.o test
