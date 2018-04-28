#include <sys/syscall.h>
#include <sys/mman.h>
#include <alloca.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef signed int node_t;
typedef unsigned int unode_t;

static node_t *restrict nodes;

static node_t
uf_root(register node_t i)
{
	if (nodes[i] >= 0)
		return (nodes[i] = uf_root(nodes[i]));
	return i;
}

static const char *restrict
parseintpair(register const char *restrict s, register node_t *restrict ap, register node_t *restrict bp)
{
	register unode_t r;

	r = *s & 15;
	while (*++s != ' ')
		r = (r * 10) + (*s & 15);
	*ap = (node_t)r;

	r = *++s & 15;
	while (*++s != '\n')
		r = (r * 10) + (*s & 15);
	*bp = (node_t)r;

	return s;
}

int
main(void)
{
	char *restrict buf;
	register const char *restrict p;
	register char *restrict w;
	size_t size = 16000016, ptr = 0;
	ssize_t r;

	node_t q, a, b;
	char op;

	p = w = buf = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	for (;; ptr += (size_t)r)
		if (!(r = syscall(SYS_read, 0, &buf[ptr], size)))
			break;

	p = parseintpair(p, &a, &q);

	nodes = alloca((size_t)a * sizeof(*nodes));
	memset(nodes, -1, (size_t)a * sizeof(*nodes));

	while (q--) {
		op = p[1];
		p = parseintpair(&p[3], &a, &b);

		a = uf_root(a);
		b = uf_root(b);
		if (__builtin_expect(op == '=', 1)) {
			if (nodes[a] < nodes[b]) {
				nodes[a] += nodes[b];
				nodes[b] = a;
			} else if (a != b) {
				nodes[b] += nodes[a];
				nodes[a] = b;
			}
		} else if (a == b) {
			w[0] = 'y', w[1] = 'e', w[2] = 's', w[3] = '\n', w += 4;
		} else {
			w[0] = 'n', w[1] = 'o', w[2] = '\n', w += 3;
		}
	}

	size = (size_t)(w - buf);
	ptr = 0;
	while (ptr < size) {
		r = syscall(SYS_write, 1, &buf[ptr], size - ptr);
		ptr += (size_t)r;
	}

	return 0;
}
