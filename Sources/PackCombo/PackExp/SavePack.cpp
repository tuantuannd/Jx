// SavePack.cpp: implementation of the CSavePack class.
//
//////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <direct.h>

#include "SavePack.h"
#include <conio.h>
#include <IOSTREAM>
using namespace std;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CSavePack::CSavePack()
{
	memset(FileName,0,256);
	output = 0;
	offset = 0;
	header.count = 0;
}

bool CSavePack::open(char* FileName, index_info* index)
{//打开包文件
	index_list = index;
	output = fopen(FileName, "wb");
	if(!output)
		return false;

	memset(&header, 0, sizeof(header));
	fwrite(&header, 1, sizeof(header), output);	//写头部
	offset += sizeof(header);					//指针指到头部后面

	return true;
}

bool CSavePack::Close()
{//保存包文件
	memcpy(header.signature, "PACK", 4);
	header.index_offset = offset;
	header.data_offset = sizeof(header);
	int result = fwrite(&index_list[0], 1, header.count * sizeof(index_info), output);
	fseek(output, 0, SEEK_SET);
	fwrite(&header, 1, sizeof(header), output);
	if(fclose(output))
		return false;
	return true;
}

bool CSavePack::AddData(char* data, index_info* index, char *temp_buffer)
{//添加文件

	int i;
	for(i = 0; i < header.count; i++) {
		if(index->id < index_list[i].id) {
			memmove(temp_buffer, (char *)&index_list[i], (header.count - i) * sizeof(index_info));
			memmove((char *)&index_list[i + 1], temp_buffer, (header.count - i) * sizeof(index_info));
			break;
		}
		else if(index->id == index_list[i].id) {
			cout<<"Error: Same ID "<<index->id;
			getch();
			exit(1);
		}
	}

	memcpy(&index_list[i], index, sizeof(index_info));
	index_list[i].offset = offset;
	int fileSize = index->compress_size & 0x00FFFFFF;
	fwrite(data, 1, fileSize, output);
	offset += fileSize;
	++header.count;
	return true;
}