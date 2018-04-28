#include <sys/mman.h>
#include <sys/stat.h>
#include <alloca.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


typedef signed long int int_t;
typedef unsigned long int uint_t;


static int_t a[5000000];


static inline const char *restrict
parseint(register const char *restrict s, int_t *restrict ap)
{
	register int_t r;

	r = *s & 15;
	while (*++s != '\n')
		r = (r * 10) + (*s & 15);

	*ap = r;
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

	if (*++s != '-') {
		r = *s & 15;
		while (*++s != '\n')
			r = (r * 10) + (*s & 15);
		*bp = (int_t)r;
	} else {
		r = *++s & 15;
		while (*++s != '\n')
			r = (r * 10) + (*s & 15);
		*bp = -(int_t)r;
	}

	return s;
}


int
main()
{
	register const char *restrict p;
	struct stat st;

	int_t n, q, j, delta;
	register int_t rc, i;

	fstat(0, &st);
	p = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, 0, 0);
	p = parseintpair(p, &n, &q);

	while (q--) {
		if (p[1] == '+') {
			p = parseintpair(&p[3], &j, &delta), i = j;
			while (i < n) {
				a[i] += delta;
				i |= i + 1;
			}
		} else {
			p = parseint(&p[3], &j), i = j;
			for (rc = 0; i;) {
				rc += a[i - 1];
				i -= (i & -i);
			}
			printf("%li\n", rc);
		}
	}

	return 0;
}
