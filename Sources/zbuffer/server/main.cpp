//目前使用的网络协议，头上两个字节是数据包的大小

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include "../IServer.h"

int main(int argc, char **argv) {
	IServer server(100);
	server.Startup();
	server.Open(inet_addr("127.0.0.1"), 1010);
        server.buffer->startPerf();
	unsigned long length;
	int i = 0;
	while(true) {
#ifdef WIN32		
		Sleep(1);
#else
		sleep(1);
#endif
		for(int index = 0; index < 100; index++) {
			char *data = (char *)server.GetPackFromClient(index, length);
                        if(data) {
				printf("%d index = %d client %d get data %d\n", server.buffer->receive_perf.remain_number, i++, index, length);
				for(int j = 0; j < 10; j++) {
					server.PackDataToClient(index, data, length);
				}
				server.SendPackToClient(index);
			while(data) {
//				printf("index = %d client %d get data %d\n", i++, index, length);
//                                printf("remain number %d\n", server.buffer->send_perf.remain_number);
//                                printf("remain number %d\n", server.buffer->receive_perf.remain_number);
				data = (char *)server.GetPackFromClient(index, length);
			}
                               }
		}
	}
	return 0;
}

