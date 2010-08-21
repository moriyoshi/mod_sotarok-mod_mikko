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

module AP_MODULE_DECLARE_DATA sotarok_module;

static const char *data[] = {
    "VAAAAAgAAAABAAEAAAADAAEAAQAAAAIAAQADAAAAAgABAAQAAAACAAEABAAAAAIAAQABAAAAAwABAAEAAAAH",
    "AAEAAQAAAAMAAQABAAAAAQABAAUAAAABAAEABAAAAAIAAQAEAAAAAwABAAMAAAACAAEAAQAAAAMAAQABAAAA",
    "AgABAAQAAAADAAEAAQAAAAMAAQABAAAAAwABAAEAAAABAAEAAQAAAAMAAQABAAAAAQABAAEAAAADAAEAAQAA",
    "AAEAAQABAAAAAwABAAEAAAABAAEAAQAAAAMAAQABAAAABwABAAEAAAADAAEAAQAAAAEAAQABAAAABQABAAEA",
    "AAADAAEAAQAAAAEAAQABAAAAAwABAAEAAAADAAEAAQAAAAMAAQABAAAAAwABAAEAAAABAAEAAQAAAAcAAQAB",
    "AAAAAwABAAEAAAADAAEAAQAAAAEAAQABAAAAAwABAAEAAAABAAEAAQAAAAMAAQABAAAAAQABAAEAAAADAAEA",
    "AQAAAAIAAQABAAAAAQABAAEAAAAIAAEAAQAAAAEAAQABAAAAAQABAAEAAAABAAEAAQAAAAUAAQABAAAAAwAB",
    "AAEAAAABAAEAAQAAAAMAAQABAAAAAwABAAEAAAADAAEAAgAAAAIAAQABAAAAAQABAAEAAAAHAAEAAQAAAAMA",
    "AQAFAAAAAQABAAUAAAABAAEABAAAAAIAAQAEAAAAAwABAAEAAAABAAEAAQAAAAgAAQABAAAAAQABAAEAAAAB",
    "AAEAAQAAAAEAAQAEAAAAAgABAAEAAAADAAEAAQAAAAEAAQABAAAAAwABAAEAAAADAAEAAQAAAAMAAQABAAAA",
    "AQABAAEAAAABAAEAAQAAAAEAAQABAAAAAQABAAMAAAADAAEAAQAAAAMAAQABAAAAAwABAAEAAAABAAEAAQAA",
    "AAMAAQABAAAAAQABAAEAAAAFAAEAAQAAAAcAAQABAAAACQABAAEAAAABAAEAAQAAAAEAAQABAAAAAQABAAEA",
    "AAAFAAEAAQAAAAMAAQABAAAAAQABAAEAAAADAAEAAQAAAAMAAQABAAAAAwABAAEAAAACAAEAAgAAAAEAAQAB",
    "AAAAAwABAAEAAAADAAEAAQAAAAMAAQABAAAAAwABAAEAAAABAAEAAQAAAAMAAQABAAAAAQABAAEAAAAFAAEA",
    "AQAAAAcAAQABAAAACQABAAEAAAABAAEAAQAAAAEAAQABAAAAAQABAAEAAAAFAAEAAQAAAAMAAQABAAAAAQAB",
    "AAEAAAADAAEAAQAAAAMAAQABAAAAAwABAAEAAAADAAEAAQAAAAEAAQABAAAAAwABAAEAAAAHAAEAAQAAAAMA",
    "AQABAAAAAQABAAEAAAADAAEAAQAAAAEAAQABAAAABQABAAEAAAAHAAEAAQAAAAoAAQABAAAAAQABAAEAAAAC",
    "AAEABQAAAAEAAQAEAAAAAgABAAQAAAADAAEAAwAAAAIAAQABAAAAAwABAAEAAAACAAEABAAAAAMAAQABAAAA",
    "AwAAAFQAAAAAAA=="
};

static int sotarok_handler(request_rec *r)
{
    int i;
    if (strcmp(r->handler, "sotarok-handler")) {
        return DECLINED;
    }

    ap_set_content_type(r, "text/plain; charset=US-ASCII");
    if (r->header_only) return OK;
    for (i = 0; i < sizeof(data) / sizeof(data[0]); i++) {
        ap_rputs(data[i], r);
        ap_rputs("\n", r);
    }
    return OK;
}

static void sotarok_register_hooks(apr_pool_t *p)
{
    ap_hook_handler(sotarok_handler, NULL, NULL, APR_HOOK_MIDDLE);
}

module AP_MODULE_DECLARE_DATA sotarok_module =
{
    STANDARD20_MODULE_STUFF,
    NULL,                   /* per-directory config creator */
    NULL,                   /* dir config merger */
    NULL,                   /* server config creator */
    NULL,                   /* server config merger */
    NULL,                   /* command table */
    sotarok_register_hooks    /* set up other request processing hooks */
};
