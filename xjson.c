#include <assert.h>     // assert()
#include <errno.h>      // errno, ERANGE
#include <math.h>       // HUGE_VAL
#include <stdlib.h>     // NULL, strtod()

#include "xjson.h"

#define EXPECT(c, ch)	do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch)     ((ch) >= '0' && (ch) <= '9')
#define ISDIGITNZ(ch)    ((ch) >= '1' && (ch) <= '9')

typedef struct {
        const char *json; 
}xjson_context;

static void xjson_parse_whitespace(xjson_context *c) {
        const char *p = c->json;

        while (*p == ' ' || *p == '\n' || *p == '\t' || *p == '\r') {
                p++;
        }

        c->json = p;
}

static int xjson_parse_literal(xjson_context *c, xjson_value *v, const char *literal, xjson_type type) {
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

static int xjson_parse_number(xjson_context *c, xjson_value *v) {
        const char *p = c->json;
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

        errno = 0;

        v->number = strtod(c->json, NULL);
        if (errno == ERANGE && (v->number == HUGE_VAL || v->number == -HUGE_VAL))
                return XJSON_PARSE_NUMBER_TOO_BIG;

        c->json = p;
        v->type = XJSON_NUMBER;

        return XJSON_PARSE_OK;
}

static int xjson_parse_value(xjson_context *c, xjson_value *v) {

        switch (*c->json) {
                case 't':       return xjson_parse_literal(c, v, "true", XJSON_TRUE);
                case 'f':       return xjson_parse_literal(c, v, "false", XJSON_FALSE);
                case 'n':       return xjson_parse_literal(c, v, "null", XJSON_NULL);
                default:        return xjson_parse_number(c, v);
                case '\0':      return XJSON_PARSE_EXPECT_VALUE;
        }
}

int xjson_parse(xjson_value *v, const char *json) {
        assert(v != NULL);
        
        xjson_context c;
        c.json = json;
        v->type = XJSON_NULL;

        xjson_parse_whitespace(&c);
        int ret = xjson_parse_value(&c, v);
        if (ret == XJSON_PARSE_OK) {
                xjson_parse_whitespace(&c);
                if (*c.json != '\0') {
                        v->type = XJSON_NULL;
                        ret = XJSON_PARSE_ROOT_NOT_SINGULAR;
                }
        }

        return ret;
}

xjson_type xjson_get_type(const xjson_value *v) {
        assert(v != NULL);

        return v->type;
}

double xjson_get_number(const xjson_value *v) {
        assert(v != NULL);

        return v->number;
}
