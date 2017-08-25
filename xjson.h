#ifndef XJSON_H_
#define XJSON_H_

#include <stddef.h>                     // size_t

#define xjson_true                      1
#define xjson_false                     0

#ifndef XJSON_PARSE_STACK_INIT_SIZE
#define XJSON_PARSE_STACK_INIT_SIZE     256
#endif

typedef enum {
	XJSON_NULL,
	XJSON_FALSE,
	XJSON_TRUE,
	XJSON_NUMBER,
	XJSON_STRING,
	XJSON_ARRAY,
	XJSON_OBJECT
} xjson_type;

typedef struct {
	xjson_type      type;

        union {
                double number;          // number
                struct {
                        char *string;   // null-terminated string
                        size_t length;  // string length
                }s;
        }u;

} xjson_value;

enum {
	XJSON_PARSE_OK = 0,

	XJSON_PARSE_EXPECT_VALUE,               // lack of token
	XJSON_PARSE_INVALID_VALUE,              // invalid token
	XJSON_PARSE_ROOT_NOT_SINGULAR,

        XJSON_PARSE_NUMBER_TOO_BIG,             // number overflow

        XJSON_PARSE_MISS_QUOTATION_MARK,
        XJSON_PARSE_INVALID_STRING_ESCAPE,
        XJSON_PARSE_INVALID_STRING_CHAR
};

#define xjson_init(v) do { (v)->type = XJSON_NULL; } while(0)

int xjson_parse(xjson_value *v, const char *json);
void xjson_free(xjson_value *v);

#define xjson_set_null(v)       xjson_free(v)

xjson_type xjson_get_type(const xjson_value *v);

int xjson_get_boolean(const xjson_value *v);
void xjson_set_boolean(xjson_value *v, int boolean);

double xjson_get_number(const xjson_value *v);
void xjson_set_number(xjson_value *v, double number);

const char* xjson_get_string(const xjson_value *v);
size_t xjson_get_string_length(const xjson_value *v);
void xjson_set_string(xjson_value *v, const char *string, size_t length);

#endif
