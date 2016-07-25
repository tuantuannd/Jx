// PackSingle.cpp : Defines the entry point for the console application.
//

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <direct.h>
//首先是处理SPR文件的代码

//SPR头文件的定义，为了实现可移植的目标


#include "ZPackFile.h"
typedef struct {
	BYTE	Comment[4];	// 注释文字(SPR\0)
	WORD	Width;		// 图片宽度
	WORD	Height;		// 图片高度
	WORD	CenterX;	// 重心的水平位移
	WORD	CenterY;	// 重心的垂直位移
	WORD	Frames;		// 总帧数
	WORD	Colors;		// 颜色数
	WORD	Directions;	// 方向数
	WORD	Interval;	// 每帧间隔（以游戏帧为单位）
	WORD	Reserved[6];// 保留字段（到以后使用）
} SPRHEAD;

typedef struct
{
	DWORD	Offset;		// 每一帧的偏移
	DWORD	Length;		// 每一帧的长度
} SPROFFS;


//重新定义
#define FRAME_SIZE			800 * 1024			//800K以上的SPR文件使用分帧的压缩
int root_length = 0;

#define MAX_FILE					2004800			//最多20万个文件

index_info index_list[MAX_FILE];
char *temp_buffer[MAX_FILE * sizeof(index_info)];
int file_count;										//当前文件的数量

unsigned long offset;								//当前偏移量
#define COMPRESS_BUF_SIZE	10240000
char compress_buffer[COMPRESS_BUF_SIZE];			//10M的压缩缓冲区，存放所有的帧，一次写

#include <ucl/ucl.h>
bool bCheck = false;
#include <conio.h>

//增加一个文件到数据文件中，返回偏移量
bool addFile(FILE *output, const char *file_name) {
	char full_name[MAX_PATH];
	getcwd(full_name, MAX_PATH);
	strcat(full_name, "\\");
	strcat(full_name, file_name);
	char *ptr = full_name;
	while(*ptr) {
		if(*ptr >= 'A' && *ptr <= 'Z') *ptr += 'a' - 'A';
		ptr++;
	}

	unsigned long id = hash(full_name + root_length);
	int index;
	for(index = 0; index < file_count; index++) {
		if(id < index_list[index].id) {
			memmove(temp_buffer, (char *)&index_list[index], (file_count - index) * sizeof(index_info));
			memmove((char *)&index_list[index + 1], temp_buffer, (file_count - index) * sizeof(index_info));
			break;
		}
		else if(id == index_list[index].id) {
			printf("error %s has the same id %d\n", full_name + root_length, id);

			if(!bCheck) exit(0);
			else getch();
		}
	}
	file_count++;
	if(bCheck) {
		printf("check %s ok\n", full_name + root_length);
		return true;
	}

	ZMapFile map(file_name);
	int compress_size = 0;

	unsigned long compress_type = TYPE_UCL;				//使用UCL压缩

	bool bSPR = false;									//是否为SPR文件
	const char *ext = file_name + strlen(file_name) - 3;
	if(*ext == 's' && *(ext + 1) == 'p' && *(ext + 2) == 'r') bSPR = true;

	int r;
	unsigned int size = 0;
	map.map();
	if(map.m_Ptr) {

		index_list[index].id = id;
		index_list[index].offset = offset;
		index_list[index].size = map.m_Size;

		ptr = compress_buffer;
		if(!bSPR || map.m_Size < FRAME_SIZE) {
			if(compress_type == TYPE_UCL) {
				r = ucl_nrv2b_99_compress((BYTE *)map.m_Ptr, map.m_Size, (BYTE *)ptr, (unsigned int *)&index_list[index].compress_size, NULL, 5, NULL, NULL);
			}
			else if(compress_type == TYPE_BZIP2) {
//				index_list[index].compress_size = COMPRESS_BUF_SIZE;
//				r = BZ2_bzBuffToBuffCompress(ptr, (unsigned int *)&index_list[index].compress_size, map.m_Ptr, map.m_Size, 9, 0, 30);
			}
			if(r) return false;
			fwrite(compress_buffer, 1, index_list[index].compress_size, output);
			offset += index_list[index].compress_size;
			printf("%s [%d]->[%d]\n", full_name + root_length, map.m_Size, index_list[index].compress_size);
			index_list[index].compress_size |= (compress_type << 24);
		}
		else {								//每帧独立压缩
			SPRHEAD *head;
			head = (SPRHEAD *)map.m_Ptr;
			memmove(ptr, head, sizeof(SPRHEAD) + head->Colors * 3);			//前面的数据不压缩
			ptr += sizeof(SPRHEAD) + head->Colors * 3;
			frame_info *compress_frame_info = (frame_info *)ptr;					//压缩后每一帧的数据
			ptr += head->Frames * sizeof(SPROFFS);
	
			SPROFFS *frame_info = (SPROFFS *)(map.m_Ptr + sizeof(SPRHEAD) + head->Colors * 3);		//原来每一帧的数据
			char *frame_data = (char *)frame_info + head->Frames * sizeof(SPROFFS);
			int frame_index;

			int frame_offset = 0;
			for(frame_index = 0; frame_index < head->Frames; frame_index++) {
//压缩每一帧的内容
				if(frame_info[frame_index].Length >= 256) {				//小于256字节的不压缩
					if(compress_type == TYPE_UCL) {
						r = ucl_nrv2b_99_compress((BYTE *)frame_data + frame_info[frame_index].Offset, frame_info[frame_index].Length, (BYTE *)ptr, &size, NULL, 10, NULL, NULL);
					}
					else if(compress_type == TYPE_BZIP2) {
//						size = COMPRESS_BUF_SIZE;
//						r = BZ2_bzBuffToBuffCompress(ptr, &size, frame_data + frame_info[frame_index].Offset, frame_info[frame_index].Length, 9, 0, 30);
					}
					if(r) return false;
					compress_frame_info[frame_index].size = frame_info[frame_index].Length;		//记录原来的大小
				}
				else {
					size = frame_info[frame_index].Length;
					memmove(ptr, (BYTE *)frame_data + frame_info[frame_index].Offset, size);
					compress_frame_info[frame_index].size = -(long)frame_info[frame_index].Length;		//记录原来的大小
				}
				compress_size += size;
				compress_frame_info[frame_index].compress_size = size;
				frame_offset += size;
				ptr += size;		
			}
			fwrite(compress_buffer, 1, ptr - compress_buffer, output);
			offset += ptr - compress_buffer;
			printf("[frame] %s old size = %d, compressed size = %d\n", full_name + root_length, map.m_Size, compress_size);
			index_list[index].compress_size = (ptr - compress_buffer) | ((compress_type | TYPE_FRAME) << 24);
		}
	}
	return true;
}

void addDirectory(FILE *output, const char *rootDir, const char *subDir = NULL) {
	char			szRealDir[MAX_PATH];
	if(subDir) sprintf(szRealDir, "%s\\%s", rootDir, subDir);
	else {
		strcpy(szRealDir, rootDir);
		root_length = strlen(rootDir);
		while(rootDir[root_length] != '\\') root_length--;
	}
	if(chdir(szRealDir)) return;
	_finddata_t FindData;
	long dir = _findfirst("*.*", &FindData);
	while(dir != -1) {
		if(strcmp(FindData.name, ".") == 0 || strcmp(FindData.name, "..") == 0)	{
			if(_findnext(dir, &FindData)) break;
			continue;
		}
		if(FindData.attrib & _A_SUBDIR)
		{
			addDirectory(output, szRealDir, FindData.name);
		}
		else
		{	
			if(!addFile(output, FindData.name)) return;
		}
		if(_findnext(dir, &FindData)) break;
	} 
	_findclose(dir);
	chdir(rootDir);
}

bool pack(const char *dir, const char *pack_name = 0) {
	char name_buffer[MAX_PATH];
	file_count = 0;
	offset = 0;
	if(!pack_name) {
		strcpy(name_buffer, dir);
		strcat(name_buffer, ".pak");
		pack_name = name_buffer;
	}
	FILE * output = NULL;
	z_pack_header header;
	if(!bCheck) {
		output = fopen(pack_name, "wb");
		memset(&header, 0, sizeof(header));
		fwrite(&header, 1, sizeof(header), output);
		offset += sizeof(header);
	}

	char aAddDir[MAX_PATH];
	strcpy(aAddDir, dir);
	strcat(aAddDir, "\\Font");
	addDirectory(output, aAddDir);					//压缩指定目录
	strcpy(aAddDir, dir);
	strcat(aAddDir, "\\Maps");
	addDirectory(output, aAddDir);					//压缩指定目录
	strcpy(aAddDir, dir);
	strcat(aAddDir, "\\settings");
	addDirectory(output, aAddDir);					//压缩指定目录
	strcpy(aAddDir, dir);
	strcat(aAddDir, "\\script");
	addDirectory(output, aAddDir);					//压缩指定目录
	strcpy(aAddDir, dir);
	strcat(aAddDir, "\\Spr");
	addDirectory(output, aAddDir);					//压缩指定目录
	strcpy(aAddDir, dir);
	strcat(aAddDir, "\\Sound");
	addDirectory(output, aAddDir);					//压缩指定目录
	strcpy(aAddDir, dir);
	strcat(aAddDir, "\\Ui");
	addDirectory(output, aAddDir);					//压缩指定目录
	strcpy(aAddDir, dir);
	strcat(aAddDir, "\\游戏资源");
	addDirectory(output, aAddDir);					//压缩指定目录

	if(!bCheck) {
		memset(&header, 0, sizeof(header));
		memcpy(header.signature, "PACK", 4);
		header.index_offset = offset;
		header.data_offset = sizeof(header);
		header.count = file_count;
		int result = fwrite(&index_list[0], 1, file_count * sizeof(index_info), output);
		fseek(output, 0, SEEK_SET);
		fwrite(&header, 1, sizeof(header), output);
		fclose(output);
	}
	return true;
}

int main(int argc, char **argv) {

	if(argc < 2) {
		printf("usage: packSingle [pack directory] [pack file name] [/c]\nwithout [pack file name], the pack file name will be the same of directory name\n/c to check the same id\n");
		return 0;
	}

	if (ucl_init() != UCL_E_OK) return 0;

	if(argc > 2) {
		if(!stricmp(argv[2], "/c")) {
			bCheck = true;
			pack(argv[1]);
		}
		else {
			if(argc > 3) {
				if(!stricmp(argv[3], "/c")) bCheck = true;
			}
			pack(argv[1], argv[2]);
		}
	}
	else pack(argv[1]);

	return 0;
}