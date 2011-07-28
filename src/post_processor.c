/*
    Ketty
    ----------------------------------------------------------------------
    Copyright (C) 2011, W.L. Chuang <ponponli2000@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <assert.h>

#include <stdlib.h>
#include <string.h>

// Include microhttpd related header files
#include <stdarg.h>
#include <stdint.h>
#include <sys/socket.h>
#include <microhttpd.h>

#include <ketty.h>
#include <ketty/code.h>
#include <ketty/filter.h>
#include <ketty/request.h>
#include <ketty/logger.h>

#include "connection.h"
#include "post_processor.h"


struct context {
	void* data; // struct MHD_PostProcessor* at this moment
	struct ketty_http_req* req;
};


static int  _handle(void* self,
					struct ketty_http_req* req,
					struct ketty_http_resp* resp,
					const char* upload_data, size_t *upload_data_size);
static void _release(void* self);
static int _post_iterator(void *cls,
						  enum MHD_ValueKind kind,
						  const char *key,
						  const char *filename,
						  const char *content_type,
						  const char *transfer_encoding,
						  const char *value, uint64_t off, size_t size);


struct ketty_req_filter* ketty_post_processor_new(struct ketty_http_req* req)
{
	struct ketty_req_filter* filter;
	struct context* ctx;

	filter = malloc(sizeof(*filter) + sizeof(*ctx));
	if (filter == NULL) return NULL;
	memset(filter, 0, sizeof(*filter));

	filter->handle = _handle;
	filter->release = _release;

	//ctx = (struct context*)(((char*)filter) + sizeof(*filter));
	ctx = (struct context*)(filter + 1);

	filter->context = ctx;
	ctx->req = req;

	ctx->data = MHD_create_post_processor(ketty_http_req_get_connection(req)->priv,
										  1024, &_post_iterator, filter);
	if (ctx->data == NULL) {
		free(filter);
		filter = NULL;
	}

	return filter;
}

static int  _handle(void* self,
					struct ketty_http_req* req,
					struct ketty_http_resp* resp,
					const char* upload_data, size_t* upload_data_size)
{
	int rc = MHD_NO;
	struct context* ctx;
	//struct ketty_logger* logger;

	struct ketty_req_filter* filter = self;

	//logger = ketty_get_logger();

	do {
		if (strcmp(ketty_http_req_get_method(req),
				   MHD_HTTP_METHOD_POST) != 0)
		{
			//rc = KETTY_FILTER_BLOCK;
			rc = KETTY_FILTER_PASSED;
			break;
		}

		if (*upload_data_size == 0) {
			rc = KETTY_FILTER_PASSED;
			break;
		}

		ctx = filter->context;
		assert(ctx != NULL);

		//logger->log("Upload data>\n%s\n", upload_data);

		MHD_post_process(ctx->data, upload_data, *upload_data_size);
		*upload_data_size = 0;

		rc = KETTY_FILTER_EXECUTING;
	} while(0);

	return rc;
}

static void _release(void* self)
{
	struct ketty_req_filter* filter = self;
	struct context* ctx;

	ctx = filter->context;
	assert(ctx != NULL);

	MHD_destroy_post_processor(ctx->data);

	free(filter);
}

static int _post_iterator(void *cls,
						  enum MHD_ValueKind kind,
						  const char *key,
						  const char *filename,
						  const char *content_type,
						  const char *transfer_encoding,
						  const char *value, uint64_t off, size_t size)
{
	struct ketty_req_filter* filter = cls;
	struct context* ctx;
	struct ketty_logger* logger;

	logger = ketty_get_logger();

	ctx = filter->context;

	logger->log("HTTP POST field:\nkey = %s\nvalue =\n%s\n", key, value);
	//log4c_category_log(log, LOG4C_PRIORITY_TRACE, "POST type/encoding: %s, %s",
	//				   content_type, transfer_encoding);
	ketty_http_req_add_property(ctx->req, key, value);

	return MHD_YES;
}
