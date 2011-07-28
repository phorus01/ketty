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

#ifndef _KETTY_H_INCLUDED_
#define _KETTY_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


struct ketty;
struct ketty_logger;
struct ketty_servlet;
struct ketty_http_req;
struct ketty_http_resp;
struct ketty_http_raelm;


struct ketty_logger* ketty_null_logger_new();
void ketty_null_logger_free(struct ketty_logger* logger);
struct ketty_logger* ketty_default_logger_new();
void ketty_default_logger_free(struct ketty_logger* logger);

struct ketty_logger* ketty_get_logger();
struct ketty_logger* ketty_set_logger(struct ketty_logger* logger);

struct ketty* ketty_new();
int    ketty_start(struct ketty* ketty, int port);
int    ketty_stop(struct ketty* ketty);
void   ketty_free(struct ketty* ketty);

const char* ketty_get_document_root(struct ketty* ketty);
int ketty_set_document_root(struct ketty* ketty, const char* path);

typedef struct ketty_servlet* (*ketty_servlet_factory)(struct ketty* ketty,
													   struct ketty_http_req* req,
													   struct ketty_http_resp* resp);
int ketty_register_servlet(struct ketty* ketty,
						   const char* name, const char* raelm,
						   const char* pattern,
						   ketty_servlet_factory factory);

int ketty_add_raelm(struct ketty* ketty,
					const char* name, const char* pattern,
					struct ketty_http_raelm* raelm);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _KETTY_H_INCLUDED_ */
