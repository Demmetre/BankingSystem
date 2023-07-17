CXX = g++
CXXFLAGS = -std=c++11 -Wall
LDFLAGS = -lcrypto

SERVER_TARGET = server
CLIENT_TARGET = client

SERVER_SRC = Server.cpp Storage.cpp
CLIENT_SRC = Client.cpp

SERVER_OBJ = $(SERVER_SRC:.cpp=.o)
CLIENT_OBJ = $(CLIENT_SRC:.cpp=.o)

.PHONY: all clean

all: $(SERVER_TARGET) $(CLIENT_TARGET)

$(SERVER_TARGET): $(SERVER_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(CLIENT_TARGET): $(CLIENT_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(SERVER_TARGET) $(CLIENT_TARGET) $(SERVER_OBJ) $(CLIENT_OBJ)
