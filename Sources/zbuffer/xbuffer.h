#ifndef XBUFFER_H
#define XBUFFER_H

#define MINIMIZE_BLOCK 32						//最小块大小为32字节

class ZBuffer {
	char *send_buffer;							//实际使用的发送缓冲区
	char *receive_buffer;						//实际使用的接收缓冲区
	long max_send;								//发送缓冲区的大小
	long max_receive;							//接收缓冲区的大小

//下面是一些统计数据
	bool bPerf;
//	ZPerf send_perf;							//发送性能统计
//	ZPerf receive_perf;							//接收性能统计

	char *receive_ptr;							//当前接收数据的指针
	long receive_size;							//接收数据的大小

	char *send_ptr;								//当前发送数据的指针
	long send_size;								//发送数据的大小
	char *packet_ptr;							//当前数据包的开始指针
	long packet_size;							//当前数据包的大小

public:
	ZBuffer(long the_max_send, long the_max_receive) {
		send_buffer = new char[the_max_send];
		if(send_buffer) max_send = the_max_send;
		else max_send = 0;
		receive_buffer = new char[the_max_receive];
		if(receive_buffer) max_receive = the_max_receive;
		else max_receive = 0;
	}
	virtual ~ZBuffer() {
		if(send_buffer) delete[] send_buffer;
		if(receive_buffer) delete[] receive_buffer;
	}

//下面是对外提供的接口
	void stopPerf() {
		bPerf = false;
		send_perf.stop();
		receive_perf.stop();
	}

	void startPerf() {
		bPerf = true;
		send_perf.start();
		receive_perf.start();
	}

//获取可用的接收数据的缓冲区
	char *getReceiveBuffer(int &size) {
		if(receive_size + MINIMIZE_BLOCK > max_receive) {
//超出最大缓冲区，关闭连接
			return NULL;
		}
		if(receive_ptr + receive_size + MINIMIZE_BLOCK > receive_buffer + max_receive) {		//尾部数据不够了
			memmove(receive_buffer, receive_ptr, receive_size);
			receive_ptr = receive_buffer;
		}
		size = max_receive - (receive_ptr - receive_buffer) - receive_size;
		return receive_ptr + receive_size;
	}
	void receiveData(int size) {
		receive_ptr += size;
	}
	const char *getPacket(int &size) {
		if(receive_size < sizeof(unsigned short)) return NULL;
		size = *(unsigned short *)receive_ptr;
		if(receive_size >= size + sizeof(unsigned short)) {
			receive_ptr += sizeof(unsigned short);
			const char *result = (const char *)receive_ptr;
			receive_ptr += size;
			return result;
		}
		return NULL;
	}

	void clear() {
		send_ptr = send_buffer;
		*(unsigned short *)send_ptr = 0;
		send_size = 0;
		packet_size = 0;
	}

//发送数据的接口分成两个函数，一个是获取当前发送的缓冲区和长度，另外一个是从缓冲区清除指定大小的数据
	char *getSendData(int &size) {
		size = (int)*(unsigned short *)send_ptr;
		if(size) return send_ptr;
		else return NULL;
	}
	void sendData(int size) {
		send_ptr += size;
		send_size -= size;
		if(!send_size) {
			send_ptr = packet_ptr = send_buffer;
		}
	}

	bool packData(const char *data, int size) {
		if(send_size + size + MINIMIZE_BLOCK >= max_send) {						
//缓冲区满，关闭连接
		}
		if(!packet_size) {
			packet_ptr += sizeof(unsigned short);									//头上两个字节表示大小
			*(unsigned short *)packet_ptr = 0;
		}

		if(packet_ptr + packet_size + size + MINIMIZE_BLOCK >= send_buffer + max_send) {		//尾部的空间不够了
			memmove(send_buffer, send_ptr, send_size);
			send_ptr = send_buffer;
			packet_ptr -= (send_ptr - send_buffer);
		}
		memcpy(packet_ptr + packet_size, data, size);
		packet_size += size;
		send_size += size;
	}

	char *sendPacket(int index, int &datalength) {							//完成一个数据包
		*(unsigned short *)packet_ptr = packet_size;
		packet_ptr += packet_size;
		packet_size = 0;
	}

};

#endif