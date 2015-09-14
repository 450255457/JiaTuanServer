EXE = Server			#可执行文件名

all : $(EXE)

$(EXE) :server_threaded.h workqueue.h data_base.h
	#g++ -g -o $(EXE) main.cpp server.cpp public_function.cpp thread_pool.cpp -lpthread -L/usr/lib/x86_64-linux-gnu
	#g++ -g -o httpServer http_server.cpp /usr/lib/libjson_linux-gcc-4.8_libmt.a -levent
	g++ -g -o $(EXE) server_threaded.c workqueue.c data_base.cpp /usr/lib/libjson_linux-gcc-4.8_libmt.a -levent -lpthread -lmysqlclient
#.PHONY表示clean是个伪目标文件
.PHONY : clean
clean : 
	-rm -f $(EXE)
	


