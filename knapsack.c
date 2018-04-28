#include <sys/mman.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef signed int int_t;
typedef unsigned int uint_t;
#define PRINT_FMT "i"

static int_t V[10001][2001];
static int_t values[2001];
static int_t weights[2001];

static inline const char *restrict
parseheadpair(register const char *restrict s, int_t *restrict ap, int_t *restrict bp)
{
	register uint_t r;

	r = *s & 15;
	while (*++s != ' ' && *s != '.')
		r = (r * 10) + (*s & 15);
	*ap = (int_t)r;
	while (*s != ' ') s++;

	r = *s & 15;
	while (*++s != '\n')
		r = (r * 10) + (*s & 15);
	*bp = (int_t)r;

	return s;
}

static inline const char *restrict
parseintpair(register const char *restrict s, int_t *restrict ap, int_t *restrict bp)
{
	register uint_t r;

	r = *s & 15;
	while (*++s != ' ')
		r = (r * 10) + (*s & 15);
	*ap = (int_t)r;

	r = *s & 15;
	while (*++s != '\n')
		r = (r * 10) + (*s & 15);
	*bp = (int_t)r;

	return s;
}

static char *restrict
solve(int_t capacity, int_t n, register char *restrict wr)
{
	register int_t i, j, jj, choosen = 0, take, w, v;
	int_t *restrict Vi = V[0], *restrict Vii;
	char *restrict p = &wr[5];

	__builtin_memset(Vi, 0, capacity * sizeof(*Vi));

	for (i = 1; i <= n; i++) {
		Vii = Vi, Vi = V[i];
		v = values[i], w = weights[i];
		__builtin_memcpy(Vi, Vii, w * sizeof(*Vi));
		for (jj = 0, j = w; j <= capacity; jj++, j++) {
			take = Vii[jj] + v;
			Vi[j] = take > Vii[j] ? take : Vii[j];
		}
	}

	for (; n; n--) {
		if (V[n][capacity] == V[n - 1][capacity - weights[n]] + values[n]) {
			p += sprintf(p, "%"PRINT_FMT, n - 1), *p++ = ' ';
			capacity -= weights[n];
			choosen++;
		}
	}

	wr[0] = wr[1] = wr[2] = wr[3] = wr[4] = ' ';
	wr += sprintf(wr, "%"PRINT_FMT, choosen), *wr++ = '\n';
	*p++ = '\n';
	return p;
}

int
main(void)
{
	int_t capacity, cap, n, i;

	size_t ptr = 0, size;
	ssize_t r;
	char output[100000UL];
	const register char *restrict p;
	register char *restrict w = output;

	p = mmap(NULL, 1000000UL, PROT_READ, MAP_SHARED, 0, 0);

	for (; *p; p++) {
		p = parseheadpair(p, &capacity, &n);
		cap = capacity + 1;
		for (i = 1; i <= n;) {
			p = parseintpair(&p[1], &values[i], &weights[i]);
			if (weights[i] > cap)
				n--;
			else
				i++;
		}
		w = solve(capacity, n, w);
	}

	size = (size_t)(w - output);
	for (ptr = 0; ptr < size; ptr += (size_t)r)
		r = syscall(SYS_write, 1, &output[ptr], size - ptr);

	return 0;
}
