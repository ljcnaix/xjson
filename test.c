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
        } while(0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual),\
                expect, actual, "%d")

#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual),\
                expect, actual, "%.17g")

#define EXPECT_EQ_STRING(expect, actual, _length)\
                EXPECT_EQ_BASE(\
                        sizeof(expect) - 1 == _length &&\
                        memcmp(expect, actual, _length) == 0,\
                        expect, actual, "%s")

#define EXPECT_TRUE(actual) EXPECT_EQ_BASE((actual) != 0, "true", "false", "%s")
#define EXPECT_FALSE(actual) EXPECT_EQ_BASE((actual) == 0, "false", "true", "%s")

#define EXPECT_EQ_SIZE_T(expect, actual)\
        EXPECT_EQ_BASE((expect) == (actual),\
                        (size_t)expect,\
                        (size_t)actual,\
                        "%zu")

static void test_parse_null() {
        xjson_value v;
        xjson_init(&v);
        xjson_set_boolean(&v, xjson_false);
        EXPECT_EQ_INT(XJSON_PARSE_OK, xjson_parse(&v, "null"));
        EXPECT_EQ_INT(XJSON_NULL, xjson_get_type(&v));
        xjson_free(&v);
}

static void test_parse_true() {
        xjson_value v;
        xjson_init(&v);
        xjson_set_boolean(&v, xjson_false);
        EXPECT_EQ_INT(XJSON_PARSE_OK, xjson_parse(&v, "true"));
        EXPECT_EQ_INT(XJSON_TRUE, xjson_get_type(&v));
        xjson_free(&v);
}

static void test_parse_false() {
        xjson_value v;
        xjson_init(&v);
        xjson_set_boolean(&v, xjson_true);
        EXPECT_EQ_INT(XJSON_PARSE_OK, xjson_parse(&v, "false"));
        EXPECT_EQ_INT(XJSON_FALSE, xjson_get_type(&v));
        xjson_free(&v);
}

#define TEST_NUMBER(expect, json)\
        do {\
                xjson_value v;\
                xjson_init(&v);\
                EXPECT_EQ_INT(XJSON_PARSE_OK, xjson_parse(&v, json));\
                EXPECT_EQ_INT(XJSON_NUMBER, xjson_get_type(&v));\
                EXPECT_EQ_DOUBLE(expect, xjson_get_number(&v));\
                xjson_free(&v);\
        } while(0)

static void test_parse_number() {
        TEST_NUMBER(0.0, "0");
        TEST_NUMBER(0.0, "-0");
        TEST_NUMBER(0.0, "0.0");
        TEST_NUMBER(0.0, "-0.0");

        TEST_NUMBER(1.0, "1");
        TEST_NUMBER(1.0, "1.0");
        TEST_NUMBER(-1.0, "-1");
        TEST_NUMBER(-1.0, "-1.0");

        TEST_NUMBER(1.5, "1.5");
        TEST_NUMBER(-1.5, "-1.5");

        TEST_NUMBER(3.1415926, "3.1415926");

        TEST_NUMBER(1E10, "1E10");
        TEST_NUMBER(1e10, "1e10");
        TEST_NUMBER(1E+10, "1E+10");
        TEST_NUMBER(1E-10, "1E-10");
        TEST_NUMBER(-1E+10, "-1E+10");
        TEST_NUMBER(-1E-10, "-1E-10");

        TEST_NUMBER(3.1415926E+10, "3.1415926E+10");
        TEST_NUMBER(3.1415926E-10, "3.1415926E-10");

        TEST_NUMBER(0.0, "1e-10000");

        /* the smallest number > 1 */
        TEST_NUMBER(1.0000000000000002, "1.0000000000000002");

        /* minimum denormal */
        TEST_NUMBER( 4.9406564584124654e-324, "4.9406564584124654e-324");
        TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");

        /* Max subnormal double */
        TEST_NUMBER( 2.2250738585072009e-308, "2.2250738585072009e-308");
        TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");

        /* Min normal positive double */
        TEST_NUMBER( 2.2250738585072014e-308, "2.2250738585072014e-308");
        TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");

        /* Max double */
        TEST_NUMBER( 1.7976931348623157e+308, "1.7976931348623157e+308");
        TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

#define TEST_STRING(expect, json)\
        do {\
                xjson_value v;\
                xjson_init(&v);\
                EXPECT_EQ_INT(XJSON_PARSE_OK, xjson_parse(&v, json));\
                EXPECT_EQ_INT(XJSON_STRING, xjson_get_type(&v));\
                EXPECT_EQ_STRING(expect, xjson_get_string(&v), xjson_get_string_length(&v));\
                xjson_free(&v);\
        } while(0)

static void test_parse_string() {
        TEST_STRING("", "\"\"");
        TEST_STRING("Hello", "\"Hello\"");
        TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
        TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
}

static void test_parse_array() {
        xjson_value v;
        xjson_init(&v);

        EXPECT_EQ_INT(XJSON_PARSE_OK, xjson_parse(&v, "[ ]"));
        EXPECT_EQ_INT(XJSON_ARRAY, xjson_get_type(&v));
        EXPECT_EQ_SIZE_T(0, xjson_get_array_size(&v));

        xjson_free(&v);
}

#define TEST_ERROR(error, json)\
        do {\
                xjson_value v;\
                xjson_init(&v);\
                v.type = XJSON_FALSE;\
                EXPECT_EQ_INT(error, xjson_parse(&v, json));\
                EXPECT_EQ_INT(XJSON_NULL, xjson_get_type(&v));\
                xjson_free(&v);\
        } while(0)

static void test_parse_expect_value() {
        TEST_ERROR(XJSON_PARSE_EXPECT_VALUE, ""); 
        TEST_ERROR(XJSON_PARSE_EXPECT_VALUE, " "); 
} 

static void test_parse_invalid_value() {
        TEST_ERROR(XJSON_PARSE_INVALID_VALUE, "tru"); 
        TEST_ERROR(XJSON_PARSE_INVALID_VALUE, "fal"); 
        TEST_ERROR(XJSON_PARSE_INVALID_VALUE, "nul"); 
        TEST_ERROR(XJSON_PARSE_INVALID_VALUE, "?"); 

        TEST_ERROR(XJSON_PARSE_INVALID_VALUE, "+0"); 
        TEST_ERROR(XJSON_PARSE_INVALID_VALUE, "+1"); 
        TEST_ERROR(XJSON_PARSE_INVALID_VALUE, ".123"); 
        TEST_ERROR(XJSON_PARSE_INVALID_VALUE, "1."); 
        TEST_ERROR(XJSON_PARSE_INVALID_VALUE, "INF"); 
        TEST_ERROR(XJSON_PARSE_INVALID_VALUE, "inf"); 
        TEST_ERROR(XJSON_PARSE_INVALID_VALUE, "NAN"); 
        TEST_ERROR(XJSON_PARSE_INVALID_VALUE, "nan"); 

        /* invalid array value */
        TEST_ERROR(XJSON_PARSE_INVALID_VALUE, "[1,]");
        TEST_ERROR(XJSON_PARSE_INVALID_VALUE, "[\"a\", nul]");
}

static void test_parse_root_not_singular() {
        TEST_ERROR(XJSON_PARSE_ROOT_NOT_SINGULAR, "null x"); 

        /* invalid number */
        TEST_ERROR(XJSON_PARSE_ROOT_NOT_SINGULAR, "0123"); 
        TEST_ERROR(XJSON_PARSE_ROOT_NOT_SINGULAR, "0x0"); 
        TEST_ERROR(XJSON_PARSE_ROOT_NOT_SINGULAR, "0x123"); 
}

static void test_parse_number_too_big() {
        TEST_ERROR(XJSON_PARSE_NUMBER_TOO_BIG, "1e309");
        TEST_ERROR(XJSON_PARSE_NUMBER_TOO_BIG, "-1e309");
}

static void test_parse_missing_quotation_mark() {
        TEST_ERROR(XJSON_PARSE_MISS_QUOTATION_MARK, "\"");
        TEST_ERROR(XJSON_PARSE_MISS_QUOTATION_MARK, "\"abc");
}

static void test_parse_invalid_string_escape() {
        TEST_ERROR(XJSON_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
        TEST_ERROR(XJSON_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
        TEST_ERROR(XJSON_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
        TEST_ERROR(XJSON_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}

static void test_parse_invalid_string_char() {
        TEST_ERROR(XJSON_PARSE_INVALID_STRING_CHAR, "\"\x01\"");
        TEST_ERROR(XJSON_PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}

static void test_parse_miss_comma_or_square_bracket() {
        TEST_ERROR(XJSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1");
        TEST_ERROR(XJSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1}");
        TEST_ERROR(XJSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1 2");
        TEST_ERROR(XJSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[[]");
}

static void test_parse() {
        test_parse_null();
        test_parse_true();
        test_parse_false();
        test_parse_number();
        test_parse_string();
        test_parse_array();
        
        test_parse_expect_value();
        test_parse_invalid_value();
        test_parse_root_not_singular();
        test_parse_number_too_big();
        test_parse_missing_quotation_mark();
        test_parse_invalid_string_escape();
        test_parse_invalid_string_char();
        test_parse_miss_comma_or_square_bracket();
}

static void test_access_null() {
        xjson_value v;
        xjson_init(&v);
        xjson_set_string(&v, "a", 1);
        xjson_set_null(&v);
        EXPECT_EQ_INT(XJSON_NULL, xjson_get_type(&v));
        xjson_free(&v);
}

static void test_access_boolean() {
        xjson_value v;
        xjson_init(&v);
        xjson_set_boolean(&v, xjson_true);
        EXPECT_TRUE(xjson_get_boolean(&v));
        xjson_set_boolean(&v, xjson_false);
        EXPECT_FALSE(xjson_get_boolean(&v));
        xjson_free(&v);
}

static void test_access_number() {
        xjson_value v;
        xjson_init(&v);
        xjson_set_number(&v, 3.1415926);
        EXPECT_EQ_DOUBLE(3.1415926, xjson_get_number(&v));
        xjson_free(&v);
}

static void test_access_string() {
        xjson_value v;
        xjson_init(&v);
        xjson_set_string(&v, "", 0);
        EXPECT_EQ_STRING("", xjson_get_string(&v), xjson_get_string_length(&v));
        xjson_set_string(&v, "Hello", 5);
        EXPECT_EQ_STRING("Hello", xjson_get_string(&v), xjson_get_string_length(&v));
        xjson_free(&v);
}

static void test_access() {
        test_access_null();
        test_access_boolean();
        test_access_number();
        test_access_string();
}

int main() {
        test_parse();
        test_access();

        printf("%d/%d (%3.2f%%) passed\n",\
                        test_pass,\
                        test_count,
                        test_pass * 100.0 / test_count);

        return test_ret;
}
