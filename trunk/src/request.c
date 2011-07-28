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

#include "../simclist-1.5/simclist.h"

#include <ketty/request.h>
#include "request-private.h"


struct ketty_http_req
{
	struct ketty_http_connection* connection;

	const char* url;
	const char* method;
	const char* version;

	void* properties;
};


struct property
{
	const char* key;
	const char* val;
};


struct ketty_http_req* ketty_http_req_new(struct ketty_http_connection* c,
										  const char* url,
										  const char* method,
										  const char* version)
{
	struct ketty_http_req* req = NULL;
	list_t* list = NULL;

	req = malloc(sizeof(*req) + sizeof(*list));
	if (req == NULL) return NULL;

	req->connection = c;
	req->url = url;
	req->method = method;
	req->version = version;

	req->properties = list = (void*)(req + 1);
	if (list_init(list) != 0) goto error;

	return req;

error:
	if (req != NULL) free(req);

	return NULL;
}

void ketty_http_req_free(struct ketty_http_req* req)
{
	struct property* p = NULL;

	assert(req != NULL);
	if (req == NULL) return;

	while (!list_empty(req->properties)) {
		p = list_fetch(req->properties);

		assert(p != NULL);
		if (p == NULL) continue;

		if (p->key != NULL) free((void*)p->key);
		if (p->val != NULL) free((void*)p->val);
		free(p);
	}

	list_destroy(req->properties);

	free(req);
}

struct ketty_http_connection* ketty_http_req_get_connection(struct ketty_http_req* req)
{
	return req->connection;
}

const char* ketty_http_req_get_url(struct ketty_http_req* req)
{
	return req->url;
}

const char* ketty_http_req_get_method(struct ketty_http_req* req)
{
	return req->method;
}

/*
void* ketty_http_req_get_connection(struct ketty_http_req* req)
{
	assert(req != NULL);
	if (req == NULL) return NULL;

	return req->connection;
}
*/

int ketty_http_req_add_property(struct ketty_http_req* req,
								const char* key, const char* val)
{
	int rc = 1;
	struct property* p;

	assert(key != NULL);
	assert(val != NULL);

	do {
		if (key == NULL || val == NULL) break;

		p = malloc(sizeof(*p));
		if (p == NULL) break;

		p->key = strdup(key);
		p->val = strdup(val);

		//list_insert_end(req->props, p);
		if (list_append(req->properties, p) < 0) {
			free((void*)p->key);
			free((void*)p->val);
			free(p);
			break;
		}

		rc = 0;
	} while (0);

	return rc;
}

const char* ketty_http_req_get_property(struct ketty_http_req* req,
										const char* key)
{
	struct property* p;
	const char* val = NULL;

	// TODO: To use seeker of the list
	list_iterator_start(req->properties);
	while (list_iterator_hasnext(req->properties)) {
		p = list_iterator_next(req->properties);

		assert(p != NULL);
		if (p == NULL) continue;

		if (strcmp(p->key, key) == 0) {
			val = p->val;
			break;
		}
	}
	list_iterator_stop(req->properties);

	return val;
}
