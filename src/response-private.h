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

#ifndef _KETTY_HTTP_RESPONSE_PRIVATE_H_INCLUDED_
#define _KETTY_HTTP_RESPONSE_PRIVATE_H_INCLUDED_


struct ketty_http_connection;


struct ketty_http_resp* ketty_http_resp_new(struct ketty_http_connection* c);
void ketty_http_resp_free(struct ketty_http_resp* resp);

//void* ketty_http_resp_get_connection(struct ketty_http_resp* resp);


#endif /* _KETTY_HTTP_RESPONSE_PRIVATE_H_INCLUDED_ */
