/*
 * libwebsockets - small server side websockets and web server implementation
 *
 * Copyright (C) 2010 - 2019 Andy Green <andy@warmcat.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "private-lib-core.h"

#ifdef LWS_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if defined(LWS_PLAT_OPTEE)
void lwsl_emit_optee(int level, const char *line);
#endif

int log_level = LLL_ERR | LLL_WARN | LLL_NOTICE;
static void (*lwsl_emit)(int level, const char *line);

#ifndef LWS_PLAT_OPTEE
static const char * log_level_names ="EWNIDPHXCLUT??";
#endif

#if defined(LWS_LOGS_TIMESTAMP)
int
lwsl_timestamp(int level, char *p, int len)
{
#ifndef LWS_PLAT_OPTEE
	time_t o_now;
	unsigned long long now;
	struct timeval tv;
	struct tm *ptm = NULL;
#ifndef WIN32
	struct tm tm;
#endif
	int n;

	gettimeofday(&tv, NULL);
	o_now = tv.tv_sec;
	now = ((unsigned long long)tv.tv_sec * 10000) + (tv.tv_usec / 100);

#ifndef _WIN32_WCE
#ifdef WIN32
	ptm = localtime(&o_now);
#else
	if (localtime_r(&o_now, &tm))
		ptm = &tm;
#endif
#endif
	p[0] = '\0';
	for (n = 0; n < LLL_COUNT; n++) {
		if (level != (1 << n))
			continue;

		if (ptm)
			n = lws_snprintf(p, len,
				"[%04d/%02d/%02d %02d:%02d:%02d:%04d] %c: ",
				ptm->tm_year + 1900,
				ptm->tm_mon + 1,
				ptm->tm_mday,
				ptm->tm_hour,
				ptm->tm_min,
				ptm->tm_sec,
				(int)(now % 10000), log_level_names[n]);
		else
			n = lws_snprintf(p, len, "[%llu:%04d] %c: ",
					(unsigned long long) now / 10000,
					(int)(now % 10000), log_level_names[n]);
		return n;
	}
#else
	p[0] = '\0';
#endif

	return 0;
}
#endif

#ifndef LWS_PLAT_OPTEE

#endif

#if !(defined(LWS_PLAT_OPTEE) && !defined(LWS_WITH_NETWORK))
void _lws_logv(int filter, const char *format, va_list vl)
{
}

void _lws_log(int filter, const char *format, ...)
{
}
#endif
void lws_set_log_level(int level, void (*func)(int level, const char *line))
{
	log_level = level;
	if (func)
		lwsl_emit = func;
}

int lwsl_visible(int level)
{
	return log_level & level;
}

void
lwsl_hexdump_level(int hexdump_level, const void *vbuf, size_t len)
{
	unsigned char *buf = (unsigned char *)vbuf;
	unsigned int n;

	if (!lwsl_visible(hexdump_level))
		return;

	if (!len) {
		_lws_log(hexdump_level, "(hexdump: zero length)\n");
		return;
	}

	if (!vbuf) {
		_lws_log(hexdump_level, "(hexdump: NULL ptr)\n");
		return;
	}

	_lws_log(hexdump_level, "\n");

	for (n = 0; n < len;) {
		unsigned int start = n, m;
		char line[80], *p = line;

		p += lws_snprintf(p, 10, "%04X: ", start);

		for (m = 0; m < 16 && n < len; m++)
			p += lws_snprintf(p, 5, "%02X ", buf[n++]);
		while (m++ < 16)
			p += lws_snprintf(p, 5, "   ");

		p += lws_snprintf(p, 6, "   ");

		for (m = 0; m < 16 && (start + m) < len; m++) {
			if (buf[start + m] >= ' ' && buf[start + m] < 127)
				*p++ = buf[start + m];
			else
				*p++ = '.';
		}
		while (m++ < 16)
			*p++ = ' ';

		*p++ = '\n';
		*p = '\0';
		_lws_log(hexdump_level, "%s", line);
		(void)line;
	}

	_lws_log(hexdump_level, "\n");
}

void
lwsl_hexdump(const void *vbuf, size_t len)
{
#if defined(_DEBUG)
	lwsl_hexdump_level(LLL_DEBUG, vbuf, len);
#endif
}
