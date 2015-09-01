all:Client
Client:client_test.cpp
	g++ -g client_test.cpp -o Client /usr/lib/libjson_linux-gcc-4.8_libmt.a
.PHONY : clean
clean : 
	-rm -f client_test
