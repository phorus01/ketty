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

#ifndef _KETTY_SERVLET_H_INCLUDED_
#define _KETTY_SERVLET_H_INCLUDED_


#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


struct ketty;
struct ketty_http_req;
struct ketty_http_resp;
struct ketty_req_filter;


struct ketty_servlet
{
	//struct ketty*            ketty;
	//struct ketty_http_req*   request;
	//struct ketty_http_resp*  response;

	struct ketty_req_filter* filter;

	int  (*start)(void* self,
				  struct ketty_http_req* req, struct ketty_http_resp* resp);
	int  (*write)(void* self, uint64_t pos, char* buf, size_t max);
	void (*release)(void* self);

	void* context;
};


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _KETTY_SERVLET_H_INCLUDED_ */
