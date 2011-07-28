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

//#include <pthread.h>

// Include microhttpd related header files
#include <stdarg.h>
#include <stdint.h>
#include <sys/socket.h>
#include <microhttpd.h>

#include "../simclist-1.5/simclist.h"

#include <ketty.h>
#include <ketty/code.h>
#include <ketty/request.h>
#include <ketty/response.h>
#include <ketty/filter.h>
#include <ketty/servlet.h>
#include <ketty/logger.h>

#include "connection.h"
#include "request-private.h"
#include "response-private.h"
#include "post_processor.h"
#include "static_file.h"


struct ketty
{
	struct MHD_Daemon* daemon;

	list_t*            servletRegistry;
	const char*        docRoot;

	MHD_AccessHandlerCallback    onAccess;
	MHD_RequestCompletedCallback onReqComplete;
};

struct servlet_registry_entry
{
	const char* name;
	const char* raelm;
	const char* pattern;
	ketty_servlet_factory factory;

	void (*free)(struct servlet_registry_entry* self);
};
static struct servlet_registry_entry* _servletRegistryEntryNew(
							const char* name,
							const char* raelm, const char* pattern,
							ketty_servlet_factory factory);
static void _servletRegistryEntryFree(struct servlet_registry_entry* self);

static int _onAccess(void* cls,
					 struct MHD_Connection* connection,
					 const char* url,
					 const char* method,
					 const char* version,
					 const char* upload_data, size_t *upload_data_size,
					 void** ptr);
static void _onReqComplete(void *cls,
						   struct MHD_Connection * connection,
						   void **con_cls,
						   enum MHD_RequestTerminationCode toe);
static struct ketty_servlet* _createServlet(struct ketty* ketty,
											struct ketty_http_req* req,
											struct ketty_http_resp* resp);
static int  _executeServlet(struct ketty* ketty,
							struct ketty_http_connection* connection,
							const char* upload_data, size_t *upload_data_size);

struct ketty* ketty_new()
{
	struct ketty* ketty;

	ketty = malloc(sizeof(*ketty) + sizeof(list_t));
	if (ketty == NULL) return NULL;
	memset(ketty, 0, sizeof(*ketty) + sizeof(list_t));

	ketty->servletRegistry = (list_t*)(ketty + 1);
	if (list_init(ketty->servletRegistry) != 0) goto error;

	ketty->docRoot = strdup("");
	if (ketty->docRoot == NULL) goto error;

	ketty->onAccess = _onAccess;
	ketty->onReqComplete = _onReqComplete;

	return ketty;

error:
	if (ketty->docRoot != NULL) free((void*)ketty->docRoot);
	free(ketty);

	return NULL;
}

void ketty_free(struct ketty* ketty)
{
	assert(ketty != NULL);

	if (ketty == NULL) return;

	if (ketty->daemon != NULL) {
		MHD_stop_daemon(ketty->daemon);
		ketty->daemon = NULL;
	}

	if (ketty->servletRegistry != NULL) {
		struct servlet_registry_entry* entry;

		while (!list_empty(ketty->servletRegistry)) {
			entry = list_fetch(ketty->servletRegistry);

			assert(entry != NULL);
			if (entry == NULL) continue;

			entry->free(entry);
		}

		list_destroy(ketty->servletRegistry);
	}

	if (ketty->docRoot != NULL) free((void*)ketty->docRoot);

	free(ketty);
}

static void _mhdLogger(void* arg, const char* fmt, va_list ap);

int ketty_start(struct ketty* ketty, int port)
{
	struct ketty_logger* logger;

	assert(ketty != NULL);
	if (ketty == NULL) return -1;

	logger = ketty_get_logger();
	logger->log("Ketty options:\n");
	logger->log("    port: %d\n", port);

	logger->log("Starting ketty...\n");
	ketty->daemon =
		MHD_start_daemon(MHD_USE_DEBUG | MHD_USE_THREAD_PER_CONNECTION,
						 port,
						 NULL, NULL,     // MHD_AcceptPolicyCallback
						 ketty->onAccess, ketty, // MHD_AccessHandlerCallback
						 MHD_OPTION_NOTIFY_COMPLETED, ketty->onReqComplete, ketty,
						 MHD_OPTION_EXTERNAL_LOGGER, _mhdLogger, ketty,
						 MHD_OPTION_END);
	if (ketty->daemon == NULL) {
		logger->log("Failed to start server\n");
		return -1;
	}

	return 0;
}

int ketty_stop(struct ketty* ketty)
{
	struct ketty_logger* logger;

	assert(ketty != NULL);
	if (ketty == NULL) return -1;

	logger = ketty_get_logger();

	MHD_stop_daemon(ketty->daemon);
	logger->log("Server stopped...\n");

	ketty->daemon = NULL;

	return 0;
}

int ketty_register_servlet(struct ketty* ketty,
						   const char* name,
						   const char* raelm, const char* pattern,
						   ketty_servlet_factory factory)
{
	struct ketty_logger* logger;
	struct servlet_registry_entry* entry;

	entry = _servletRegistryEntryNew(name, raelm, pattern, factory);
	if (entry == NULL) return -1;

	logger = ketty_get_logger();
	logger->log("servlet registry entry(%s) created\n", name);

	if (list_append(ketty->servletRegistry, entry) < 0) {
		entry->free(entry);
		return -1;
	}

	return 0;
}

static int _onAccess(void* cls,
					 struct MHD_Connection* conn,
					 const char* url,
					 const char* method,
					 const char* version,
					 const char* upload_data, size_t *upload_data_size,
					 void** ptr)
{
	int rc = MHD_YES;
	struct ketty* ketty = cls;
	struct ketty_logger* logger;

	logger = ketty_get_logger();

	if (*ptr == NULL) {
		struct ketty_http_connection* connection;

		logger->log("\nCreating connection for:\n\turl: %s\n\tmethod: %s\n\t"
					"version: %s\n", url, method, version);

		// Create connection on first call
		connection = ketty_http_connection_new(ketty,
											   url, method, version, conn);
		if (connection == NULL) {
			logger->log("Failed to create connection\n");
			return MHD_NO;
		}

		connection->request = ketty_http_req_new(connection,
												 url, method, version);
		if (connection->request == NULL) goto error;

		connection->response = ketty_http_resp_new(connection);
		if (connection->response == NULL) goto error;

		connection->servlet = _createServlet(ketty,
											 connection->request,
											 connection->response);
		if (connection->servlet == NULL) goto error;

		*ptr = connection;
		logger->log("Connection established\n");

		return MHD_YES;

error:
		if (connection != NULL) ketty_http_connection_free(connection);

		return MHD_NO;
	}

	rc = _executeServlet(ketty, *ptr, upload_data, upload_data_size);

	return rc;
}

static void _onReqComplete(void *cls,
						   struct MHD_Connection * conn,
						   void **con_cls,
						   enum MHD_RequestTerminationCode toe)
{
	//struct ketty* ketty = cls;
	struct ketty_logger* logger;
	struct ketty_http_connection* connection = NULL;

	logger = ketty_get_logger();
	logger->log("Request Completed event received\n");

	connection = *con_cls;
	assert(connection);
	if (connection == NULL) return;

	ketty_http_connection_free(connection);
	*con_cls = NULL;
}

static struct ketty_servlet* _createServlet(struct ketty* ketty,
											struct ketty_http_req* req,
											struct ketty_http_resp* resp)
{
	struct ketty_logger* logger;
	struct ketty_servlet* servlet = NULL;
	struct servlet_registry_entry* entry = NULL;

	logger = ketty_get_logger();

	list_iterator_start(ketty->servletRegistry);
	while (list_iterator_hasnext(ketty->servletRegistry)) {
		entry = list_iterator_next(ketty->servletRegistry);

		assert(entry != NULL);
		if (entry == NULL) continue;

		if (strcmp(ketty_http_req_get_url(req), entry->pattern) == 0) {
			servlet = entry->factory(ketty, req, resp);
			break;
		}
	}
	list_iterator_stop(ketty->servletRegistry);

	if (servlet == NULL) {
		servlet = ketty_file_reader_new(ketty, req, resp);
	}

	assert(servlet != NULL);
	if (servlet == NULL) {
		logger->log("Failed to create servlet\n");
		return NULL;
	}

	if (strcmp(ketty_http_req_get_method(req), MHD_HTTP_METHOD_POST) == 0) {
		servlet->filter = ketty_post_processor_new(req);
		if (servlet->filter == NULL) {
			logger->log("Failed to create servlet HTTP POST filter\n");
			goto error;
		}
		logger->log("HTTP POST processor filter is created\n");
	} else if (strcmp(ketty_http_req_get_method(req), MHD_HTTP_METHOD_GET) == 0) {
		// TODO: Adds HTTP GET value parser
	}

	return servlet;

error:
	if (servlet != NULL) servlet->release(servlet);

	return NULL;
}

static int  _executeServlet(struct ketty* ketty,
							struct ketty_http_connection* connection,
							const char* upload_data, size_t *upload_data_size)
{
	int size;
	struct ketty_logger* logger;
	int rc = KETTY_FILTER_PASSED;
	struct ketty_servlet* servlet = connection->servlet;

	struct MHD_Response* resp;

	logger = ketty_get_logger();

	if (servlet->filter != NULL) {
		logger->log("Invoking request filter...\n");
		logger->log("upload data size: %d\n", *upload_data_size);
		rc = servlet->filter->handle(servlet->filter,
									 connection->request, connection->response,
									 upload_data, upload_data_size);
	}

	switch (rc) {
	case KETTY_FILTER_PASSED: // The request is bypassed by the filter
		logger->log("Processing servlet...\n");

		size = servlet->start(servlet,
							  connection->request, connection->response);
		if (size == KETTY_SERVLET_ERROR) {
			logger->log("Failed to execute servlet. rc: %d\n", size);
			rc = MHD_NO;
			break;
		}
		logger->log("Estimated output size of servlet is: %d\n", size);

		resp = MHD_create_response_from_callback(size,
												 1024 * 16, /* 16k size */
												 servlet->write, servlet,
												 NULL);
		if (resp == NULL) {
			logger->log("Failed to make response to this request\n");
			rc = MHD_NO;
			break;
		}

		assert(connection->priv != NULL);
		rc = MHD_queue_response(connection->priv, MHD_HTTP_OK, resp);
		logger->log("HTTP response code: %d\n", rc);
		MHD_destroy_response(resp);
		break;
	case KETTY_FILTER_BLOCK:     // The request is blocked by the filter
		logger->log("HTTP request is blocked by filter\n");
		rc = MHD_NO;
		break;
	case KETTY_FILTER_EXECUTING: // The request is handled by the filter
		logger->log("HTTP request is handled by filter\n");
		rc = MHD_YES;
		break;
	default:
		logger->log("Unknown status code: %d\n", rc);
		rc = MHD_NO;
		break;
	}

	return rc;
}

static struct servlet_registry_entry* _servletRegistryEntryNew(
							const char* name,
							const char* raelm, const char* pattern,
							ketty_servlet_factory factory)
{
	struct servlet_registry_entry* entry;

	if (factory == NULL) return NULL;

	entry = calloc(1, sizeof(*entry));
	if (entry == NULL) return NULL;

	entry->free = _servletRegistryEntryFree;

	entry->name = strdup(name);
	if (entry->name == NULL) goto error;

	entry->raelm = strdup(raelm);
	if (entry->raelm == NULL) goto error;

	entry->pattern = strdup(pattern);
	if (entry->pattern == NULL) goto error;

	entry->factory = factory;

	return entry;

error:
	_servletRegistryEntryFree(entry);

	return NULL;
}

static void _servletRegistryEntryFree(struct servlet_registry_entry* self)
{
	if (self->name != NULL) free((void*)self->name);

	if (self->raelm == NULL) free((void*)self->raelm);

	if (self->pattern == NULL) free((void*)self->pattern);

	free(self);
}

const char* ketty_get_document_root(struct ketty* ketty)
{
	assert(ketty != NULL);
	if (ketty == NULL) return NULL;

	return ketty->docRoot;
}

int ketty_set_document_root(struct ketty* ketty, const char* path)
{
	const char* root;

	assert(ketty != NULL);
	assert(path != NULL);
	if (ketty == NULL) return -1;
	if (path == NULL) return -1;

	root = strdup(path);
	if (root == NULL) return -1;

	free((void*)ketty->docRoot);

	ketty->docRoot = root;

	return 0;
}

static void _mhdLogger(void* arg, const char* fmt, va_list ap)
{
	struct ketty_logger* logger;

	logger = ketty_get_logger();

	logger->vlog(fmt, ap);
}

int ketty_add_raelm(struct ketty* ketty,
					const char* name, const char* pattern,
					struct ketty_http_raelm* raelm)
{
	return -1;
}
