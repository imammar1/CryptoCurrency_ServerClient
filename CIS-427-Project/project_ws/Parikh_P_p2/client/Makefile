# Makefile for client (works on umich server as well as my)
client:
	g++ main.cpp -std=c++17 -I $(BOOST_ROOT) -L $(BOOST_LIB) -lpthread -ldl -o client.out

clean:
	rm client.out
