#include <assert.h>
#include <stdlib.h>

#include "xjson.h"

#define EXPECT(c, ch)	do { assert(*c->json == (ch)); c->json++; } while(0)

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

static int xjson_parse_true(xjson_context *c, xjson_value *v) {
        EXPECT(c, 't');

        if (c->json[0] != 'u' || c->json[1] != 'r' || c->json[2] != 'e') {
                return XJSON_PARSE_INVALID_VALUE;
        }

        c->json += 3;
        v->type = XJSON_TRUE;

        return XJSON_PARSE_OK;
}


static int xjson_parse_false(xjson_context *c, xjson_value *v) {
        EXPECT(c, 'f');

        if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e') {
                return XJSON_PARSE_INVALID_VALUE;
        }

        c->json += 4;
        v->type = XJSON_FALSE;

        return XJSON_PARSE_OK;
}

static int xjson_parse_null(xjson_context *c, xjson_value *v) {
        EXPECT(c, 'n');

        if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l') {
                return XJSON_PARSE_INVALID_VALUE;
        }

        c->json += 3;
        v->type = XJSON_NULL;

        return XJSON_PARSE_OK;
}

static int xjson_parse_value(xjson_context *c, xjson_value *v) {

        switch (*c->json) {
                case 't':       return xjson_parse_true(c, v);
                case 'f':       return xjson_parse_false(c, v);
                case 'n':       return xjson_parse_null(c, v);
                case '\0':      return XJSON_PARSE_EXPECT_VALUE;
                default:        return XJSON_PARSE_INVALID_VALUE;
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
                        ret = XJSON_PARSE_ROOT_NOT_SINGULAR;
                }
        }

        return ret;
}

xjson_type xjson_get_type(const xjson_value *v) {
        assert(v != NULL);

        return v->type;
}
