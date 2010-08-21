/* 
 * Copyright (c) 2010 Moriyoshi Koizumi
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <assert.h>
#include "httpd.h"
#include "http_config.h"
#include "http_core.h"
#include "http_log.h"
#include "http_main.h"
#include "http_protocol.h"
#include "http_request.h"
#include "util_script.h"
#include "http_connection.h"

#include "apr_strings.h"

#include <stdio.h>

module AP_MODULE_DECLARE_DATA mikko_module;

static unsigned int read_u32le(const char *p)
{
    return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}

static unsigned int read_u16le(const char *p)
{
    return p[0] | (p[1] << 8);
}

static char *base64_decode_all(apr_pool_t *pool, const char *buf, apr_size_t buf_len, apr_size_t *result_len)
{
    const char *p, *e = buf + buf_len;
    char *result, *q;
    q = result = apr_palloc(pool, (buf_len / 3) * 4);
    p = buf;
    while (p < e) {
        int l = apr_base64_decode(q, p);
        if (l == 0)
            p++;
        else {
            assert(l > 0);
            q += l;
            p += ((l + 2) / 3) * 4;
        }
    }

    *result_len = q - result;
    return result;
}

static apr_status_t mikko_do_out_filter(ap_filter_t *f, apr_bucket_brigade *bb)
{
    static const char prologue[] = "<html>\n\
<head>\n\
  <title>Happy Wedding!</title>\n\
  <style type=\"text/css\">\n\
.colored { color: red; }\n\
.uncolored { color: lightgray; }\n\
</style>\n\
</head>\n\
<body>\n\
<pre>";
    static const char epilogue[] = "</pre>\n\
</body>\n\
</html>\n";
    static const char span_colored_open[] = "<span class=\"colored\">";
    static const char span_uncolored_open[] = "<span class=\"uncolored\">";
    static const char span_close[] = "</span>";
    apr_status_t err = APR_SUCCESS;
    char *buf = 0;
    apr_size_t buf_len = 0;
    char *decoded_data = 0;
    unsigned int width, height;
    const char *p, *pe;
    const char *q;

    err = apr_brigade_pflatten(bb, &buf, &buf_len, f->r->pool);
    if (err)
        return err;
    apr_brigade_cleanup(bb);

    APR_BRIGADE_INSERT_TAIL(bb, apr_bucket_immortal_create(
        prologue, sizeof(prologue) - 1, f->c->bucket_alloc));

    {
        apr_size_t decoded_len;
        p = decoded_data = base64_decode_all(f->r->pool, buf, buf_len, &decoded_len);
        pe = decoded_data + decoded_len;
    }

    if (p >= pe)
        goto error;
    width = read_u32le(p), p += 4;
    if (p >= pe)
        goto error;
    height = read_u32le(p), p += 4;
   
    q = buf; 
    for (;;) { 
        int v, c;
        if (p >= pe)
            goto error;
        v = read_u16le(p), p += 2;
        if (p >= pe)
            goto error;
        c = read_u16le(p), p += 2;
        if (!c)
            break;
        if (v) {
            APR_BRIGADE_INSERT_TAIL(bb, apr_bucket_immortal_create(
                span_colored_open, sizeof(span_colored_open) - 1, f->c->bucket_alloc));
            APR_BRIGADE_INSERT_TAIL(bb, apr_bucket_transient_create(q, c,
                f->c->bucket_alloc));
            APR_BRIGADE_INSERT_TAIL(bb, apr_bucket_immortal_create(
                span_close, sizeof(span_close) - 1, f->c->bucket_alloc));
        } else {
            APR_BRIGADE_INSERT_TAIL(bb, apr_bucket_immortal_create(
                span_uncolored_open, sizeof(span_uncolored_open) - 1, f->c->bucket_alloc));
            APR_BRIGADE_INSERT_TAIL(bb, apr_bucket_transient_create(q, c,
                f->c->bucket_alloc));
            APR_BRIGADE_INSERT_TAIL(bb, apr_bucket_immortal_create(
                span_close, sizeof(span_close) - 1, f->c->bucket_alloc));
        }
        q += c;
        while (*q == '\n') {
            APR_BRIGADE_INSERT_TAIL(bb, apr_bucket_transient_create(q, 1,
                f->c->bucket_alloc));
            q++;
        }
    }

    APR_BRIGADE_INSERT_TAIL(bb, apr_bucket_immortal_create(
        span_uncolored_open, sizeof(span_uncolored_open) - 1, f->c->bucket_alloc));
    APR_BRIGADE_INSERT_TAIL(bb, apr_bucket_transient_create(
        q, buf_len - (q - buf), f->c->bucket_alloc));
    APR_BRIGADE_INSERT_TAIL(bb, apr_bucket_immortal_create(
        span_close, sizeof(span_close) - 1, f->c->bucket_alloc));

    APR_BRIGADE_INSERT_TAIL(bb, apr_bucket_immortal_create(
        epilogue, sizeof(epilogue) - 1, f->c->bucket_alloc));

    APR_BRIGADE_INSERT_TAIL(bb, apr_bucket_eos_create(f->c->bucket_alloc));

    f->r->content_type = "text/html; charset=US-ASCII";

    return ap_pass_brigade(f->next, bb);
error:
    APR_BRIGADE_INSERT_TAIL(bb, apr_bucket_immortal_create(
        "error", sizeof("error") - 1, f->c->bucket_alloc));
    APR_BRIGADE_INSERT_TAIL(bb, apr_bucket_eos_create(f->c->bucket_alloc));
    return ap_pass_brigade(f->next, bb);
}

static void mikko_register_hooks(apr_pool_t *p)
{
    ap_register_output_filter("MIKKO_OUT", mikko_do_out_filter, NULL, AP_FTYPE_RESOURCE);
}

module AP_MODULE_DECLARE_DATA mikko_module =
{
    STANDARD20_MODULE_STUFF,
    NULL,                   /* per-directory config creator */
    NULL,                   /* dir config merger */
    NULL,                   /* server config creator */
    NULL,                   /* server config merger */
    NULL,                   /* command table */
    mikko_register_hooks    /* set up other request processing hooks */
};
