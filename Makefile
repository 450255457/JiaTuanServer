all:Client
Client:client_test.cpp
	g++ -g client_test.cpp -o Client
.PHONY : clean
clean : 
	-rm -f client_test