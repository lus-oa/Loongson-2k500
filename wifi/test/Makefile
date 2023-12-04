# Copyright (c) 2023 Shandong University
# Copyright (c) 2023 Jinrun Yang, Junchi Ren

CC = loongarch64-linux-gnu-gcc
all:
	$(CC) -lpthread -O2 server.c -o server
	$(CC) -lpthread -O2 client.c -o client
clean:
	rm -rf ./server ./client

