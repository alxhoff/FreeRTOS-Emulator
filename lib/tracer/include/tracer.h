#ifndef __TRACER_H__
#define __TRACER_H__
#include <time.h>
#include <stdio.h>

static FILE *fp_trace = NULL;

void __attribute__((constructor)) trace_begin(void)
{
    fp_trace = fopen("trace.out", "w");
}

void __attribute__((destructor)) trace_end(void)
{
    if (fp_trace) {
        fclose(fp_trace);
    }
}

void __attribute((no_instrument_function))
__cyg_profile_func_enter(void *func, void *caller)
{
    if (fp_trace) {
        fprintf(fp_trace, "e %p %p %lu\n", func, caller, time(NULL));
    }
}

void __attribute((no_instrument_function))
__cyg_profile_func_exit(void *func, void *caller)
{
    if (fp_trace) {
        fprintf(fp_trace, "x %p %p %lu\n", func, caller, time(NULL));
    }
}
#endif
