CC=gcc
CFLAGS=-lX11 -lstdc++ -g

all:
	@$(CC) $(CFLAGS) ./zwin.cpp -o zwin

clean:
	if [ -e ./zwin ] {
		rm ./zwin
	}

#install:
#	sudo cp ./zwin /bin

#uninstall:
#	sudo rm /bin/zwin
