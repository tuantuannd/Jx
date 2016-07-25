#include "DBTable.h"

int main(int argc, char **argv) {
	ZDBTable table("data", "test");
	if(table.open()) {
//		table.add("zhuchuanjing", 10, "askdfhkdsjfhkdshfkj", 10);
//		table.add("zhu", 3, "askdfhkdsjfhkdshfkj", 10);
//		table.commit();
		int size;
		char *data = table.search("zhu", 3, size);
		if(data) delete[] data;
		table.remove("zhu", 3);
		data = table.search("zhu", 3, size);
		if(data) delete[] data;
//		table.close();
	}
	return 0;
}