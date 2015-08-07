EXE = PosServer			#可执行文件名

all : $(EXE)

$(EXE) : main.cpp server.h public_function.h thread_pool.h
	g++ -g -o $(EXE) main.cpp server.cpp public_function.cpp thread_pool.cpp -lpthread -L/usr/lib/x86_64-linux-gnu
	
#.PHONY表示clean是个伪目标文件
.PHONY : clean
clean : 
	-rm -f $(EXE)
	


