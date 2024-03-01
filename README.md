A Service that harbours a Rocksdb database

Language: C++

Port: 2153

Message format: JSON

Dependencies: asio (hhtpS://think-async.com), jsconcpp (https://github.com/open-source-parsers/jsoncpp)
They are both included here. 

Creation of the database 
The options used in the creation of the database are described in config.json.

Design
The config file lists all the databases that are meant to be open by the server. The server listens on a port specified on the commandline. I use 2153 for test.
Listed below are the available options. Checks RocksDB docs to find out what they mean.
Invalid option keys are ignored. 
* "increase_parallelism" = <int>
*  "info_log_level"= between 0 and 5 as listed below
        -  0: DEBUG;
        -  1: INFO;
        -  2: WARN;
        -  3: ERROR;
        -  4: FATAL;
        -  5: HEADER;
* "max_open_files" = <int>
* "num_levels" = <int> 
* "level0_file_num_compaction_trigger" = <int>
* "level0_slowdown_writes_trigger" = <int>
* "level0_stop_writes_trigger" = <int>
* "target_file_size_multiplier" = <int>
* "max_write_buffer_number" = <int>
* "min_write_buffer_number_to_merge" = <int>
* "max_write_buffer_number_to_maintain" = <int>
* "max_background_compactions" = <int>
* "max_background_flushes" = <int>
* "table_cache_numshardbits" = <int>
* "use_fsync" = <int>
* "access_hint_on_compaction_start" = between 0 and 4 as listed below
          0: NONE ;
          1: NORMAL;
          2: SEQUENTIAL;
          3: WILLNEED ;
* "disable_auto_compactions" = <int>
* "report_bg_io_stats" = <int>
* "compression" = between 0 and 9 as listed below
            0 : No Compression;
            1 : Snappy Compression;
            2 : Zlib Compression;
            3 : BZip2 Compression;
            4 : LZ4 Compression;
            5 : LZ4H CCompression;
            6 : Xpress Compression;
            7 : ZSTD;
            8 : ZSTDNotFinal Compression;
            9 : Disable CompressionOption;
* __bool options__ 
* "create_if_missing" = <bool>
* "create_missing_column_families" = <bool>
* "error_if_exists" = <bool>
* "paranoid_checks" = <bool>
* "allow_mmap_reads" = <bool>
* "allow_mmap_writes" = <bool>
* "is_fd_close_on_exec" = <bool>
* "advise_random_on_open" = <bool>
* "use_adaptive_mutex" = <bool>
* "inplace_update_support" = <bool>
*  __uint64 options__
* "optimize_for_point_lookup" = <unit64>
* "optimize_level_style_compaction" = <unit64>
* "optimize_universal_style_compaction" = <unit64>
* "max_total_wal_size" = <uint64>
* "target_file_size_base" = <uint64>
* "max_bytes_for_level_base" = <uint64>
* "WAL_ttl_seconds" = <uint64>
* "WAL_size_limit_MB" = <uint64>
* "bytes_per_sync" = <uint64>
* "max_sequential_skip_in_iterations" = <uint64>
* "delete_obsolete_files_period_micros" = <uint64>


I use Catchtest as my test framework. The catchtest.cc lists all the five functions currently iimplemented.
These are
  - put (put a key-value pair)
  - get (get a value given a key)
  - put_cf (put a key-value pair in a named column-family)
  - get_cf(get a value give a key and column-family name)
  - write_batch (put a list of key value pairs)






