#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xjson.h"

static int test_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ_BASE(equality, expect, actual, format)\
        do {\
                test_count++;\
                if (equality)\
                        test_pass++;\
                else {\
                        fprintf(stderr, "[%s:%d] expect: " format ", actual: "\
                                format "\n", __FILE__, __LINE__,\
                                expect, actual);\
                        test_ret = 1;\
                }\
        }\while(0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual),\
                expect, actual, "%d")

static void test_parse_null() {
        xjson_value v;
        v.type = XJSON_FALSE;
        EXPECT_EQ_INT(XJSON_PARSE_OK, xjson_parse(&v, "null"));
        EXPECT_EQ_INT(XJSON_NULL, xjson_get_type(&v));
}

static void test_parse_expect_value() {
        xjson_value v;
        v.type = XJSON_FALSE;
        EXPECT_EQ_INT(XJSON_PARSE_EXPECT_VALUE, xjson_parse(&v, ""));
        EXPECT_EQ_INT(XJSON_NULL, xjson_get_type(&v));

        EXPECT_EQ_INT(XJSON_PARSE_EXPECT_VALUE, xjson_parse(&v, " "));
        EXPECT_EQ_INT(XJSON_NULL, xjson_get_type(&v));
} 

static void test_parse_invalid_value() {
        xjson_value v;
        v.type = XJSON_FALSE;
        EXPECT_EQ_INT(XJSON_PARSE_INVALID_VALUE, xjson_parse(&v, "nul"));
        EXPECT_EQ_INT(XJSON_NULL, xjson_get_type(&v));

        v.type = XJSON_FALSE;
        EXPECT_EQ_INT(XJSON_PARSE_INVALID_VALUE, xjson_parse(&v, "?"));
        EXPECT_EQ_INT(XJSON_NULL, xjson_get_type(&v));
}

static void test_parse_root_not_singular() {
        xjson_value v;
        v.type = XJSON_FALSE;
        EXPECT_EQ_INT(XJSON_PARSE_ROOT_NOT_SINGULAR, xjson_parse(&v, "null x"));
        EXPECT_EQ_INT(XJSON_NULL, xjson_get_type(&v));
}

static void test_parse() {
        test_parse_null();
        test_parse_expect_value();
        test_parse_invalid_value();
        test_parse_root_not_singular();
}

int main() {
        test_parse();
        printf("%d/%d (%3.2f%%) passed\n",\
                        test_pass,\
                        test_count,
                        test_pass * 100.0 / test_count);

        return test_ret;
}
