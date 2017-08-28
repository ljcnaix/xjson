#include <assert.h>     // assert()
#include <errno.h>      // errno, ERANGE
#include <math.h>       // HUGE_VAL
#include <stdlib.h>     // NULL, strtod()
#include <string.h>     // malloc()

#include "xjson.h"

#define EXPECT(c, ch)	do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch)     ((ch) >= '0' && (ch) <= '9')
#define ISDIGITNZ(ch)   ((ch) >= '1' && (ch) <= '9')
#define PUTC(c, ch)     do {\
                *(char *)xjson_context_push(c, sizeof(char)) = (ch);\
        } while(0)

typedef struct {
        const char      *json; 
        char            *stack;
        size_t          size, top;
}xjson_context;

/*---------------------------------------------------------------------------*
        函数名: xjson_context_push
        描述:   向当前json会话的栈中压入size大小的数据，并抬高栈指针

        input:  c,              json会话
                size,           压栈的数据大小

        output: None

        return: success, 当前会话的栈顶指针
                failure, 终止程序
 *---------------------------------------------------------------------------*/
static void *
xjson_context_push(xjson_context *c, size_t size) {
        void *ret;
        assert(size > 0);

        if (c->top + size >= c->size) {
                if (c->size == 0) {
                        c->size = XJSON_PARSE_STACK_INIT_SIZE;
                }

                while (c->top + size >= c->size) {
                        c->size += c->size >> 1;
                }

                c->stack = (char *)realloc(c->stack, c->size);
        }

        ret = c->stack + c->top;
        c->top += size;

        return ret;
}

/*---------------------------------------------------------------------------*
        函数名: xjson_context_pop
        描述:   弹出当前json会话中size大小的数据

        input:  c,              json会话
                size,           出栈的数据大小

        output: None

        return: success, 当前会话的栈顶指针
                failure, 终止程序
 *---------------------------------------------------------------------------*/
static void *
xjson_context_pop(xjson_context *c, size_t size) {
        assert(c->top >= size);

        return c->stack + (c->top -= size);
}

/*---------------------------------------------------------------------------*
        函数名: xjson_context_whitespace
        描述:   读取并丢弃json字符串中的空白字符

        input:  c,              json会话

        output: c->json,        指向json字符串中第一个非空白字符

        return: None
 *---------------------------------------------------------------------------*/
static void
xjson_parse_whitespace(xjson_context *c) {
        const char *p = c->json;

        while (*p == ' ' || *p == '\n' || *p == '\t' || *p == '\r') {
                p++;
        }

        c->json = p;
}

/*---------------------------------------------------------------------------*
        函数名: xjson_parse_literal
        描述:   解析boolean和null类型

        input:  c,              json会话
                v,              json对象，用于存储json解析结果
                literal,        指定解析内容，可以为“true”、“false”或“null”
                type,           指定解析类型，可以为XJSON_{TRUE|FALSE|NULL}

        output: v.type          json解析结果, 可以为XJSON_{TRUE|FALSE|NULL}

        return: success, XJSON_PARSE_OK
                failure, XJSON_PARSE_INVALID_VALUE
 *---------------------------------------------------------------------------*/
static int
xjson_parse_literal(
                xjson_context *c,
                xjson_value *v,
                const char *literal,
                xjson_type type) {

        EXPECT(c, literal[0]);
        
        size_t i;
        for (i = 0; literal[i+1]; i++) {
                if (c->json[i] != literal[i+1]) {
                        return XJSON_PARSE_INVALID_VALUE;
                }
        }

        c->json += i;
        v->type = type;

        return XJSON_PARSE_OK;
}

/*---------------------------------------------------------------------------*
        函数名: xjson_parse_number
        描述:   解析number类型

        input:  c,              json会话
                v,              json对象，用于存储json解析结果

        output: v.type          json解析结果, 应为XJSON_NUMBER
                v.number        json解析结果，存储双精度浮点型数值

        return: success, XJSON_PARSE_OK
                failure, XJSON_PARSE_INVALID_VALUE ||
                         XJSON_PARSE_NUMBER_TOO_BIG
 *---------------------------------------------------------------------------*/
static int
xjson_parse_number(xjson_context *c, xjson_value *v) {
        const char *p = c->json;

/*------------------------------校验number格式-------------------------------*/
        if (*p == '-') p++;
        if (*p == '0') p++;
        else {
                if (!ISDIGITNZ(*p)) return XJSON_PARSE_INVALID_VALUE;
                for (p++; ISDIGIT(*p); p++);
        }

        if (*p == '.') {
                p++;
                if (!ISDIGIT(*p)) return XJSON_PARSE_INVALID_VALUE;
                for (p++; ISDIGIT(*p); p++);
        }

        if (*p == 'e' || *p == 'E') {
                p++;
                if (*p == '+' || *p == '-') p++;
                if (!ISDIGIT(*p)) return XJSON_PARSE_INVALID_VALUE;
                for (p++; ISDIGIT(*p); p++);                                
        }
/*---------------------------------------------------------------------------*/

        errno = 0;

        v->u.number = strtod(c->json, NULL);
        if (errno == ERANGE && (v->u.number == HUGE_VAL || v->u.number == -HUGE_VAL))
                return XJSON_PARSE_NUMBER_TOO_BIG;

        c->json = p;
        v->type = XJSON_NUMBER;

        return XJSON_PARSE_OK;
}

/*---------------------------------------------------------------------------*
        函数名: xjson_parse_string
        描述:   解析string类型

        input:  c,              json会话
                v,              json对象，用于存储json解析结果

        output: v.type          json解析结果, 应为XJSON_STRING
                v.u.s.string    json解析结果，存储字符串
                v.u.s.length    json解析结果，存储字符串长度

        return: success, XJSON_PARSE_OK
                failure, XJSON_PARSE_INVALID_STRING_ESCAPE ||
                         XJSON_PARSE_INVALID_STRING_CHAR ||
                         XJSON_PARSE_MISS_QUOTATION_MARK
 *---------------------------------------------------------------------------*/
static int xjson_parse_string(xjson_context *c, xjson_value *v) {
        EXPECT(c, '\"');

        size_t head = c->top, len;
        const char *p = c->json;
        for (;;) {
                char ch = *p++;
                switch(ch) {
                        case '\"':
                                len = c->top - head;
                                xjson_set_string(v, (const char *)xjson_context_pop(c, len), len);
                                c->json = p;
                                return XJSON_PARSE_OK;
                        case '\\':
                                switch (*p++) {
                                        case '\"': PUTC(c, '\"'); break;
                                        case '\\': PUTC(c, '\\'); break;
                                        case '/':  PUTC(c, '/' ); break;
                                        case 'b':  PUTC(c, '\b'); break;
                                        case 'f':  PUTC(c, '\f'); break;
                                        case 'n':  PUTC(c, '\n'); break;
                                        case 'r':  PUTC(c, '\r'); break;
                                        case 't':  PUTC(c, '\t'); break;
                                        default:
                                                c->top = head;
                                                return XJSON_PARSE_INVALID_STRING_ESCAPE;
                                }
                                break;
                        case '\0':
                                c->top = head;
                                return XJSON_PARSE_MISS_QUOTATION_MARK;
                        default:
                                if ((unsigned char)ch < 0x20) {
                                        c->top = head;
                                        return XJSON_PARSE_INVALID_STRING_CHAR;
                                }
                                PUTC(c, ch);
                }
        }
}

/*---------------------------------------------------------------------------*
        函数名: xjson_parse_value
        描述:   json token解析函数

        input:  c,              json会话
                v,              json对象，用于存储json解析结果

        output: v               json解析结果

        return: success, XJSON_PARSE_OK
                failure, XJSON_PARSE_INVALID_VALUE ||
                         XJSON_PARSE_EXPECT_VALUE ||

                         XJSON_PARSE_NUMBER_TOO_BIG ||

                         XJSON_PARSE_INVALID_STRING_ESCAPE ||
                         XJSON_PARSE_INVALID_STRING_CHAR ||
                         XJSON_PARSE_MISS_QUOTATION_MARK
 *---------------------------------------------------------------------------*/
static int
xjson_parse_value(xjson_context *c, xjson_value *v) {

        switch (*c->json) {
                case 't':       return xjson_parse_literal(c, v, "true", XJSON_TRUE);
                case 'f':       return xjson_parse_literal(c, v, "false", XJSON_FALSE);
                case 'n':       return xjson_parse_literal(c, v, "null", XJSON_NULL);
                case '"':       return xjson_parse_string(c, v);
                default:        return xjson_parse_number(c, v);
                case '\0':      return XJSON_PARSE_EXPECT_VALUE;
        }
}

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
int
xjson_parse(xjson_value *v, const char *json) {
        assert(v != NULL);
        
        xjson_context c;
        c.json = json;
        c.stack = NULL;
        c.size = c.top = 0;

        xjson_init(v);
        xjson_parse_whitespace(&c);
        int ret = xjson_parse_value(&c, v);
        if (ret == XJSON_PARSE_OK) {
                xjson_parse_whitespace(&c);
                if (*c.json != '\0') {
                        v->type = XJSON_NULL;
                        ret = XJSON_PARSE_ROOT_NOT_SINGULAR;
                }
        }

        assert(c.top == 0);
        free(c.stack);

        return ret;
}

/*---------------------------------------------------------------------------*
        函数名: xjson_free
        描述:   释放

        input:  v,              json对象

        output: None

        return: success, None
                failure, 程序终止
 *---------------------------------------------------------------------------*/
void
xjson_free(xjson_value *v) {
        assert(v != NULL);

        if (v->type == XJSON_STRING) {
                free(v->u.s.string);
        }

        v->type = XJSON_NULL;
}

/*---------------------------------------------------------------------------*
        函数名: xjson_get_type
        描述:   获取json对象类型

        input:  v,              json对象

        output: None

        return: success, XJSON_[TYPE]
                failure, 程序终止
 *---------------------------------------------------------------------------*/
xjson_type
xjson_get_type(const xjson_value *v) {
        assert(v != NULL);
        return v->type;
}

/*---------------------------------------------------------------------------*
        函数名: xjson_get_boolean
        描述:   获取json boolean对象的值

        input:  v,              json对象

        output: None

        return: success, true || false
                failure, 程序终止
 *---------------------------------------------------------------------------*/
int
xjson_get_boolean(const xjson_value *v) {
        assert(v != NULL &&
                (v->type == XJSON_TRUE || v->type == XJSON_FALSE));
        return v->type == XJSON_TRUE;
}

/*---------------------------------------------------------------------------*
        函数名: xjson_set_boolean
        描述:   设置json boolean对象的值

        input:  v,              json对象
                boolean,        设置的值            

        output: None

        return: success, None
                failure, 程序终止
 *---------------------------------------------------------------------------*/
void
xjson_set_boolean(xjson_value *v, int boolean) {
        assert(v != NULL);
        xjson_free(v);

        v->type = boolean ? XJSON_TRUE : XJSON_FALSE;
}

/*---------------------------------------------------------------------------*
        函数名: xjson_get_number
        描述:   获取json number对象的值

        input:  v,              json对象

        output: None

        return: success, 双精度浮点型数值
                failure, 程序终止
 *---------------------------------------------------------------------------*/
double
xjson_get_number(const xjson_value *v) {
        assert(v != NULL && v->type == XJSON_NUMBER);
        return v->u.number;
}

/*---------------------------------------------------------------------------*
        函数名: xjson_set_number
        描述:   设置json number对象的值

        input:  v,              json对象

        output: None

        return: success, None
                failure, 程序终止
 *---------------------------------------------------------------------------*/
void
xjson_set_number(xjson_value *v, double number) {
        assert(v != NULL);
        xjson_free(v);

        v->u.number = number;
        v->type = XJSON_NUMBER;
}

/*---------------------------------------------------------------------------*
        函数名: xjson_get_string
        描述:   获取json string对象的值

        input:  v,              json对象

        output: None

        return: success, 字符串
                failure, 程序终止
 *---------------------------------------------------------------------------*/
const char *
xjson_get_string(const xjson_value *v) {
        assert(v != NULL && v->type == XJSON_STRING);
        return v->u.s.string;
}

/*---------------------------------------------------------------------------*
        函数名: xjson_set_string_length
        描述:   设置json string对象的长度

        input:  v,              json对象

        output: None

        return: success, 字符串长度
                failure, 程序终止
 *---------------------------------------------------------------------------*/
size_t xjson_get_string_length(const xjson_value *v) {
        assert(v != NULL && v->type == XJSON_STRING);
        return v->u.s.length;
}

/*---------------------------------------------------------------------------*
        函数名: xjson_set_string
        描述:   设置json string对象的值

        input:  v,              json对象

        output: None

        return: success, None
                failure, 程序终止
 *---------------------------------------------------------------------------*/
void
xjson_set_string(
        xjson_value *v,
        const char *string,
        size_t length) {

        assert(v != NULL && (string != NULL || length == 0));
        xjson_free(v);

        v->u.s.string = (char *)malloc(length + 1);
        memcpy(v->u.s.string, string, length);
        v->u.s.string[length] = '\0';
        v->u.s.length = length;
        v->type = XJSON_STRING;
}
