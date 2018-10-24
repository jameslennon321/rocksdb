// Copyright (c) 2011-present, Facebook, Inc.  All rights reserved.
//  This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#include <cstdio>
#include <ctime>
#include <iostream>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <functional>
#include <random>

#include "rocksdb/db.h"
#include "rocksdb/slice.h"
#include "rocksdb/options.h"

#include "setup_data.h"

using namespace rocksdb;
using namespace std;


#define Q_RANGE 0
#define Q_READ 1
#define Q_WRITE 2


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


/* Returns time in microseconds */
int timed_func(function<void()> func) {
	clock_t start = clock();
	func();
	return (clock() - start) / (double) (CLOCKS_PER_SEC / 1000000);
}


double timed_execute_query(DB* db, int query_type, int key1, int key2) {
	function<void()> func;
	if (query_type == Q_READ) {
		func = [db, key1] {
			execute_point_read(db, key1);
		};
	} else if (query_type == Q_WRITE) {
		func = [db, key1, key2] {
			execute_point_write(db, key1, key2);
		};
	} else if (query_type == Q_RANGE) {
		func = [db, key1, key2] {
			execute_range_query(db, min(key1, key2), max(key1, key2));
		};
	} else {
		cerr << "Unrecognized query type: " << query_type << endl;
		assert(false);
	}

	return timed_func(func);
}


int execute_workload(DB* db, int db_size, int n_queries, double w1, double w2, double w3, double* usec_trace) {

	default_random_engine generator;
	// uniform_int_distribution<int> q_type_dist(0, 2);
	discrete_distribution<int> q_type_dist({w1, w2, w3});
	uniform_int_distribution<int> key_dist(0, db_size - 1);

	for (int i = 0; i < n_queries; ++i)
	{
		int query_type = q_type_dist(generator);
		int key1 = key_dist(generator);
		int key2 = key_dist(generator);

		usec_trace[i] = timed_execute_query(db, query_type, key1, key2);
	}
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

	int n_queries = 1000;
	cout << "Executing workload for " << n_queries << " queries..." << endl;
	double usec_trace[n_queries];
	execute_workload(db, count, n_queries, 1, 1, 1, usec_trace);
	for (int i = 0; i < count; ++i)
	{
		printf("%f\n", usec_trace[i]);
	}

	execute_workload(db, count, n_queries, 5, 1, 1, usec_trace);
	for (int i = 0; i < count; ++i)
	{
		printf("%f\n", usec_trace[i]);
	}

	execute_workload(db, count, n_queries, 1, 1, 1, usec_trace);
	for (int i = 0; i < count; ++i)
	{
		printf("%f\n", usec_trace[i]);
	}

	int retcode = system(("rm -r " + kDBPath).c_str());
	assert(retcode == 0);

	delete db;
	return 0;
}
