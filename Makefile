CXXSERVERSOURCES = server.cpp
CXXCLIENTSOURCES = client.cpp
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

all: server client

server: $(CXXSERVERSOURCES)
	$(CXX) $(CXXFLAGS) $(CXXSERVERSOURCES) -o screen-worms-server

client: $(CXXCLIENTSOURCES)
	$(CXX) $(CXXFLAGS) $(CXXCLIENTSOURCES) -o screen-worms-client

.PHONY: clean
clean:
	rm -rf *.o screen-worms-server
	rm -rf *.o screen-worms-client