#ifndef XJSON_H_
#define XJSON_H_

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
	xjson_type type;
} xjson_value;

enum {
	XJSON_PARSE_OK = 0,
	XJSON_PARSE_EXPECT_VALUE,
	XJSON_PARSE_INVALID_VALUE,
	XJSON_PARSE_ROOT_NOT_SINGULAR
};

/*
* Parse json string.
*/
int xjson_parse(xjson_value *v, const char *json);
xjson_type xjson_get_type(const xjson_value *v);

#endif
