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

#ifndef _KETTY_HTTP_REQUEST_PRIVATE_H_INCLUDED_
#define _KETTY_HTTP_REQUEST_PRIVATE_H_INCLUDED_


struct ketty_http_connection;


struct ketty_http_req* ketty_http_req_new(struct ketty_http_connection* c,
										  const char* url,
										  const char* method,
										  const char* version);
void ketty_http_req_free(struct ketty_http_req* req);


#endif /* _KETTY_HTTP_REQUEST_PRIVATE_H_INCLUDED_ */
