#include <fstream>
#include <iostream>
#include <string>
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <random>
#include <functional>

#include <db_cxx.h>

#include "setup_data.h"


#define DB_PATH "my_db.db"

#define Q_RANGE 0
#define Q_READ 1
#define Q_WRITE 2

#define RANGE_LEN 500
#define N_ITERS 3


using namespace std;


int execute_read_query(Db &db, int key) {
    Dbt dbt_key(&key, sizeof(int));
    int result;
    Dbt dbt_val(&result, sizeof(int));
    // dbt_val.set_flags(DB_DBT_USERMEM);
    int ret = db.get(NULL, &dbt_key, &dbt_val, 0);
    if (ret == DB_NOTFOUND) {
        return -1;
    }
    memcpy(&result, dbt_val.get_data(), 4);
    return result;
}


int execute_write_query(Db &db, int key, int val) {
    Dbt dbt_key(&key, sizeof(int));
    Dbt dbt_val(&val, sizeof(int));
    db.put(NULL, &dbt_key, &dbt_val, DB_NOOVERWRITE);
}


int execute_range_query(Db &db, int low, int high) {
    int count = 0;
    Dbc *cursorp;
    Dbt key(&low, sizeof(int));
    Dbt value;

    db.cursor(NULL, &cursorp, 0);

    int ret = cursorp->get(&key, &value, DB_SET);
    while (ret != DB_NOTFOUND) {
        int cur_key, cur_val;
        memcpy(&cur_key, key.get_data(), 4);
        memcpy(&cur_val, value.get_data(), 4);
        // cout << "key = " << cur_key << "; value = " << cur_val << endl;

        if (cur_key >= high) {
            break;
        }

        count += 1;

        ret = cursorp->get(&key, &value, DB_NEXT);
    }

    return count;
}


/* Returns time in microseconds */
int timed_func(function<void()> func) {
    clock_t start = clock();
    func();
    return (clock() - start) / (double) (CLOCKS_PER_SEC / 1000000);
}


double timed_execute_query(Db *db, int query_type, int key1, int key2) {
    function<void()> func;
    if (query_type == Q_READ) {
        func = [db, key1] {
            execute_read_query(*db, key1);
        };
    } else if (query_type == Q_WRITE) {
        func = [db, key1, key2] {
            execute_write_query(*db, key1, key2);
        };
    } else if (query_type == Q_RANGE) {
        func = [db, key1, key2] {
            execute_range_query(*db, min(key1, key2), max(key1, key2));
        };
    } else {
        cerr << "Unrecognized query type: " << query_type << endl;
        assert(false);
    }

    return timed_func(func);
}


int execute_workload(Db &db, const int db_size, const int n_queries, double w1, double w2, double w3, double* usec_trace) {

    default_random_engine generator(SEED);
    // uniform_int_distribution<int> q_type_dist(0, 2);
    discrete_distribution<int> q_type_dist({w1, w2, w3});
    uniform_int_distribution<int> key_dist(0, db_size - 1);

    for (int i = 0; i < n_queries; ++i)
    {
        int query_type = q_type_dist(generator);
        int key1 = key_dist(generator);
        int key2 = key_dist(generator);
        if (query_type == Q_RANGE) {
            key1 = rand() % (db_size - RANGE_LEN);
            key2 = key1 + RANGE_LEN;
        }

        usec_trace[i] = timed_execute_query(&db, query_type, key1, key2);
    }
}


int load_db_from_file(Db &db, string filename) {
    ifstream fin(filename, ios::binary);
    char buf[4];
    fin.read(buf, 4);
    int n = int_of_string(buf);

    for (int i = 0; i < n; ++i)
    {
        fin.read(buf, 4);
        int key = int_of_string(buf);
        int val = i;
        // cout << "Adding (" << key << ", " << val << ")" << endl;
        execute_write_query(db, key, val);
    }

    fin.close();

    return n;
}


int compare_int(Db *dbp, const Dbt *a, const Dbt *b)
{
    int ai, bi;

    // Returns: 
    // < 0 if a < b 
    // = 0 if a = b 
    // > 0 if a > b 
    memcpy(&ai, a->get_data(), sizeof(int)); 
    memcpy(&bi, b->get_data(), sizeof(int)); 
    return (ai - bi); 
} 


int main() {
    Db db(NULL, 0);
    db.set_bt_compare(compare_int);
    
    u_int32_t oFlags = DB_CREATE; // Open flags;

    try {
        // Open the database
        db.open(NULL,                // Transaction pointer 
                DB_PATH,             // Database file name 
                NULL,                // Optional logical database name
                DB_BTREE,            // Database access method
                oFlags,              // Open flags
                0);                  // File mode (using defaults)

        const int count = load_db_from_file(db, KEYS_FILENAME);
        // execute_write_query(db, 123, 42);
        // cout << "wrote one" << endl;
        // execute_write_query(db, 124, 43);
        // cout << "wrote one" << endl;
        // execute_write_query(db, 125, 44);
        // cout << "wrote one" << endl;
        // int val = execute_read_query(db, 123);
        // cout << "got value: " << val << endl;

        // int count = execute_range_query(db, 123, 150);
        // cout << "count = " << count << endl;

        const int n_queries = 2000;
        double usec_trace[n_queries];
        execute_workload(db, count, n_queries, 1, 1, 1, usec_trace);
        execute_workload(db, count, n_queries, 1, 1, 1, usec_trace);
        for (int j = 0; j < N_ITERS; ++j)
        {
            // execute_workload(db, count, n_queries, 1, 1, 1, usec_trace);
            // for (int i = 0; i < n_queries; ++i)
            // {
            //     printf("%f\n", usec_trace[i]);
            // }

            execute_workload(db, count, n_queries, 19, 80, 1, usec_trace);
            for (int i = 0; i < n_queries; ++i)
            {
                printf("%f\n", usec_trace[i]);
            }

            execute_workload(db, count, n_queries, 1, 69, 30, usec_trace);
            for (int i = 0; i < n_queries; ++i)
            {
                printf("%f\n", usec_trace[i]);
            }

            // execute_workload(db, count, n_queries, 0, 0, 5, usec_trace);
            // for (int i = 0; i < n_queries; ++i)
            // {
            //     printf("%f\n", usec_trace[i]);
            // }

            // execute_workload(db, count, n_queries, 1, 1, 1, usec_trace);
            // for (int i = 0; i < n_queries; ++i)
            // {
            //     printf("%f\n", usec_trace[i]);
            // }
        }

        db.close(0);
    // DbException is not subclassed from std::exception, so
    // need to catch both of these.
    } catch(DbException &e) {
        // Error handling code goes here    
        cerr << "Got db exception :(" << endl;
    } catch(std::exception &e) {
        // Error handling code goes here
    }

    string cmd = "rm ";
    assert(system((cmd + DB_PATH).c_str()) == 0);

    return 0;
}

