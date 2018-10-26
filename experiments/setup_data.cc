#include "setup_data.h"

#include <fstream>
#include <iostream>
#include <algorithm>
#include <random>

// #include "rocksdb/db.h"
// #include "rocksdb/options.h"


using namespace std;
// using namespace rocksdb;


int keys[N_ENTRIES];

// int load_keys_data(const char* filename, DB* db) {
//     ifstream fin(filename, ios::binary);

//     char buf[4];
//     char val_buf[4];
//     fin.read(buf, 4);
//     int n = int_of_string(buf);
//     // cout << "n = " << n << endl;

//     int key;
//     Status s;
//     WriteOptions write_options;
//     for (int i = 0; i < n; ++i)
//     {
//         fin.read(buf, sizeof(int));
//         key = int_of_string(buf);
//         string_of_int(val_buf, i);
//         // cout << key << " -> " << i << endl;
//         s = db->Put(write_options,
//                     Slice(buf, 4),
//                     Slice(val_buf, 4));
//         assert(s.ok());

//         string value;
//         s = db->Get(ReadOptions(), Slice(buf, 4), &value);
//         assert(s.ok());
//         assert(int_of_string(value.c_str()) == i);
//         // cout << int_of_string(value.c_str()) << endl;
//     }

//     fin.close();
//     return n;
// }

void generate_keys_data() {
    /* Setup array */
    for (int i = 0; i < N_ENTRIES; ++i)
    {
        keys[i] = i;
    }

    /* Shuffle Array */
    shuffle(keys, keys + N_ENTRIES, default_random_engine(SEED));

    ofstream fout(KEYS_FILENAME, ios::binary);
    int n = N_ENTRIES;
    char buf[4];
    string_of_int(buf, n);
    fout.write(buf, sizeof(int));
    for (int i = 0; i < N_ENTRIES; ++i)
    {
        string_of_int(buf, keys[i]);
        fout.write(buf, sizeof(int));
    }
    fout.close();
}


int int_of_string(const char* str) {
    unsigned int result = 0;
    const unsigned char* str2 = reinterpret_cast<const unsigned char*>(str);
    result |= ((unsigned int) str2[3]);
    result |= ((unsigned int) str2[2]) << 8;
    result |= ((unsigned int) str2[1]) << 16;
    result |= ((unsigned int) str2[0]) << 24;
    return result;
}

void string_of_int(char* str, int x) {
    str[3] = x % 256;
    str[2] = (x >> 8) % 256;
    str[1] = (x >> 16) % 256;
    str[0] = (x >> 24) % 256;
}

// int main() {
//     generate_keys_data();
// }

