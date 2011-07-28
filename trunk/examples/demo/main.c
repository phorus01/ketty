/*******************************************************************************
 (c) Copyright 2010, ACTi Corporation, Inc. ALL RIGHTS RESERVED

 All software are Copyright 2010 by ACTi Corporation. ALL RIGHTS RESERVED.
 Redistribution and use in source and binary forms, with or
 without modification, are strictly prohibited.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY EXPRESS
 OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <ketty.h>
#include <ketty/logger.h>


struct _options
{
	//int debug;
	int port;
	const char* root;
};


static int _keepRunning = 0;


static int _readOpts(int argc, char** argv, struct _options* opts);
static void _exitHandler(int sig);

static void _printUsage()
{
	printf("Usage: ketty-web OPTIONS\n");
	printf("\t-p: TCP port to listening on\n");
	printf("\t-r: Root directory of HTTP documents\n");
}

int main(int argc, char** argv)
{
	int rc;
	struct ketty* ketty;
	struct ketty_logger* logger;
	struct _options opts = { .port = 8888 };

	signal(SIGINT, _exitHandler);

	rc = _readOpts(argc, argv, &opts);
	if (rc != 0) {
		_printUsage();
		exit(EXIT_FAILURE);
	}

	logger = ketty_default_logger_new();
	if (logger == NULL) {
		fprintf(stderr, "Failed to create logger\n");
		goto error;
	}
	ketty_set_logger(logger);

	ketty = ketty_new();
	if (ketty == NULL) {
		fprintf(stderr, "Failed to allocate memory for ketty\n");
		goto error;
	}

	ketty_set_document_root(ketty, opts.root);
	if (ketty_start(ketty, opts.port) != 0) goto error;

	_keepRunning = 1;
	while (_keepRunning == 1) sleep(1);

	ketty_stop(ketty);

error:
	if (ketty != NULL) ketty_free(ketty);
	if (logger != NULL) ketty_default_logger_free(logger);

	return 0;
}

static int _readOpts(int argc, char** argv, struct _options* opts)
{
	int c;
	int rc = 0;

	do {
		if ((c = getopt(argc, argv, "p:r:")) == -1) break;

		switch(c) {
		case 'p': // which port to listen
			opts->port = atoi(optarg);
			rc = 0;
			break;
		case 'r': // which port to listen
			opts->root = optarg;
			rc = 0;
			break;
		default:
			rc = 1;
			break;
		}
	} while (rc == 0);

	if (opts->port < 1 || opts->port > 65535) return -1;
	if (opts->root == NULL) return -1;

	return rc;
}

static void _exitHandler(int sig)
{
	struct ketty_logger* logger;

	logger = ketty_get_logger();

	_keepRunning = 0;

	logger->log("SIGINT caught\n");
}
