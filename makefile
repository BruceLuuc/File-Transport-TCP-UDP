CLI:=./bin/client  #二进制生成
SER:=./bin/server

all:$(CLI)  $(SER)

INC_DIR:=./include

CLI_OBJ:=./client/client.c ./client/client_work.c ./include/common.c ./include/md5.c
SER_OBJ:=./server/server.c ./server/server_work.c ./include/common.c ./include/md5.c

$(CLI):$(CLI_OBJ) $(INCLUDES)
	gcc -g -W -Wall -o $@ $(CLI_OBJ) -I$(INC_DIR) -I /lib/x86_64-linux-gnu/ -lreadline

$(SER):$(SER_OBJ) $(INCLUDES)
	gcc -g -W -Wall -o $@ $(SER_OBJ) -I$(INC_DIR) -lpthread

.PHONY:clean
clean:
	rm -rvf $(CLI) $(SER)
