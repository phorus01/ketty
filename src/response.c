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

// Include microhttpd related header files
#include <stdarg.h>
#include <stdint.h>
#include <sys/socket.h>
#include <microhttpd.h>

#include <ketty/response.h>
#include "response-private.h"


struct ketty_http_resp
{
	struct ketty_http_connection* connection; /* = struct MHD_Connection* at this moment */
	void* data;       /* = struct MHD_Response* at this moment, could be NULL */
};


struct ketty_http_resp* ketty_http_resp_new(struct ketty_http_connection* c)
{
	struct ketty_http_resp* resp;

	resp = malloc(sizeof(*resp));
	if (resp == NULL) return NULL;

	resp->connection = c;
	resp->data = NULL;

	return resp;
}

void ketty_http_resp_free(struct ketty_http_resp* resp)
{
	assert(resp != NULL);
	if (resp == NULL) return;

	if (resp->data != NULL) {
		MHD_destroy_response(resp->data);
		resp->data = NULL;
	}

	free(resp);
}

//void* ketty_http_resp_get_connection(struct ketty_http_resp* resp)
//{
//	assert(resp != NULL);
//	if (resp == NULL) return NULL;
//
//	return resp->connection;
//}
