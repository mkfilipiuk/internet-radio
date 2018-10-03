TARGET: sikradio-sender sikradio-receiver

CFLAGS = -std=c++17 -O2 -Wall 
LFLAGS = -pthread
sikradio-sender: cyclic_buffer.o  FIFO.o server.o server_c.o Storage.o err.o
	g++ $(LFLAGS) $(CFLAGS) cyclic_buffer.o FIFO.o server.o server_c.o Storage.o err.o -o sikradio-sender

err.o: common/err.cc
	g++ $(CFLAGS) -c common/err.cc

cyclic_buffer.o: server/cyclic_buffer.cc
	g++ $(CFLAGS) -c server/cyclic_buffer.cc
FIFO.o: server/FIFO.cc
	g++ $(CFLAGS) -c server/FIFO.cc
server.o: server/server.cc
	g++ $(CFLAGS) -c server/server.cc
server_c.o : server/server_c.cc
	g++ $(CFLAGS) -c server/server_c.cc
Storage.o: server/Storage.cc
	g++ $(CFLAGS) -c server/Storage.cc

sikradio-receiver: Broadcaster.o client.o client_c.o menu.o err.o Buffer.o
	g++ $(CFLAGS) $(LFLAGS) Broadcaster.o client.o client_c.o err.o Buffer.o  menu.o -o sikradio-receiver
Buffer.o: client/Buffer.cc
	g++ $(CFLAGS) -c client/Buffer.cc

Broadcaster.o: client/Broadcaster.cc
	g++ $(CFLAGS) -c client/Broadcaster.cc
client.o: client/client.cc
	g++ $(CFLAGS) -c client/client.cc
client_c.o: client/client_c.cc
	g++ $(CFLAGS) -c client/client_c.cc
menu.o: client/menu.cc
	g++ $(CFLAGS) -c client/menu.cc

.PHONY: clean TARGET
clean:
	rm -f sikradio-receiver sikradio-sender *.o
