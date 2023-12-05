#include "db.h"
#include <json/value.h>
#ifdef TESTING_NWC
#include <iostream>
#endif
 ROCKSDB_NAMESPACE::Options options_from_json(const Json::Value& json) {
 ROCKSDB_NAMESPACE::Options options;
 for(Json::Value::const_iterator itr=json.begin(); itr!=json.end(); ++itr){
    int opt_int;
    uint64_t opt_uint64;
    unsigned char opt_bool;
    const std::string key = itr.key().asString();
    /* int options */
    if(key == "increase_parallelism") {
      opt_int = (*itr).asInt();
      options.IncreaseParallelism( opt_int);
    }
    else if(key == "info_log_level") {
      opt_int = (*itr).asInt();
      auto int2enum= [](int k) {
        switch(k){
          default:
          case 0: return ROCKSDB_NAMESPACE::DEBUG_LEVEL ;
          case 1: return ROCKSDB_NAMESPACE::INFO_LEVEL;
          case 2: return ROCKSDB_NAMESPACE::WARN_LEVEL;
          case 3: return ROCKSDB_NAMESPACE::ERROR_LEVEL;
          case 4: return ROCKSDB_NAMESPACE::FATAL_LEVEL;
          case 5: return ROCKSDB_NAMESPACE::HEADER_LEVEL;
        }};
      options.info_log_level= int2enum(opt_int); 
    }
    else if(key == "max_open_files") {
      opt_int = (*itr).asInt();
      options.max_open_files = opt_int;
    }
    else if(key == "num_levels") {
      opt_int = (*itr).asInt();
      options.num_levels = opt_int;
    }
    else if(key == "level0_file_num_compaction_trigger") {
      opt_int = (*itr).asInt();
      options.level0_file_num_compaction_trigger= opt_int;
    }
    else if(key == "level0_slowdown_writes_trigger") {
      opt_int = (*itr).asInt();
      options.level0_slowdown_writes_trigger= opt_int;
    }
    else if(key == "level0_stop_writes_trigger") {
      opt_int = (*itr).asInt();
      options.level0_stop_writes_trigger= opt_int;
    }
    else if(key == "target_file_size_multiplier") {
      opt_int = (*itr).asInt();
      options.target_file_size_multiplier= opt_int;
    }
    else if(key == "max_write_buffer_number") {
      opt_int = (*itr).asInt();
      options.max_write_buffer_number= opt_int;
    }
    else if(key == "min_write_buffer_number_to_merge") {
      opt_int = (*itr).asInt();
      options.min_write_buffer_number_to_merge= opt_int;
    }
    else if(key == "max_write_buffer_number_to_maintain") {
      opt_int = (*itr).asInt();
      options.max_write_buffer_number_to_maintain= opt_int;
    }
    else if(key == "max_background_compactions") {
      opt_int = (*itr).asInt();
      options.max_background_compactions= opt_int;
    }
    else if(key == "max_background_flushes") {
      opt_int = (*itr).asInt();
      options.max_background_flushes= opt_int;
    }
    else if(key == "table_cache_numshardbits") {
      opt_int = (*itr).asInt();
      options.table_cache_numshardbits= opt_int;
    }
    else if(key == "use_fsync") {
      opt_int = (*itr).asInt();
      options.use_fsync= opt_int;
    }
    else if(key == "access_hint_on_compaction_start") {
      opt_int = (*itr).asInt();
      auto int2enum= [](int k) {
        switch(k){
          case 0: return ROCKSDB_NAMESPACE::Options::NONE ;
          default:
          case 1: return ROCKSDB_NAMESPACE::Options::NORMAL;
          case 2: return ROCKSDB_NAMESPACE::Options::SEQUENTIAL;
          case 3: return ROCKSDB_NAMESPACE::Options::WILLNEED ;
        }};
      options.access_hint_on_compaction_start= int2enum(opt_int);
    }
    else if(key == "disable_auto_compactions") {
      opt_int = (*itr).asInt();
      options.disable_auto_compactions= opt_int;
    }
    else if(key == "report_bg_io_stats") {
      opt_int = (*itr).asInt();
      options.report_bg_io_stats= opt_int;
    }
    else if(key == "compression") {
      opt_int = (*itr).asInt();
      auto int2enum= [](int k) {
        switch(k){
          default:
          case 0 : return  ROCKSDB_NAMESPACE::kNoCompression;
          case 1 : return  ROCKSDB_NAMESPACE::kSnappyCompression;
          case 2 : return  ROCKSDB_NAMESPACE::kZlibCompression;
          case 3 : return  ROCKSDB_NAMESPACE::kBZip2Compression;
          case 4 : return  ROCKSDB_NAMESPACE::kLZ4Compression;
          case 5 : return  ROCKSDB_NAMESPACE::kLZ4HCCompression;
          case 6 : return  ROCKSDB_NAMESPACE::kXpressCompression;
          case 7 : return  ROCKSDB_NAMESPACE::kZSTD;
          case 8 : return  ROCKSDB_NAMESPACE::kZSTDNotFinalCompression;
          case 9 : return  ROCKSDB_NAMESPACE::kDisableCompressionOption;
        }
      };
      options.compression= int2enum(opt_int);
#ifdef TESTING_NWC
      std::cout << "compression = " << opt_int << std::endl;
#endif
    }
    else if(key == "compaction_style") {
      opt_int = (*itr).asInt();
      //options.compaction_style= opt_int;
    }
    /* bool options */
    else if(key == "create_if_missing") {
      opt_bool = (*itr).asBool();
      options.create_if_missing= opt_bool;
    }
    else if(key == "create_missing_column_families") {
      opt_bool = (*itr).asBool();
      options.create_missing_column_families= opt_bool;
    }
    else if(key == "error_if_exists") {
      opt_bool = (*itr).asBool();
      options.error_if_exists= opt_bool;
    }
    else if(key == "paranoid_checks") {
      opt_bool = (*itr).asBool();
      options.paranoid_checks= opt_bool;
    }
    else if(key == "allow_mmap_reads") {
      opt_bool = (*itr).asBool();
      options.allow_mmap_reads= opt_bool;
    }
    else if(key == "allow_mmap_writes") {
      opt_bool = (*itr).asBool();
      options.allow_mmap_writes= opt_bool;
    }
    else if(key == "is_fd_close_on_exec") {
      opt_bool = (*itr).asBool();
      options.is_fd_close_on_exec= opt_bool;
    }
    else if(key == "advise_random_on_open") {
      opt_bool = (*itr).asBool();
      options.advise_random_on_open= opt_bool;
    }
    else if(key == "use_adaptive_mutex") {
      opt_bool = (*itr).asBool();
      options.use_adaptive_mutex= opt_bool;
    }
    else if(key == "inplace_update_support") {
      opt_bool = (*itr).asBool();
      options.inplace_update_support= opt_bool;
    }
    /* uint64 options */
    else if(key == "optimize_for_point_lookup") {
      opt_uint64 = (*itr).asUInt64();
      options.OptimizeForPointLookup(opt_uint64);
    }
    else if(key == "optimize_level_style_compaction") {
      opt_uint64 = (*itr).asUInt64();
      options.OptimizeLevelStyleCompaction (opt_uint64);
    }
    else if(key == "optimize_universal_style_compaction") {
      opt_uint64 = (*itr).asUInt64();
      options.OptimizeUniversalStyleCompaction (opt_uint64);
    }
    else if(key == "max_total_wal_size") {
      opt_uint64 = (*itr).asUInt64();
      options.max_total_wal_size=opt_uint64;
    }
    else if(key == "target_file_size_base") {
      opt_uint64 = (*itr).asUInt64();
      options.target_file_size_base=opt_uint64;
    }
    else if(key == "max_bytes_for_level_base") {
      opt_uint64 = (*itr).asUInt64();
      options.max_bytes_for_level_base=opt_uint64;
    }
    else if(key == "WAL_ttl_seconds") {
      opt_uint64 = (*itr).asUInt64();
      options.WAL_ttl_seconds=opt_uint64;
    }
    else if(key == "WAL_size_limit_MB") {
      opt_uint64 = (*itr).asUInt64();
      options.WAL_size_limit_MB=opt_uint64;
    }
    else if(key == "bytes_per_sync") {
      opt_uint64 = (*itr).asUInt64();
      options.bytes_per_sync=opt_uint64;
    }
    else if(key == "max_sequential_skip_in_iterations") {
      opt_uint64 = (*itr).asUInt64();
      options.max_sequential_skip_in_iterations=opt_uint64;
    }
    else if(key == "delete_obsolete_files_period_micros") {
      opt_uint64 = (*itr).asUInt64();
      options.delete_obsolete_files_period_micros=opt_uint64;
    }
  }
  return options;
}
