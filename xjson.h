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

typedef struct _xjson_value xjson_value;
struct _xjson_value {
	xjson_type      type;

        union {
                double number;          // number
                struct {
                        xjson_value *e; // array elements
                        size_t size;    // array count
                }a;
                struct {
                        char *string;   // null-terminated string
                        size_t length;  // string length
                }s;
        }u;

};

enum {
	XJSON_PARSE_OK = 0,

	XJSON_PARSE_EXPECT_VALUE,               // lack of token
	XJSON_PARSE_INVALID_VALUE,              // invalid token
	XJSON_PARSE_ROOT_NOT_SINGULAR,

        XJSON_PARSE_NUMBER_TOO_BIG,             // number overflow

        XJSON_PARSE_MISS_QUOTATION_MARK,
        XJSON_PARSE_INVALID_STRING_ESCAPE,
        XJSON_PARSE_INVALID_STRING_CHAR,
        XJSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET
};

#define xjson_init(v) do { (v)->type = XJSON_NULL; } while(0)

/*---------------------------------------------------------------------------*
        函数名: xjson_free
        描述:   释放(xjson_value *)(v)->u.s.string

        input:  v,              json对象

        output: None

        return: success, None
                failure, 程序终止
 *---------------------------------------------------------------------------*/
void xjson_free(xjson_value *v);

#define xjson_set_null(v)       xjson_free(v)

/*---------------------------------------------------------------------------*
        函数名: xjson_parse
        描述:   json字符串解析函数

        input:  v,              json对象，用于存储json解析结果
                json,           json字符串

        output: v               json解析结果

        return: success, XJSON_PARSE_OK
                failure, XJSON_PARSE_INVALID_VALUE ||
                         XJSON_PARSE_EXPECT_VALUE ||
                         XJSON_PARSE_ROOT_NOT_SINGULAR || 

                         XJSON_PARSE_NUMBER_TOO_BIG ||

                         XJSON_PARSE_INVALID_STRING_ESCAPE ||
                         XJSON_PARSE_INVALID_STRING_CHAR ||
                         XJSON_PARSE_MISS_QUOTATION_MARK
 *---------------------------------------------------------------------------*/
int xjson_parse(xjson_value *v, const char *json);

/*---------------------------------------------------------------------------*
        函数名: xjson_get_type
        描述:   获取json对象类型

        input:  v,              json对象

        output: None

        return: success, XJSON_[TYPE]
                failure, 程序终止
 *---------------------------------------------------------------------------*/
xjson_type xjson_get_type(const xjson_value *v);

/*---------------------------------------------------------------------------*
        函数名: xjson_get_boolean
        描述:   获取json boolean对象的值

        input:  v,              json对象

        output: None

        return: success, true || false
                failure, 程序终止
 *---------------------------------------------------------------------------*/
int xjson_get_boolean(const xjson_value *v);

/*---------------------------------------------------------------------------*
        函数名: xjson_set_boolean
        描述:   设置json boolean对象的值

        input:  v,              json对象
                boolean,        设置的值            

        output: None

        return: success, None
                failure, 程序终止
 *---------------------------------------------------------------------------*/
void xjson_set_boolean(xjson_value *v, int boolean);

/*---------------------------------------------------------------------------*
        函数名: xjson_get_number
        描述:   获取json number对象的值

        input:  v,              json对象

        output: None

        return: success, 双精度浮点型数值
                failure, 程序终止
 *---------------------------------------------------------------------------*/
double xjson_get_number(const xjson_value *v);

/*---------------------------------------------------------------------------*
        函数名: xjson_set_number
        描述:   设置json number对象的值

        input:  v,              json对象

        output: None

        return: success, None
                failure, 程序终止
 *---------------------------------------------------------------------------*/
void xjson_set_number(xjson_value *v, double number);

/*---------------------------------------------------------------------------*
        函数名: xjson_get_string
        描述:   获取json string对象的值

        input:  v,              json对象

        output: None

        return: success, 字符串
                failure, 程序终止
 *---------------------------------------------------------------------------*/
const char* xjson_get_string(const xjson_value *v);

/*---------------------------------------------------------------------------*
        函数名: xjson_set_string_length
        描述:   设置json string对象的长度

        input:  v,              json对象

        output: None

        return: success, 字符串长度
                failure, 程序终止
 *---------------------------------------------------------------------------*/
size_t xjson_get_string_length(const xjson_value *v);

/*---------------------------------------------------------------------------*
        函数名: xjson_set_string
        描述:   设置json string对象的值

        input:  v,              json对象

        output: None

        return: success, None
                failure, 程序终止
 *---------------------------------------------------------------------------*/
void xjson_set_string(xjson_value *v, const char *string, size_t length);

/*---------------------------------------------------------------------------*
        函数名: xjson_get_array_size
        描述:   获取json array对象的长度

        input:  v,              json对象

        output: None

        return: success, 返回json array对象的长度
                failure, 程序终止
 *---------------------------------------------------------------------------*/
size_t xjson_get_array_size(const xjson_value *v);

/*---------------------------------------------------------------------------*
        函数名: xjson_get_array_element
        描述:   根据index，获取json array对象的成员

        input:  v,              json对象
                index,          索引

        output: None

        return: success, 返回json array对象的成员
                failure, 程序终止
 *---------------------------------------------------------------------------*/
xjson_value *xjson_get_array_element(const xjson_value *v, size_t index);

#endif
