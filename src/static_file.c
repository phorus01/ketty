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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>

#include <microhttpd.h>

#include <ketty.h>
#include <ketty/code.h>
#include <ketty/filter.h>
#include <ketty/servlet.h>
#include <ketty/request.h>
#include <ketty/response.h>
#include <ketty/logger.h>

#include "static_file.h"


struct _context
{
	char* root;
	FILE* input;
};


static int  _open(void* servlet,
				  struct ketty_http_req* req, struct ketty_http_resp* resp);
static int  _read(void* servlet, uint64_t pos, char* buf, size_t max);
static void _close(void* servlet);


struct ketty_servlet* ketty_file_reader_new(struct ketty* ketty,
											struct ketty_http_req* req,
											struct ketty_http_resp* resp)
{
	struct ketty_servlet* servlet;

	assert(ketty != NULL);
	if (ketty == NULL) return NULL;

	servlet = calloc(1, sizeof(*servlet) + sizeof(struct _context));
	if (servlet == NULL) return NULL;

	//servlet->ketty = ketty;
	//servlet->request = req;
	//servlet->response = resp;

	servlet->start   = _open;
	servlet->write   = _read;
	servlet->release = _close;

	servlet->context = (struct _context*)(servlet + 1);
	((struct _context*)(servlet->context))->root =
								strdup(ketty_get_document_root(ketty));

	return servlet;
}

static int  _open(void* servlet,
				  struct ketty_http_req* req, struct ketty_http_resp* resp)
{
	char buffer[256];
	struct stat file;
	struct ketty_logger* logger;
	struct ketty_servlet* self = servlet;
	const char* url = ketty_http_req_get_url(req);

	logger = ketty_get_logger();

	memset(buffer, 0, 256);
	snprintf(buffer, 256, "%s%s",
			 ((struct _context*)(self->context))->root, url);
	stat(buffer, &file);

	if (S_ISDIR(file.st_mode)) {
		logger->log("File not exist: %s\n", buffer);
		return KETTY_SERVLET_ERROR;
	}
	((struct _context*)(self->context))->input = fopen(buffer, "rb");
	if (((struct _context*)(self->context))->input == NULL) {
		logger->log("File not exist: %s\n", buffer);
		return KETTY_SERVLET_ERROR;
	}
	logger->log("Opening file: %s(%d)\n", buffer, file.st_size);

	return file.st_size;
}

static int _read(void* servlet, uint64_t pos, char* buf, size_t max)
{
	int read;
	FILE* input;
	struct ketty_logger* logger;
	struct ketty_servlet* self = servlet;

	logger = ketty_get_logger();

	input = ((struct _context*)(self->context))->input;

	if (input == NULL) {
		logger->log("File handler is NULL\n");
		return MHD_CONTENT_READER_END_WITH_ERROR;
	}

	if (feof(input)) {
		logger->log("EoF is encountered\n");
		return MHD_CONTENT_READER_END_OF_STREAM;
	}

	fseek(input, pos, SEEK_SET);

	read = fread(buf, 1, max, input);
	logger->log("Reading file at %d, max = %d, actual = %d\n",
				(int)pos, max, read);

	return read;
}

static void _close(void* servlet)
{
	struct ketty_logger* logger;

	struct ketty_servlet* self = servlet;
	struct _context* context = self->context;

	logger = ketty_get_logger();
	logger->log("Destroy servlet: static_file\n");


	if (self->filter != NULL) {
		assert(self->filter->release != NULL);

		self->filter->release(self->filter);
	}

	if (context != NULL) {
		if (context->root != NULL) free(context->root);
		if (context->input != NULL) fclose(context->input);
	}

	free(self);
}
