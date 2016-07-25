#ifdef ZBUFFER_H
#define ZBUFFER_H

#include "zport.h"
#define MINIMIZE_BLOCK_SIZE	16					//16字节对齐，最小的块大小为16字节

#define STATE_FREE						0					//未使用状态
#define STATE_IDLE						1					//已经分配没有数据状态
#define STATE_RECEIVING					2					//正在接收数据
#define STATE_WAITING					3					//等待应用程序取走数据
#define STATE_PACKING					4					//正在打包数据
#define STATE_SENDING					5					//正在发送

typedef struct {
	// is it should be 2 bytes? 
	unsigned long  size;									//数据包的大小(包括对齐的字节)
	unsigned short data_size;								//实际数据的大小
	unsigned short current_size;							//当前大小(发送或者接收过程中)
	unsigned short  state;									//当前状态
	unsigned short  reserved;								//保留
	long next;												//下一数据包(偏移量)
} packet_info;

//数据包在缓冲区中的定义
#define CONNECTION_FREE					0					//连接没有使用
#define CONNECTION_USED					1					//连接已经使用

//连接的基本信息
class connection_info
{
public:
	short state;								//连接的状态
	unsigned short halfword;					//边界处理 [wxb 2003-8-5]
	long head_offset;							//头数据包在缓冲区中的偏移量,-1表示没有数据
	long tail_offset;							//尾数据包在缓冲区中的偏移量,-1表示没有数据
	long used_bufsize;							//所占用的缓冲区字节数
	unsigned long packet_time;					//上次数据包的接收时间
	connection_info() { reset(); }
	void reset()
	{
		state = CONNECTION_FREE;
		head_offset = tail_offset = -1;
		packet_time = 0;
		halfword = 0;
		used_bufsize = 0;
	}
};								

//进行性能统计的类
class ZPerf {
public:
	unsigned long packet_number;				//发送/接收的数据包总数
	unsigned long remain_number;				//当前仍然在缓冲区的数目

	int max_interval;							//包之间的最大时间间隔
	int min_interval;							//包之间的最小时间间隔
	unsigned long tick_count;					//开始的时间

	short max_size;								//包的最大大小
	short min_size;								//包的最小大小
	unsigned long total_size;					//总大小
	unsigned long remain_size; 
	ZPerf();
	void start();
	void stop();
	void perfPacket(int size, connection_info *connection);
	void useBuffer(int size, connection_info *connection);
	void freeBuffer(int size, connection_info *connection);
};

class ZBuffer {
public:
	char *buffer;								//实际的缓冲区
	packet_info *free_packet;					//空闲块
	long buffer_size;							//缓冲区的大小
	connection_info *connections;				//连接的列表
	int max_connection;							//最大连接数
	ZMutex mutex;
	
	char **recv_buffers;						//等待处理的收取数据
	int *recv_buffer_size;

protected:

//得到一个空闲的数据块，将这个数据块放到指定索引的空闲节点链表最后
	bool inline getNode(int index, int size);

	char *getRecvBuffer(int index, int size)
	{
#define MIN_RECV_BUF_SIZE	1024

		if (recv_buffer_size[index] >= size)
			return recv_buffers[index];

		//realloc
		if (recv_buffers[index])
			delete recv_buffers[index];
		recv_buffer_size[index] = MIN_RECV_BUF_SIZE > size ? MIN_RECV_BUF_SIZE : size;
		recv_buffers[index] = new char[recv_buffer_size[index]];
		return recv_buffers[index];
	}
public:
//下面是一些统计数据
	bool bPerf;
	ZPerf send_perf;							//发送性能统计
	ZPerf receive_perf;							//接收性能统计

	ZBuffer(long size, int number);
	virtual ~ZBuffer();

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

	void clear(int index, bool bSendOrRecv);	//清除指定连接的所有数据
	long getUsedBufferSize(int index);			//获取指定连接所占用的缓冲区大小
	int  getConnection();						//获得一个没有使用的连接

	bool receiveData(int index, const char *data, int size);
	char *getPacket(int index, int &size);

//发送数据的接口分成两个函数，一个是获取当前发送的缓冲区和长度，另外一个是从缓冲区清除指定大小的数据
	char *getSendData(int index, int &size);
	void sendData(int index, int size);
	bool packData(int index, const char *data, int size);
	char *sendPacket(int index, int &datalength);
};

#define MAX_PACKET_SIZE			1024			//最大的包大小

#endif
