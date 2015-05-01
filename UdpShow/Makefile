#all: udpShow

#udpShow: 
#	g++-4.9 -fPIC -g -Wall `pkg-config --cflags opencv` udpSocket.cpp udpShow.cpp -o udpShow `pkg-config --libs opencv` -L/usr/local/cuda/lib64/ -lcufft

#clean:
#	rm -rf *.o udpShow

all: udpShow

CFLAGS=-fPIC -g -Wall `pkg-config --cflags opencv`
LIBS = `pkg-config --libs opencv`
INCLUDE = -I/usr/include/libfreenect
FREE_LIBS = -L/usr/lib -lfreenect
CUFFT_LIB = -L/usr/local/cuda/lib64/ -lcufft
OBJS = tinyxml.o udpShow.o udpSocket.o

udpShow: tinyxml.cpp tinyxmlparser.cpp tinyxmlerror.cpp tinystr.cpp udpSocket.cpp udpShow.cpp 
	g++-4.9 $(CFLAGS) $? -o $@  $(LIBS) $(FREE_LIBS) $(CUFFT_LIB)

%.o: %.cpp
	g++-4.9 -c $(CFLAGS) $< -o $@ 
	
clean:
	rm -rf *.o udpShow
