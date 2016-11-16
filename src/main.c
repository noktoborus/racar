/* vim: ft=c ff=unix fenc=utf-8
 * file: src/main.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "tlog.h"
#include "base.h"

void
begin(TL_V)
{
	struct rcr rcr = {};

	rcr_free_all(TL_A, &rcr);
}

int
main(int argc, char *argv[])
{
    tlog_open();
    begin(TL_A);
    tlog_close();
	return EXIT_SUCCESS;
}

