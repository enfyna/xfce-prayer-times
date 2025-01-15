#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#include "calculate-times.h"

FILE* f = NULL;
int total_fails = 0;

#define TEST_RUN_COUNT 10000

pt_args failed_cases[TEST_RUN_COUNT];
int failed_count = 0;

void* lib;

typedef pt_list (*get_list)(pt_args*);
get_list fun_get;

typedef pt_time_cstr (*get_str)(pt_time);
get_str fun_str;

typedef pt_time* (*get_next)(pt_list* pt_list);
get_next fun_next;

int load_lib()
{
    if (lib)
        dlclose(lib);

    lib = dlopen("../build/libprayer-times-plugin.so", RTLD_NOW);
    if (lib == NULL) {
        fprintf(stderr, "[ERROR] Couldnt load lib: %s", dlerror());
        return -1;
    }

    fun_get = dlsym(lib, "pt_get_list");
    if (fun_get == NULL) {
        fprintf(stderr, "[ERROR] Couldnt load fun_get: %s", dlerror());
        return -1;
    }

    fun_str = dlsym(lib, "pt_to_string");
    if (fun_get == NULL) {
        fprintf(stderr, "[ERROR] Couldnt load fun_str: %s", dlerror());
        return -1;
    }

    fun_next = dlsym(lib, "pt_next_prayer");
    if (fun_get == NULL) {
        fprintf(stderr, "[ERROR] Couldnt load fun_d_str: %s", dlerror());
        return -1;
    }
    return 0;
}

void test_pt_args() {
    pt_args args = {
        .latitude = (rand() % 181) - 90, // [90, -90]
        .longitude = (rand() % 361) - 180, // [180, -180]
        .elevation = (rand() % 9000), // [0, 9000]
        .fajr_angle = rand() % 8 + 12, // [12, 20]
        .isha_angle = rand() % 8 + 12, // [12, 20]
        .shadow_factor = (rand() % 2) + 1, // [1, 2]
    };

    pt_list list = fun_get(&args);
    bool did_fail = false;

    int prev_hour = 0;

    for (size_t i = 0; i < PT_TIME_COUNT; i++) {
        CU_ASSERT(list.items[i].HOUR >= prev_hour);
        CU_ASSERT(list.items[i].HOUR >= 0);
        CU_ASSERT(list.items[i].MINUTE >= 0);
        CU_ASSERT(list.items[i].SECOND >= 0);
        CU_ASSERT(list.items[i].HOUR < 25);
        CU_ASSERT(list.items[i].MINUTE < 60);
        CU_ASSERT(list.items[i].SECOND < 60);
        if (!(list.items[i].HOUR >= prev_hour && list.items[i].HOUR >= 0 && list.items[i].HOUR < 25 &&
              list.items[i].MINUTE >= 0 && list.items[i].MINUTE < 60 &&
              list.items[i].SECOND >= 0 && list.items[i].SECOND < 60)) {
            did_fail = true;
            break;
        }
        prev_hour = list.items[i].HOUR;
    }

    if (did_fail) {
        failed_cases[failed_count++] = args;
        total_fails++;
    }
}

void write_edge_cases_to_file() {
    if (!f) return;

    fprintf(f, "\
#include <stdio.h>\n\
#include <stddef.h>\n\
#include \"calculate-times.h\"\n\n\
pt_args* get_edge_cases() {\n\
\tstatic pt_args edge[] = {\n");

    for (int i = 0; i < failed_count; i++) {
        fprintf(f, "\t\t{.latitude = %f, .longitude = %f, .elevation = %f, .fajr_angle = %f, .isha_angle = %f, .shadow_factor = %f},\n",
                failed_cases[i].latitude, failed_cases[i].longitude, failed_cases[i].elevation,
                failed_cases[i].fajr_angle, failed_cases[i].isha_angle, failed_cases[i].shadow_factor);
    }

    fprintf(f, "\t};\n\treturn edge;\n}\n\n#define EDGE_CASE_COUNT %d\n\n\
int main(void) {\n\
\tpt_args* edge = get_edge_cases();\n\
\tfor (int i = 0; i < EDGE_CASE_COUNT; i++) {\n\
\t\tpt_list list = pt_get_list(&edge[i]);\n\
\t\tfor (size_t j = 0; j < PT_TIME_COUNT; j++) {\n\
\t\t\tprintf(\"%%zu: %%s\\n\", j, pt_to_string(list.items[j]).data);\n\
\t\t}\n\
\t\tprintf(\"---\\n\");\n\
\t}\n\
}\n", failed_count);

    fclose(f);
}

int main(void) {
    int res = load_lib();
    if (res != 0) {
        return res;
    }

    srand(time(NULL)); // Use srand instead of srandom for portability

    f = fopen("edge_cases.c", "w");
    if (!f) {
        perror("Failed to open edge_cases.c");
        return EXIT_FAILURE;
    }

    CU_initialize_registry();
    CU_pSuite suite = CU_add_suite("AddTestSuite", 0, 0);
    for (int i = 0; i < TEST_RUN_COUNT; i++) {
        CU_add_test(suite, "test pt calculations", test_pt_args);
    }

    CU_basic_run_tests();
    CU_cleanup_registry();

    write_edge_cases_to_file();

    return EXIT_SUCCESS;
}

