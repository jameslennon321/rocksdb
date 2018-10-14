// Copyright (c) 2011-present, Facebook, Inc.  All rights reserved.
//  This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#include <cstdio>
#include <cstdlib>
#include <string>

#include "rocksdb/db.h"
#include "rocksdb/slice.h"
#include "rocksdb/options.h"

using namespace rocksdb;
using namespace std;

#define N_ENTRIES 1000000

std::string kDBPath = "/tmp/rocksdb_range_experiment";


void fill_db(DB* db) {

  char key_str[33];
  char val_str[33];
  Status s;
  for (int i = 0; i < N_ENTRIES; ++i)
  {
    do {
      int key = rand();
      sprintf(key_str, "%d", key);
      string val_str2 = string(val_str);
      s = db->Get(ReadOptions(), key_str, &val_str2);
    } while (!s.IsNotFound());

    int val = rand();
    sprintf(val_str, "%d", val);
    s = db->Put(WriteOptions(), key_str, val_str);
    assert(s.ok());
  }
}


int main() {
  DB* db;
  Options options;
  // Optimize RocksDB. This is the easiest way to get RocksDB to perform well
  options.IncreaseParallelism();
  options.OptimizeLevelStyleCompaction();
  // create the DB if it's not already present
  options.create_if_missing = true;

  // open DB
  Status s = DB::Open(options, kDBPath, &db);
  assert(s.ok());

  fill_db(db);

  delete db;
  return 0;
}
