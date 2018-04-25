#define _GNU_SOURCE
#include <sys/mman.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define INPUT_SIZE 20000000UL

typedef unsigned long int idx_t;
#define PRINT_FMT "lu"

static idx_t pi[5 << 20];
static const char *restrict pattern;
static const char *restrict text;

static char *restrict
find(idx_t m, idx_t n, register char *restrict w)
{
	register idx_t state = 0, i, h = 0;
	const char *restrict pp = &pattern[1];

	i = 3, pi[1] = 0;
	if (2 < m)
		pi[2] = state = pp[1] == pp[0];
	for (; i < m; i++) {
		while (pattern[i] != pp[state]) {
			if (state)
				state = pi[state];
			else
				goto next;
		}
		state += 1;
	next:
		pi[i] = state;
	}  

	i = 1, m--, state = *pp == text[1];
	goto search;
	for (; i < n; i++) {
		while (text[i] != pp[state]) {
			if (state)
				state = pi[state];
			else
				goto search;
		}
		state += 1;
	search:
		if (state == m)
			w += sprintf(w, "%"PRINT_FMT, i - m), *w++ = ' ';
	}

	*w++ = '\n';
	return w;
}

int
main(void)
{
	size_t ptr = 0, size;
	ssize_t r;
	char *restrict input = mmap(NULL, INPUT_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	register const char *restrict p, *restrict q;
	char *restrict output, *restrict w;

	register idx_t pn, tn;

	while ((r = syscall(SYS_read, 0, &input[ptr], INPUT_SIZE)))
                ptr += (size_t)r;
        input[ptr] = '\0';
	w = output = &input[ptr + 1];
	for (p = input; *p;) {
		for (pattern = q = p; *q++ != '\n';);
		pn = (idx_t)(q - p);
		for (text = p = q; *p++ != '\n';);
		tn = (idx_t)(p - q);
		pattern--;
		text--;
		w = find(pn, tn, w);
	}

	size = (size_t)(w - output);
	for (ptr = 0; ptr < size; ptr += (size_t)r)
		r = syscall(SYS_write, 1, &output[ptr], size - ptr);

	return 0;
}

