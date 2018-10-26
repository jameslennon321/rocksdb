include ../make_config.mk

ifndef DISABLE_JEMALLOC
	ifdef JEMALLOC
		PLATFORM_CXXFLAGS += -DROCKSDB_JEMALLOC -DJEMALLOC_NO_DEMANGLE
	endif
	EXEC_LDFLAGS := $(JEMALLOC_LIB) $(EXEC_LDFLAGS) -lpthread
	PLATFORM_CXXFLAGS += $(JEMALLOC_INCLUDE)
endif

ifneq ($(USE_RTTI), 1)
	CXXFLAGS += -fno-rtti
endif

.PHONY: clean librocksdb

all: rocksdb_workload berkeleydb_workload

berkeleydb_workload: berkeleydb_workload.cpp setup_data.cc
	$(CXX) setup_data.cc $@.cpp -std=c++11 -ldb_cxx -o$@

rocksdb_workload: librocksdb rocksdb_workload.cc setup_data.cc
	$(CXX) $(CXXFLAGS) $@.cc setup_data.cc -o$@ ../librocksdb.a -I../include -O2 -std=c++11 $(PLATFORM_LDFLAGS) $(PLATFORM_CXXFLAGS) $(EXEC_LDFLAGS)

setup_data: librocksdb setup_data.cc
	$(CXX) $(CXXFLAGS) $@.cc -o$@ ../librocksdb.a -I../include -O2 -std=c++11 $(PLATFORM_LDFLAGS) $(PLATFORM_CXXFLAGS) $(EXEC_LDFLAGS)

clean:
	rm -rf ./simple_example ./column_families_example ./compact_files_example ./compaction_filter_example ./c_simple_example c_simple_example.o ./optimistic_transaction_example ./transaction_example ./options_file_example

librocksdb:
	cd .. && $(MAKE) static_lib
