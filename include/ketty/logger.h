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

#ifndef _KETTY_LOGGER_H_INCLUDED_
#define _KETTY_LOGGER_H_INCLUDED_


#include <stdarg.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


struct ketty_logger
{
	void* priv;

	void  (*log)(const char *format, ...);
	void (*vlog)(const char *format, va_list ap);
};


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _KETTY_LOGGER_H_INCLUDED_ */
