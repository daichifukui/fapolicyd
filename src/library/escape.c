/*
 * escape.c - Source file for escaping capability
 * Copyright (c) 2021 Red Hat Inc.
 * All Rights Reserved.
 *
 * This software may be freely redistributed and/or modified under the
 * terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor
 * Boston, MA 02110-1335, USA.
 *
 * Authors:
 *   Radovan Sroka <rsroka@redhat.com>
 */

#include "escape.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "message.h"


#define IS_HEX(X) (isxdigit(X) > 0 && !(islower(X) > 0))

static char asciiHex2Bits(char X)
{
	char base = 0;
	if (X >= '0' && X <= '9') {
		base = '0';
	} else if (X >= 'A' && X <= 'F') {
		base = 'A' - 10;
  }
	return (X - base) & 0X00FF;
}


/*
 * the function returns escaped copy of the input string
 * returned string should be freed by caller
 * it returns NULL in case of error
 *
*/
char *escape(const char *input, int mode)
{
	char buffer[4096 + 1] = {0};
	size_t input_len = strlen(input);
	size_t pos = 0;

	for (size_t i = 0 ; i < input_len; i++) {
		int should_escape = 0;

		switch(mode) {

		case EVERYTHING:
			should_escape = 1;
			break;

		case WHITESPACES:
			if (isblank(input[i]))
				should_escape = 1;
			break;

		default:
			return NULL;
			break;

		}

		// always ascape % sign !!!
		if (input[i] == '%')
			should_escape = 1;

		if (should_escape) {
			char buff[4] = {0}; // "%A5\0"
			int res = snprintf(buff, 4, "%c%02X", '%', input[i]);

			if (res == -1)
				return NULL;

			/*
			buffer[pos++] = buff[0]; // '%'
			buffer[pos++] = buff[1]; // 'A'
			buffer[pos++] = buff[2]; // '5'
			*/

			for (size_t ii = 0; ii < 3; ii++) {

				if (pos >=(sizeof(buffer) - 1))
					return NULL;

				buffer[pos++] = buff[ii];
			}

		} else {

			if (pos >=(sizeof(buffer) - 1))
				return NULL;

			buffer[pos++] = input[i];
		}
	}

	return strdup(buffer);
}

char *unescape(const char *input)
{
	char buffer[4096 + 1] = {0};
	size_t input_len = strlen(input);
	size_t pos = 0;

	for (size_t i = 0; i < input_len; i++ ) {
		if (input[i] == '%') {

			if (i+2 < input_len && (IS_HEX(input[i+1]) && IS_HEX(input[i+2])) ) {
				char c = asciiHex2Bits(input[i+1]);
				char d = asciiHex2Bits(input[i+2]);

				if (pos >=(sizeof(buffer) - 1))
					return NULL;

				buffer[pos++] = (c << 4) + d;
				i += 2;

			} else {

				msg(LOG_WARNING, "Input %s does not have a valid escape sequence, "
					"unable to unescape, copying char by char", input);

				// if not vaid sequence, copy char by char
				if (pos >=(sizeof(buffer) - 1))
					return NULL;

				buffer[pos++] = input[i];

			}

		} else {

			if (pos >=(sizeof(buffer) - 1))
				return NULL;

			buffer[pos++] = input[i];
		}
	}

	return strdup(buffer);
}
