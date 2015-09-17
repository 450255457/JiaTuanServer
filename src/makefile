EXE = Server			#可执行文件名

all : $(EXE)

$(EXE) :server_threaded.h workqueue.h dbmanager.h
	g++ -g -o $(EXE) server_threaded.c workqueue.c dbmanager.cpp /usr/lib/libjson_linux-gcc-4.8_libmt.a -levent -lpthread -lmysqlclient
#.PHONY表示clean是个伪目标文件
.PHONY : clean
clean : 
	-rm -f $(EXE)
	


