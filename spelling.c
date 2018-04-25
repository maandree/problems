#include <sys/mman.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define CORR_SIZE  400UL
#define INPUT_SIZE 5000001UL
#define DICT_SIZE  400000L


typedef struct word {
	char *string;
	size_t length;
	size_t reusable;
} word_t;


#define abs(a)    __builtin_abs(a)
#define min(a, b) ((a) < (b) ? (a) : (b))
#define min3(a, b, c)\
	({\
		typeof(a) c_ = c;\
		typeof(a) ab = min(a, b);\
		ab < c_ ? ab : c_;\
	})


#define fputc fputc_unlocked

#define printstr(str) output(str)
#define printchr(chr) fputc(chr, stdout)
#define printfmt(...) printf(__VA_ARGS__)

static inline void
output(register const char *restrict s)
{
	for (; *s; s++) {
		if (__builtin_expect(*s & 0x80, 0))
			fputc(0xC3, stdout);
		fputc(*s, stdout);
	}
}


static size_t M[41][41] = {
	{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40},
	{1},{2},{3},{4},{5},{6},{7},{8},{9},{10},{11},{12},{13},{14},{15},{16},{17},{18},{19},{20},{21},{22},{23},{24},
	{25},{26},{27},{28},{29},{30},{31},{32},{33},{34},{35},{36},{37},{38},{39},{40}
};

static ssize_t dict_end = DICT_SIZE;
static word_t dict[DICT_SIZE];

static const char *restrict corr[CORR_SIZE];
static size_t corr_ptr = 0;
static size_t closest_dist;
static size_t filled_rows = 0;


static size_t
distance(const word_t *restrict proper, size_t y0, const word_t *restrict used)
{
	register size_t yn = proper->length, xn = used->length;
	register char p;
	size_t *restrict my = M[y0];
	size_t *restrict myy;
	size_t y, x;
	ssize_t change, remove, add;

	for (y = y0; y < yn; y++) {
		myy = M[y + 1];
		p = proper->string[y];

		if (my[xn - yn + y] > closest_dist) {
			filled_rows = y;
			return SSIZE_MAX;
		}

		for (x = 0; x < xn;) {
			change = (ssize_t)(my[x]);
			if (p == used->string[x])
				change--;
			remove = (ssize_t)(myy[x]);
			add = (ssize_t)(my[++x]);

			myy[x] = (size_t)(1 + min3(change, add, remove));
		}

		my = myy;
	}

	filled_rows = yn;
	return my[xn];
}


static void
correct(const word_t *restrict used)
{
	register size_t reuseable_rows = 0, dist;
	register size_t used_length = used->length;
	const word_t *restrict proper;
	ssize_t d;

	closest_dist = SSIZE_MAX;

	for (d = DICT_SIZE - 1; d > dict_end; d--) {
		proper = dict + d;
		reuseable_rows = min(proper->reusable, reuseable_rows);

		if ((size_t)abs((ssize_t)used_length - (ssize_t)(proper->length)) > closest_dist)
			continue;

		dist = distance(proper, reuseable_rows, used);
		reuseable_rows = filled_rows;

		if (closest_dist > dist) {
			closest_dist = dist;
			corr[0] = dict[d].string;
			corr_ptr = 1;
		} else if (closest_dist == dist) {
			corr[corr_ptr++] = dict[d].string;
		}
	}

	return;
}


static inline size_t
start_similarity(register const char *restrict a, register const char *restrict b)
{
	register size_t i = 0;
	while (a[i] == b[i] && a[i]) i++;
	return i;
}


int
main(void)
{
	size_t k;
	size_t ptr = 0;
	ssize_t got;
	char *restrict input = mmap(NULL, INPUT_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	register char *restrict r;
	register char *restrict w;
	word_t used;
	register const char *restrict last_word = "";

	while ((got = read(0, &input[ptr], INPUT_SIZE)))
		ptr += (size_t)got;
	input[ptr] = '\0';

	for (r = input; *r && (unsigned char)*r != 0xC3; r++);
	for (w = r; *r; r++)
		if ((unsigned char)*r != 0xC3)
			*w++ = *r;
	*w = '\0';

	dict[--dict_end].string = input;
	for (w = r = input; *r != '#'; r++) {
		while (*r != '\n') r++;
		*r = '\0';
		dict[dict_end].length = (size_t)(r - w);
		dict[dict_end].reusable = start_similarity(dict[dict_end].string, last_word);
		last_word = dict[dict_end].string;
		dict[--dict_end].string = w = &r[1];
	}
	r = &r[2];

	used.string = r;
	for (w = r; *r; r++) {
		while (*r != '\n') r++;
		used.length = (size_t)(r - w);
		used.string[used.length] = '\0';

		correct(&used);

		printstr(used.string);
		printchr(' ');
		printchr('(');
		printfmt("%zu)", closest_dist);
		for (k = 0; k < corr_ptr; k++) {
			printchr(' ');
			printstr(corr[k]);
		}
		printchr('\n');

		used.string = w = &r[1];
	}

	return 0;
}
