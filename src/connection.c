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

#include <ketty/servlet.h>

#include "connection.h"
#include "request-private.h"
#include "response-private.h"


struct ketty_http_connection* ketty_http_connection_new(struct ketty* ketty,
														const char* url,
														const char* method,
														const char* version,
														void* priv)
{
	struct ketty_http_connection* connection;

	connection = calloc(1, sizeof(*connection));
	if (connection == NULL) return NULL;

	connection->ketty = ketty;
	connection->request = NULL;
	connection->response = NULL;
	connection->servlet = NULL;
	connection->priv = priv;

	return connection;
}

void ketty_http_connection_free(struct ketty_http_connection* connection)
{
	assert(connection != NULL);
	if (connection == NULL) return;

	if (connection->request != NULL) {
		ketty_http_req_free(connection->request);
	}

	if (connection->response != NULL) {
		ketty_http_resp_free(connection->response);
	}

	if (connection->servlet != NULL) {
		connection->servlet->release(connection->servlet);
	}
}
