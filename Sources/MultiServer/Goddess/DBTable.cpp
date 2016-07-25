#include "stdafx.h"
#include "DBTable.h"
#ifndef WIN32
#include <sys/stat.h>
#else
#include <direct.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


ZDBTable::ZDBTable(const char *path, const char *name) {
	
#ifdef WIN32
		getcwd(env_path, MAX_TABLE_NAME);
		char buff[MAX_TABLE_NAME];
		getcwd( buff, MAX_TABLE_NAME );
#else
#endif
		strcat(buff, "\\");
		strcat(buff, path);
#ifdef WIN32
		int ret = mkdir(buff);
#else
        int ret = mkdir(env_path, 0);
#endif
	
		if(!db_env_create(&dbenv, 0)) {
			dbenv->set_errpfx(dbenv, "index_db");
            dbenv->set_lg_regionmax(dbenv, 512 * 1024);
            dbenv->set_lg_max(dbenv, 16 * 1024 * 1024);
            dbenv->set_lg_bsize(dbenv, 2 * 1024 * 1024);
            dbenv->set_cachesize(dbenv, 0, 64 * 1024 * 1024, 1);

			if(!dbenv->open(dbenv, buff, DB_CREATE | DB_INIT_LOG | DB_INIT_LOCK | DB_INIT_MPOOL | DB_INIT_TXN | DB_RECOVER | DB_THREAD | DB_PRIVATE, 0)) {
				ret = dbenv->set_flags(dbenv, DB_AUTO_COMMIT | DB_TXN_NOSYNC, 1);
				index_number = 0;
				strcpy(table_name, name);
				return;				//成功了
			}
		dbenv->close(dbenv, 0);
	}
	dbenv = NULL;
}

ZDBTable::~ZDBTable() {
	if(dbenv) dbenv->close(dbenv, 0);
}

int ZDBTable::addIndex(GetIndexFunc func, bool isUnique) {
	if(!dbenv) return -1;
	if(index_number + 1 >= MAX_INDEX) return index_number;
	get_index_funcs[index_number] = func;
	is_index_unique[index_number] = isUnique;
	return index_number++;
}

bool ZDBTable::open() {
	if(!dbenv) return false;
	bStop = false;
	char index_table_name[MAX_TABLE_NAME];
    int index;
	int ret;
	if(!db_create(&primary_db, dbenv, 0)) {
		if(!primary_db->open(primary_db, NULL, table_name, NULL, DB_BTREE, DB_CREATE| DB_AUTO_COMMIT | DB_THREAD, 0664)) {	//打开主数据库
			for(index = 0; index < index_number; index++) {
				sprintf(index_table_name, "%s.%d", table_name, index);
				if(!db_create(&index_db[index], dbenv, 0)) {
					if(!is_index_unique[index]) {
						if(index_db[index]->set_flags(index_db[index], DB_DUP | DB_DUPSORT)) break;
					}
					if(index_db[index]->open(index_db[index], NULL, index_table_name, NULL, DB_BTREE, DB_CREATE | DB_AUTO_COMMIT | DB_THREAD, 0664)) break;
					if(ret = primary_db->associate(primary_db, NULL, index_db[index], get_index_funcs[index], DB_AUTO_COMMIT)) {
						index_db[index]->close(index_db[index], 0);
						break;
					}
				}
				else break;
			}
			if(index == index_number) return true;										//成功了
			else while(--index) (index_db[index])->close(index_db[index], 0);				//出错，关闭前面的索引表
			primary_db->close(primary_db, 0);
		}
	}
	return false;
}

void ZDBTable::close() {
	if(!dbenv) return;
	primary_db->close(primary_db, 0);
	for(int index = 0; index < index_number; index++) index_db[index]->close(index_db[index], 0);
}

bool ZDBTable::commit() {
	return true;
}

bool ZDBTable::add(const char *key_ptr, int key_size, const char *data_ptr, int data_size) {
	DBT data, key;
	memset(&key, 0, sizeof(DBT));
	memset(&data, 0, sizeof(DBT));
	key.data = (void *)key_ptr;
	key.size = key_size;
	data.data = (void *)data_ptr;
	data.size = data_size;

	int ret;
	int retry = 0;
RETRY:
	ret = primary_db->put(primary_db, NULL, &key, &data, DB_AUTO_COMMIT);
	if(ret == DB_LOCK_DEADLOCK && ++retry < MAX_RETRY) {
		if(bStop) return false;
		goto RETRY;
	}
	if(ret) return false;
	else return true;
}

bool ZDBTable::remove(const char *key_ptr, int key_size, int index) {
	DBT data, key;
	memset(&key, 0, sizeof(DBT));
	memset(&data, 0, sizeof(DBT));
	key.data = (void *)key_ptr;
	key.size = key_size;
	int ret;
	int retry = 0;
RETRY:
	ret = primary_db->del(primary_db, NULL, &key, DB_AUTO_COMMIT);
	if(ret == DB_LOCK_DEADLOCK && ++retry < MAX_RETRY) {
		if(bStop) return false;
		goto RETRY;
	}
	if(ret) return false;
	else return true;
}

ZCursor *ZDBTable::_search(bool bKey, const char *key_ptr, int key_size, int index) {
	DBT key, data, pkey;
	if(index < -1 || index >= index_number) return NULL;
	memset(&key, 0, sizeof(key));
	memset(&data, 0, sizeof(data));
	memset(&pkey, 0, sizeof(pkey));
	key.flags = DB_DBT_MALLOC;
	data.flags = DB_DBT_MALLOC;
	pkey.flags = DB_DBT_MALLOC;
	key.data = (void *)key_ptr;
	key.size = key_size;
	DBC *dbcp;
	if(!key_ptr || !key_size) {							//没有设定索引值，要求遍历数据库
		if(index_db[index]->cursor(index_db[index], NULL, &dbcp, 0)) {
			return NULL;
		}
		if(dbcp->c_get(dbcp, &key, &data, DB_FIRST)) {
			dbcp->c_close(dbcp);
			return NULL;
		}
	}
	else {
		if(index == -1) {									//主键搜索
			if(primary_db->get(primary_db, NULL, &key, &data, 0)) return NULL;
		}
		else if(is_index_unique[index]) {					//没有重复索引
			if(bKey) {
				if(index_db[index]->pget(index_db[index], NULL, &key, &pkey, &data, 0)) return NULL;
			}
			else {
				if(index_db[index]->get(index_db[index], NULL, &key, &data, 0)) return NULL;
			}
		}
		else {												//打开游标
			if(index_db[index]->cursor(index_db[index], NULL, &dbcp, 0)) {
				return NULL;
			}
			if(bKey) {
				if(dbcp->c_pget(dbcp, &key, &pkey, &data, DB_SET)) {
					dbcp->c_close(dbcp);
					return NULL;
				}
			}
			else {
				if(dbcp->c_get(dbcp, &key, &data, DB_SET)) {
					dbcp->c_close(dbcp);
					return NULL;
				}
			}
		}
	}
	ZCursor *result = new ZCursor;
	result->index = 0;
	result->dbcp = dbcp;
	if (bKey) 
    {
		result->data = (char *)pkey.data;
		result->size = pkey.size;

        pkey.data    = NULL;
	}
	else 
    {
		result->data = (char *)data.data;
		result->size = data.size;

        data.data    = NULL;
	}
	
    if (!key_ptr || !key_size) 
    {
		result->key = (char *)key.data;
		result->key_size = key.size;
		result->bTravel = true;
		
        key.data = NULL;
	}
	else 
    {
        result->bTravel = false;
    }
	
    if(pkey.data) free(pkey.data);
	if(data.data) free(data.data);

	return result;
}

bool ZDBTable::_next(bool bKey, ZCursor *cursor) {
	DBT key, data, pkey;
	memset(&key, 0, sizeof(key));
	memset(&data, 0, sizeof(data));
	memset(&pkey, 0, sizeof(pkey));
	key.flags = DB_DBT_MALLOC;
	data.flags = DB_DBT_MALLOC;
	pkey.flags = DB_DBT_MALLOC;
	if(!cursor || !cursor->dbcp) return false;
	free(cursor->data);
	cursor->index++;
	if(cursor->bTravel) {
		free(cursor->key);
		if(cursor->dbcp->c_get(cursor->dbcp, &key, &data, DB_NEXT)) {
			cursor->dbcp->c_close(cursor->dbcp);
			delete cursor;
			return false;
		}
	}
	else {
		if(bKey) {
			if(cursor->dbcp->c_pget(cursor->dbcp, &key, &pkey, &data, DB_NEXT_DUP)) {
				cursor->dbcp->c_close(cursor->dbcp);
				delete cursor;
				return false;
			}
		}
		else {
			if(cursor->dbcp->c_get(cursor->dbcp, &key, &data, DB_NEXT_DUP)) {
				cursor->dbcp->c_close(cursor->dbcp);
				delete cursor;
				return false;
			}
		}
	}
	if (bKey) 
    {
		cursor->data = (char *)pkey.data;
		cursor->size = pkey.size;

        pkey.data = NULL;
	}
	else {
		cursor->data = (char *)data.data;
		cursor->size = data.size;

        data.data = NULL;
	}
	
    if (cursor->bTravel) 
    {
		cursor->key = (char *)key.data;
		cursor->key_size = key.size;

        key.data = NULL;
	}

	if (key.data) free(key.data);
	if (pkey.data) free(pkey.data);
	if (data.data) free(data.data);
	
    return true;
}

void ZDBTable::removeLog() {						//清除日志文件
	int ret;
	char **begin, **list;
	if((ret = dbenv->txn_checkpoint(dbenv, 0, 0, 0)) != 0) return;

	if((ret = dbenv->log_archive(dbenv, &list, DB_ARCH_ABS)) != 0) {
		return ;
	}
	if (list != NULL) {
		for(begin = list; *list != NULL; ++list)
			if((ret = ::remove(*list)) != 0) {
				return;
			}
		free(begin);
	}
}
