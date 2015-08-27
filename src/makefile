EXE = Server			#可执行文件名

all : $(EXE)

$(EXE) :http_server.cpp 
	#g++ -g -o $(EXE) main.cpp server.cpp public_function.cpp thread_pool.cpp -lpthread -L/usr/lib/x86_64-linux-gnu
	g++ -g -o httpServer http_server.cpp /usr/lib/libjson_linux-gcc-4.8_libmt.a -levent	
#.PHONY表示clean是个伪目标文件
.PHONY : clean
clean : 
	-rm -f $(EXE)
	


