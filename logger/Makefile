CXXFLAGS = -I../driver
LDFLAGS = -L../driver/lib/x86_64 -lmvDeviceManager

all: main.cpp
	g++ -o logger $^ $(CXXFLAGS) $(LDFLAGS)

clean:
	rm logger
