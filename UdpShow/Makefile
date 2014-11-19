all: udpShow

udpShow: 
	g++-4.9 -fPIC -g -Wall `pkg-config --cflags opencv` udpSocket.cpp udpShow.cpp -o udpShow `pkg-config --libs opencv` -L/usr/local/cuda/lib64/ -lcufft

clean:
	rm -rf *.o udpShow
