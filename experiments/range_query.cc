// Copyright (c) 2011-present, Facebook, Inc.  All rights reserved.
//  This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <string>
#include <unistd.h>

#include "rocksdb/db.h"
#include "rocksdb/slice.h"
#include "rocksdb/options.h"

#include "setup_data.h"

using namespace rocksdb;
using namespace std;


std::string kDBPath = "/tmp/rocksdb_range_experiment";


void execute_range_query(DB* db, int low, int high) {
	char key_buf[4];
	char val_buf[4];

	string_of_int(key_buf, low);
	Slice low_slice = Slice(key_buf, sizeof(int));

	int count = 0;
	int value;
	int key;
	rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
	it->Seek(low_slice);
	while (it->Valid()) {
		key = int_of_string(it->key().ToString().c_str());
		value = int_of_string(it->value().ToString().c_str());

		if (key >= high) {
			break;
		}

		count++;
		it->Next();
	}
	assert(it->status().ok()); // Check for any errors found during the scan
}


int execute_point_read(DB* db, int key) {
	char key_buf[4];
	string val_buf;
	string_of_int(key_buf, key);
	Status s = db->Get(ReadOptions(), Slice(key_buf, 4), &val_buf);
	assert(s.ok());
	return int_of_string(val_buf.c_str());
}


void execute_point_write(DB* db, int key, int val) {
	char key_buf[4];
	char val_buf[4];
	string_of_int(key_buf, key);
	string_of_int(val_buf, val);
	Status s = db->Put(WriteOptions(), Slice(key_buf, 4), Slice(val_buf, 4));
	assert(s.ok());
}


int main() {
	DB* db;
	Options options;
	// Optimize RocksDB. This is the easiest way to get RocksDB to perform well
	// options.IncreaseParallelism();
	// options.OptimizeLevelStyleCompaction();
	// create the DB if it's not already present
	options.create_if_missing = true;

	// open DB
	Status s = DB::Open(options, kDBPath, &db);
	assert(s.ok());

	cout << "Loading data..." << endl;
	int count = load_keys_data(KEYS_FILENAME, db);
	sleep(3);
	cout << "Loaded " << count << " keys" << endl;

	execute_range_query(db, 0, 23);
	cout << execute_point_read(db, 99) << endl;
	execute_point_write(db, 23, 420);

	int retcode = system(("rm -r " + kDBPath).c_str());
	assert(retcode == 0);

	delete db;
	return 0;
}
