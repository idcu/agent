#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include "msg_bus.h"
#include "coroutine.h"
#include "idcu/memory/memory_pool.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#include <unistd.h>
#endif

typedef enum {
    OUTPUT_FORMAT_TEXT,
    OUTPUT_FORMAT_CSV,
    OUTPUT_FORMAT_JSON
} OutputFormat;

typedef struct {
    uint64_t* durations;
    uint64_t count;
    uint64_t capacity;
    uint64_t min_us;
    uint64_t max_us;
    uint64_t sum_us;
    double mean_us;
    double stddev_us;
} BenchmarkStats;

static idcu_MessageBus g_bus;
static idcu_CoroScheduler g_sched;
static idcu_MemoryPool g_pool;
static OutputFormat g_output_format = OUTPUT_FORMAT_TEXT;

static uint64_t get_timestamp_us(void) {
#ifdef _WIN32
    LARGE_INTEGER freq, count;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&count);
    return (uint64_t)(count.QuadPart * 1000000 / freq.QuadPart);
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000 + (uint64_t)ts.tv_nsec / 1000;
#endif
}



static void bench_stats_init(BenchmarkStats* stats, uint64_t capacity) {
    stats->durations = (uint64_t*)malloc(capacity * sizeof(uint64_t));
    stats->count = 0;
    stats->capacity = capacity;
    stats->min_us = UINT64_MAX;
    stats->max_us = 0;
    stats->sum_us = 0;
    stats->mean_us = 0.0;
    stats->stddev_us = 0.0;
}

static void bench_stats_add(BenchmarkStats* stats, uint64_t duration_us) {
    if (stats->count < stats->capacity) {
        stats->durations[stats->count++] = duration_us;
    }
    if (duration_us < stats->min_us) stats->min_us = duration_us;
    if (duration_us > stats->max_us) stats->max_us = duration_us;
    stats->sum_us += duration_us;
}

static void bench_stats_calculate(BenchmarkStats* stats) {
    if (stats->count == 0) return;
    
    stats->mean_us = (double)stats->sum_us / stats->count;
    
    double sum_sq_diff = 0.0;
    for (uint64_t i = 0; i < stats->count; i++) {
        double diff = (double)stats->durations[i] - stats->mean_us;
        sum_sq_diff += diff * diff;
    }
    stats->stddev_us = sqrt(sum_sq_diff / stats->count);
}

static void bench_stats_destroy(BenchmarkStats* stats) {
    free(stats->durations);
}

typedef void (*BenchmarkFunc)(uint64_t);

typedef struct {
    const char* name;
    BenchmarkFunc func;
    uint64_t iterations;
    uint64_t warmup_iterations;
    uint64_t runs;
    BenchmarkStats stats;
} Benchmark;

#define MAX_BENCHMARKS 32

typedef struct {
    Benchmark benchmarks[MAX_BENCHMARKS];
    int count;
    const char* name;
} BenchmarkSuite;

static void bench_suite_init(BenchmarkSuite* suite, const char* name) {
    suite->name = name;
    suite->count = 0;
}

static void bench_suite_add_benchmark(BenchmarkSuite* suite, const char* name, 
                                       BenchmarkFunc func, uint64_t iterations,
                                       uint64_t warmup_iterations, uint64_t runs) {
    if (suite->count >= MAX_BENCHMARKS) return;
    Benchmark* b = &suite->benchmarks[suite->count];
    b->name = name;
    b->func = func;
    b->iterations = iterations;
    b->warmup_iterations = warmup_iterations;
    b->runs = runs;
    bench_stats_init(&b->stats, runs);
    suite->count++;
}

static void bench_suite_run(BenchmarkSuite* suite) {
    for (int i = 0; i < suite->count; i++) {
        Benchmark* b = &suite->benchmarks[i];
        
        if (b->warmup_iterations > 0) {
            b->func(b->warmup_iterations);
        }
        
        for (uint64_t run = 0; run < b->runs; run++) {
            uint64_t start = get_timestamp_us();
            b->func(b->iterations);
            uint64_t end = get_timestamp_us();
            bench_stats_add(&b->stats, end - start);
        }
        bench_stats_calculate(&b->stats);
    }
}

static void bench_suite_print_text(BenchmarkSuite* suite) {
    printf("\n%s Benchmark Results:\n", suite->name);
    printf("--------------------------------------------------------------------------------------------------------\n");
    printf("%-20s %12s %12s %12s %12s %12s %12s\n", 
           "Benchmark", "Iterations", "Runs", "Mean(us)", "Min(us)", "Max(us)", "Ops/sec");
    printf("--------------------------------------------------------------------------------------------------------\n");
    
    for (int i = 0; i < suite->count; i++) {
        Benchmark* b = &suite->benchmarks[i];
        double ops_per_sec = (b->stats.mean_us > 0) ? 
            (double)b->iterations * 1000000.0 / b->stats.mean_us : 0.0;
        printf("%-20s %12llu %12llu %12.0f %12llu %12llu %12.0f\n", 
               b->name, 
               (unsigned long long)b->iterations,
               (unsigned long long)b->runs,
               b->stats.mean_us,
               (unsigned long long)b->stats.min_us,
               (unsigned long long)b->stats.max_us,
               ops_per_sec);
    }
    printf("--------------------------------------------------------------------------------------------------------\n");
}

static void bench_suite_print_csv(BenchmarkSuite* suite) {
    printf("Benchmark,Iterations,Runs,Mean(us),Min(us),Max(us),StdDev(us),Ops/sec\n");
    for (int i = 0; i < suite->count; i++) {
        Benchmark* b = &suite->benchmarks[i];
        double ops_per_sec = (b->stats.mean_us > 0) ? 
            (double)b->iterations * 1000000.0 / b->stats.mean_us : 0.0;
        printf("%s,%llu,%llu,%.2f,%llu,%llu,%.2f,%.0f\n",
               b->name,
               (unsigned long long)b->iterations,
               (unsigned long long)b->runs,
               b->stats.mean_us,
               (unsigned long long)b->stats.min_us,
               (unsigned long long)b->stats.max_us,
               b->stats.stddev_us,
               ops_per_sec);
    }
}

static void bench_suite_print_json(BenchmarkSuite* suite) {
    printf("{\"suite\":\"%s\",\"benchmarks\":[", suite->name);
    for (int i = 0; i < suite->count; i++) {
        Benchmark* b = &suite->benchmarks[i];
        double ops_per_sec = (b->stats.mean_us > 0) ? 
            (double)b->iterations * 1000000.0 / b->stats.mean_us : 0.0;
        if (i > 0) printf(",");
        printf("{\"name\":\"%s\",\"iterations\":%llu,\"runs\":%llu,"
               "\"mean_us\":%.2f,\"min_us\":%llu,\"max_us\":%llu,"
               "\"stddev_us\":%.2f,\"ops_per_sec\":%.0f}",
               b->name,
               (unsigned long long)b->iterations,
               (unsigned long long)b->runs,
               b->stats.mean_us,
               (unsigned long long)b->stats.min_us,
               (unsigned long long)b->stats.max_us,
               b->stats.stddev_us,
               ops_per_sec);
    }
    printf("]}\n");
}

static void bench_suite_print_summary(BenchmarkSuite* suite) {
    switch (g_output_format) {
        case OUTPUT_FORMAT_TEXT:
            bench_suite_print_text(suite);
            break;
        case OUTPUT_FORMAT_CSV:
            bench_suite_print_csv(suite);
            break;
        case OUTPUT_FORMAT_JSON:
            bench_suite_print_json(suite);
            break;
    }
}

static void bench_suite_destroy(BenchmarkSuite* suite) {
    for (int i = 0; i < suite->count; i++) {
        bench_stats_destroy(&suite->benchmarks[i].stats);
    }
}

static void bench_msg_send(uint64_t iterations) {
    idcu_StackContext ctx;
    memset(&ctx, 0, sizeof(ctx));
    
    for (uint64_t i = 0; i < iterations; i++) {
        idcu_msg_send(&g_bus, 1, 2, IDCU_MSG_PRIO_NORMAL, &ctx);
    }
}

static void bench_msg_recv(uint64_t iterations) {
    idcu_Message msg;
    
    for (uint64_t i = 0; i < iterations; i++) {
        idcu_msg_recv(&g_bus, 2, &msg);
    }
}

static void bench_msg_batch_send(uint64_t iterations) {
    idcu_MessageBatch batch;
    idcu_StackContext ctx;
    memset(&ctx, 0, sizeof(ctx));
    
    for (uint64_t i = 0; i < iterations; i += IDCU_MSG_BATCH_MAX) {
        batch.count = 0;
        for (uint32_t j = 0; j < IDCU_MSG_BATCH_MAX && (i + j) < iterations; j++) {
            batch.msgs[j].source_mod_id = 1;
            batch.msgs[j].target_mod_id = 2;
            batch.msgs[j].priority = IDCU_MSG_PRIO_NORMAL;
            batch.msgs[j].data = ctx;
            batch.msgs[j].payload = NULL;
            batch.count++;
        }
        idcu_msg_send_batch(&g_bus, &batch);
    }
}

static void bench_msg_roundtrip(uint64_t iterations) {
    idcu_StackContext ctx;
    idcu_Message msg;
    memset(&ctx, 0, sizeof(ctx));
    
    for (uint64_t i = 0; i < iterations; i++) {
        idcu_msg_send(&g_bus, 1, 2, IDCU_MSG_PRIO_NORMAL, &ctx);
        idcu_msg_recv(&g_bus, 2, &msg);
    }
}

static void bench_memory_pool_alloc(uint64_t iterations) {
    void* ptrs[1024];
    uint32_t ptr_count = 0;
    
    for (uint64_t i = 0; i < iterations; i++) {
        if (ptr_count < 1024) {
            ptrs[ptr_count++] = idcu_mem_pool_alloc(&g_pool, 64);
        } else {
            idcu_mem_pool_free(&g_pool, ptrs[0]);
            for (uint32_t j = 1; j < 1024; j++) {
                ptrs[j - 1] = ptrs[j];
            }
            ptrs[1023] = idcu_mem_pool_alloc(&g_pool, 64);
        }
    }
    
    for (uint32_t i = 0; i < ptr_count; i++) {
        idcu_mem_pool_free(&g_pool, ptrs[i]);
    }
}

static void bench_memory_pool_alloc_free(uint64_t iterations) {
    for (uint64_t i = 0; i < iterations; i++) {
        void* ptr = idcu_mem_pool_alloc(&g_pool, 64);
        idcu_mem_pool_free(&g_pool, ptr);
    }
}

static idcu_CoroState test_coro_func(idcu_Coroutine* coro) {
    (void)coro;
    return IDCU_CORO_FINISHED;
}

static idcu_CoroState test_coro_func_simple(idcu_Coroutine* coro) {
    (void)coro;
    return IDCU_CORO_FINISHED;
}

static void bench_coro_create(uint64_t iterations) {
    for (uint64_t i = 0; i < iterations; i++) {
        int id = idcu_coro_create(&g_sched, test_coro_func, 32, 10, NULL);
        if (id > 0) {
            idcu_coro_destroy(&g_sched, id);
        }
    }
}

static void bench_coro_switch(uint64_t iterations) {
    int id = idcu_coro_create(&g_sched, test_coro_func_simple, 32, 10, NULL);
    if (id <= 0) return;
    
    for (uint64_t i = 0; i < iterations; i++) {
        idcu_coro_resume(&g_sched, id);
    }
    
    idcu_coro_destroy(&g_sched, id);
}

static void bench_coro_schedule(uint64_t iterations) {
    int ids[32];
    for (int i = 0; i < 32; i++) {
        ids[i] = idcu_coro_create(&g_sched, test_coro_func_simple, 32, 10, NULL);
    }
    
    for (uint64_t i = 0; i < iterations; i++) {
        for (int j = 0; j < 32; j++) {
            if (ids[j] > 0) {
                idcu_coro_resume(&g_sched, ids[j]);
            }
        }
    }
    
    for (int i = 0; i < 32; i++) {
        if (ids[i] > 0) {
            idcu_coro_destroy(&g_sched, ids[i]);
        }
    }
}

static char* g_baseline_file = NULL;
static char* g_save_baseline = NULL;
static int g_check_regression = 0;
static double g_regression_threshold = 10.0;

static int bench_suite_save_baseline(BenchmarkSuite* suite, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to open baseline file for writing");
        return -1;
    }
    
    fprintf(fp, "{\n");
    fprintf(fp, "  \"suite\": \"%s\",\n", suite->name);
    fprintf(fp, "  \"benchmarks\": [\n");
    
    for (int i = 0; i < suite->count; i++) {
        Benchmark* b = &suite->benchmarks[i];
        double ops_per_sec = (b->stats.mean_us > 0) ? 
            (double)b->iterations * 1000000.0 / b->stats.mean_us : 0.0;
        
        fprintf(fp, "    {\n");
        fprintf(fp, "      \"name\": \"%s\",\n", b->name);
        fprintf(fp, "      \"mean_us\": %.2f,\n", b->stats.mean_us);
        fprintf(fp, "      \"min_us\": %llu,\n", (unsigned long long)b->stats.min_us);
        fprintf(fp, "      \"max_us\": %llu,\n", (unsigned long long)b->stats.max_us);
        fprintf(fp, "      \"stddev_us\": %.2f,\n", b->stats.stddev_us);
        fprintf(fp, "      \"ops_per_sec\": %.0f\n", ops_per_sec);
        fprintf(fp, "    }%s\n", (i < suite->count - 1) ? "," : "");
    }
    
    fprintf(fp, "  ]\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return 0;
}

typedef struct {
    char name[256];
    double mean_us;
    double ops_per_sec;
} BaselineBenchmark;

typedef struct {
    BaselineBenchmark benchmarks[MAX_BENCHMARKS];
    int count;
} BaselineData;

static int bench_suite_load_baseline(const char* filename, BaselineData* baseline) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        perror("Failed to open baseline file for reading");
        return -1;
    }
    
    baseline->count = 0;
    char line[1024];
    char name[256];
    double mean_us, ops_per_sec;
    
    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "      \"name\": \"%255[^\"]\",", name) == 1) {
            strncpy(baseline->benchmarks[baseline->count].name, name, 255);
            baseline->benchmarks[baseline->count].name[255] = '\0';
        } else if (sscanf(line, "      \"mean_us\": %lf,", &mean_us) == 1) {
            baseline->benchmarks[baseline->count].mean_us = mean_us;
        } else if (sscanf(line, "      \"ops_per_sec\": %lf", &ops_per_sec) == 1) {
            baseline->benchmarks[baseline->count].ops_per_sec = ops_per_sec;
            baseline->count++;
            if (baseline->count >= MAX_BENCHMARKS) break;
        }
    }
    
    fclose(fp);
    return 0;
}

static int bench_suite_check_regression(BenchmarkSuite* suite, BaselineData* baseline, double threshold) {
    int regressions_found = 0;
    
    printf("\n=== Performance Regression Check ===\n");
    printf("Threshold: %.1f%%\n\n", threshold);
    printf("%-20s %15s %15s %15s %10s\n", 
           "Benchmark", "Baseline(ops/s)", "Current(ops/s)", "Change", "Status");
    printf("--------------------------------------------------------------------------------\n");
    
    for (int i = 0; i < suite->count; i++) {
        Benchmark* b = &suite->benchmarks[i];
        double current_ops = (b->stats.mean_us > 0) ? 
            (double)b->iterations * 1000000.0 / b->stats.mean_us : 0.0;
        
        BaselineBenchmark* base = NULL;
        for (int j = 0; j < baseline->count; j++) {
            if (strcmp(b->name, baseline->benchmarks[j].name) == 0) {
                base = &baseline->benchmarks[j];
                break;
            }
        }
        
        if (base) {
            double change = 0.0;
            if (base->ops_per_sec > 0) {
                change = ((current_ops - base->ops_per_sec) / base->ops_per_sec) * 100.0;
            }
            
            int is_regression = (change < -threshold);
            if (is_regression) regressions_found++;
            
            printf("%-20s %15.0f %15.0f %14.1f%% %10s\n",
                   b->name,
                   base->ops_per_sec,
                   current_ops,
                   change,
                   is_regression ? "REGRESSION" : "OK");
        } else {
            printf("%-20s %15s %15.0f %15s %10s\n",
                   b->name,
                   "N/A",
                   current_ops,
                   "N/A",
                   "NEW");
        }
    }
    
    printf("--------------------------------------------------------------------------------\n");
    if (regressions_found > 0) {
        printf("Found %d performance regression(s)!\n", regressions_found);
    } else {
        printf("No performance regressions found.\n");
    }
    printf("====================================\n\n");
    
    return regressions_found;
}

static void print_usage(const char* program_name) {
    printf("Usage: %s [options]\n", program_name);
    printf("Options:\n");
    printf("  -h, --help              Show this help message\n");
    printf("  -f, --format <format>   Output format (text, csv, json) [default: text]\n");
    printf("  -q, --quick             Quick benchmark (fewer iterations)\n");
    printf("  -s, --save <file>       Save baseline results to file\n");
    printf("  -b, --baseline <file>   Load baseline from file\n");
    printf("  -c, --check             Check for performance regression\n");
    printf("  -t, --threshold <pct>   Regression threshold percentage [default: 10.0]\n");
    printf("\n");
}

int main(int argc, char* argv[]) {
    int quick_mode = 0;
    int ret = 0;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--format") == 0) {
            if (i + 1 < argc) {
                if (strcmp(argv[i + 1], "csv") == 0) {
                    g_output_format = OUTPUT_FORMAT_CSV;
                } else if (strcmp(argv[i + 1], "json") == 0) {
                    g_output_format = OUTPUT_FORMAT_JSON;
                } else if (strcmp(argv[i + 1], "text") == 0) {
                    g_output_format = OUTPUT_FORMAT_TEXT;
                }
                i++;
            }
        } else if (strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--quick") == 0) {
            quick_mode = 1;
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--save") == 0) {
            if (i + 1 < argc) {
                g_save_baseline = argv[i + 1];
                i++;
            }
        } else if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--baseline") == 0) {
            if (i + 1 < argc) {
                g_baseline_file = argv[i + 1];
                i++;
            }
        } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--check") == 0) {
            g_check_regression = 1;
        } else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--threshold") == 0) {
            if (i + 1 < argc) {
                g_regression_threshold = atof(argv[i + 1]);
                i++;
            }
        }
    }
    
    if (g_output_format == OUTPUT_FORMAT_TEXT) {
        printf("IDCU Agent - Performance Benchmarks\n");
        printf("====================================\n");
    }
    
    idcu_msg_bus_init(&g_bus);
    idcu_coro_sched_init(&g_sched);
    idcu_mem_pool_init(&g_pool);
    
    uint64_t msg_iterations = quick_mode ? 10000 : 100000;
    uint64_t mem_iterations = quick_mode ? 10000 : 100000;
    uint64_t coro_iterations = quick_mode ? 1000 : 10000;
    uint64_t warmup_iterations = quick_mode ? 1000 : 10000;
    uint64_t runs = quick_mode ? 3 : 5;
    
    BenchmarkSuite suite;
    bench_suite_init(&suite, "Core Performance");
    
    bench_suite_add_benchmark(&suite, "msg_send", bench_msg_send, 
                              msg_iterations, warmup_iterations, runs);
    bench_suite_add_benchmark(&suite, "msg_recv", bench_msg_recv, 
                              msg_iterations, warmup_iterations, runs);
    bench_suite_add_benchmark(&suite, "msg_roundtrip", bench_msg_roundtrip, 
                              msg_iterations, warmup_iterations, runs);
    bench_suite_add_benchmark(&suite, "msg_batch_send", bench_msg_batch_send, 
                              msg_iterations, warmup_iterations, runs);
    bench_suite_add_benchmark(&suite, "memory_pool_alloc", bench_memory_pool_alloc, 
                              mem_iterations, warmup_iterations, runs);
    bench_suite_add_benchmark(&suite, "memory_pool_alloc_free", bench_memory_pool_alloc_free, 
                              mem_iterations, warmup_iterations, runs);
    bench_suite_add_benchmark(&suite, "coro_create_destroy", bench_coro_create, 
                              coro_iterations, warmup_iterations, runs);
    bench_suite_add_benchmark(&suite, "coro_switch", bench_coro_switch, 
                              coro_iterations * 10, warmup_iterations, runs);
    bench_suite_add_benchmark(&suite, "coro_schedule", bench_coro_schedule, 
                              coro_iterations, warmup_iterations, runs);
    
    bench_suite_run(&suite);
    bench_suite_print_summary(&suite);
    
    if (g_save_baseline) {
        if (g_output_format == OUTPUT_FORMAT_TEXT) {
            printf("\nSaving baseline to %s...\n", g_save_baseline);
        }
        if (bench_suite_save_baseline(&suite, g_save_baseline) != 0) {
            ret = 1;
        }
    }
    
    if (g_check_regression && g_baseline_file) {
        BaselineData baseline;
        if (bench_suite_load_baseline(g_baseline_file, &baseline) == 0) {
            int regressions = bench_suite_check_regression(&suite, &baseline, g_regression_threshold);
            if (regressions > 0) {
                ret = 1;
            }
        } else {
            fprintf(stderr, "Failed to load baseline file: %s\n", g_baseline_file);
            ret = 1;
        }
    }
    
    bench_suite_destroy(&suite);
    
    idcu_mem_pool_destroy(&g_pool);
    idcu_coro_sched_destroy(&g_sched);
    idcu_msg_bus_destroy(&g_bus);
    
    if (g_output_format == OUTPUT_FORMAT_TEXT) {
        printf("\nBenchmark completed!\n");
    }
    return ret;
}
