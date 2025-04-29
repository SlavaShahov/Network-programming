CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread
LDFLAGS = -lssl -lcrypto -lsqlite3

all: server client

server: server.cpp
	$(CXX) $(CXXFLAGS) server.cpp -o server $(LDFLAGS)

client: client.cpp
	$(CXX) $(CXXFLAGS) client.cpp -o client

clean:
	rm -f server client