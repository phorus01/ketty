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

#include <stdio.h>
#include <stdlib.h>

#include <ketty/logger.h>


struct ketty_logger* _logger = NULL;


struct ketty_logger* ketty_get_logger()
{
	return _logger;
}

struct ketty_logger* ketty_set_logger(struct ketty_logger* logger)
{
	struct ketty_logger* old = _logger;

	_logger = logger;

	return old;
}

void _null_log(const char *format, ...)
{
}

void _null_vlog(const char *format, va_list ap)
{
}

struct ketty_logger* ketty_null_logger_new()
{
	struct ketty_logger* logger;

	logger = calloc(1, sizeof(*logger));
	if (logger == NULL) return NULL;

	logger->log  = _null_log;
	logger->vlog = _null_vlog;

	return logger;
}

void ketty_null_logger_free(struct ketty_logger* logger)
{
	free(logger);
}

void _default_log(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
}

void _default_vlog(const char *format, va_list ap)
{
	vfprintf(stderr, format, ap);
}

struct ketty_logger* ketty_default_logger_new()
{
	struct ketty_logger* logger;

	logger = calloc(1, sizeof(*logger));
	if (logger == NULL) return NULL;

	logger->log  = _default_log;
	logger->vlog = _default_vlog;

	return logger;
}

void ketty_default_logger_free(struct ketty_logger* logger)
{
	free(logger);
}
