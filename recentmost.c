/*
many a times we want to find out most recently modified files on system 
within a directory - it may be codebase, or configuration files, where
we might have been changing things in order to make it work - but in the
end when it finally began working, we lost track of which files had we
touched.
There are already ways to query most recently modified files,
One is (we'll call it existingCmds1):

$find ~/ -type f -printf '%TY-%Tm-%Td %TT %p\n' | sort  | tail -20

(where 20 is the number of most recently modified files one would like to get). 

existingCmds1 works, if one's "find" supports -printf option (find found on 
many of the systems do not). Plus existingCmds1 takes lot more time than needed.

Another way to achieve it is (let us call it existingCmds2):

$find ~/xtg-main/ -type f|grep -v " "|xargs ls -lrt | tail -20

but xargs starts splitting the arguments if they are too long - so existingCmds2
does not achieve the objective.

This utility, assuming that we build it to create executable "recentmost" 
(gcc -Wall -Wextra -o recentmost recentmost.c),
can be used as follows (recentmost-cmd):

$find ~/ -type f|./recentmost 20

On my system, existingCmds1 is taking ~9 secs on a dataset, and recentmost-cmd for the same dataset, takes ~1 sec.
*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
// Thanks to https://gist.github.com/martinkunev/1365481
// from which basic heap related code was originally taken
struct filetimestamp {
	char* name;
	long long modtime;
	int id;
};
typedef struct filetimestamp* HeapElement;
struct heap {
	size_t size;
	size_t count;
	HeapElement *data;
};
static int instance_count = 0;
#define heap_front(h) (*(h)->data)
#define heap_term(h) (free((h)->data))
#define CMP(a, b) ((a->modtime) <= (b->modtime))
HeapElement
heap_newElement(long long modtime, char* filepath) {
	++instance_count;
	HeapElement elem = malloc(sizeof(struct filetimestamp));
	elem->id = instance_count;
	elem->modtime = modtime;
	elem->name = malloc((1+strlen(filepath))*sizeof(char));
	strcpy(elem->name, filepath);
	return elem;
}
void
heap_freeElement(HeapElement elem) {
	if (!elem) return;
	if (elem->name) free(elem->name);
	free(elem);
}
void
heap_init(struct heap *h, int size) {
	*h = (struct heap){
		.size = size,
		.count = 0,
		.data = malloc(sizeof(HeapElement) * size)
	};
	if (!h->data) _exit(1);
}
HeapElement
heap_pop(struct heap *h) {
	unsigned int index, swap, other;
	if (h->count==0) {
		return NULL;
	}
	HeapElement poppedElem = h->data[0];

	HeapElement temp = h->data[--h->count];
	for(index = 0; 1; index = swap) {
		swap = (index << 1) + 1;
		if (swap >= h->count) break;
		other = swap + 1;
		if ((other < h->count) && CMP(h->data[other], h->data[swap])) swap = other;
		if CMP(temp, h->data[swap]) break;
		h->data[index] = h->data[swap];
	}
	h->data[index] = temp;
	return poppedElem;
}
int
heap_push(struct heap *h, HeapElement value) {
	unsigned int index, parent;
	if (h->count>0) {
		if (h->count < h->size) {
			// go ahead
		} else {
			HeapElement min_in_top = heap_front(h);
			if (CMP(min_in_top, value)) {
				heap_freeElement(heap_pop(h));
			} else {
				return 0;
			}
		}
	}
	for(index = h->count++; index; index = parent)
	{
		parent = (index - 1) >> 1;
		if CMP(h->data[parent], value) break;
		h->data[index] = h->data[parent];
	}
	h->data[index] = value;
	return 1;
}
void
offertoheap(struct heap *h, char *filepath) {
	struct stat st;
	HeapElement elem = NULL;
	if (stat(filepath, &st)) {
		fprintf(stderr, "error: filepath '%s' not found", filepath);
	} else {
		elem = heap_newElement(st.st_mtim.tv_sec, filepath);
		if (!heap_push(h, elem)) {
			heap_freeElement(elem);
			elem = NULL;
		}
	}
}
#define LINEBUFLEN 10
void processLines(struct heap *h) {
	int c = EOF;
	size_t linelen = 0;
	char* filepath = NULL;
	size_t allocated = LINEBUFLEN+1;
	char *linebuf = (char*)malloc(sizeof(char)*allocated);
	while (1) {
		c = fgetc(stdin);
		if (c == EOF) {
			break;
		}
		if (linelen >= allocated) {
			allocated += LINEBUFLEN;
			linebuf = realloc(linebuf, sizeof(char) * allocated);
		}
		if (c == '\n') {
			linebuf[linelen] = '\0';
			offertoheap(h, linebuf);
			memset(linebuf, '\0', sizeof(linebuf));
			linelen = 0;
			continue;
		}
		linebuf[linelen++] = (unsigned char)c;
	}
	free(filepath);
}
void
usage(char* progname) {
	fprintf(stderr, "Usage:%s <filecount>\n", progname);
}
int
checkInputs(int argc, char** argv, int* N) {
	if (argc<2) {
		usage(argv[0]);
		return 0;
	}
	*N = atoi(argv[1]);
	if (*N==0) {
		usage(argv[0]);
	}
	return *N;
}
int
main(int argc, char** argv) {
	int i, N; struct heap hh;
	if (!checkInputs(argc, argv, &N))
		return -1;

	heap_init(&hh, N);
	processLines(&hh);
	HeapElement popped;
	for (i=0; i<N*10; i++) {
		popped = heap_pop(&hh);
		if (!popped)
			break;
		printf("%s\n", popped->name);
		heap_freeElement(popped);
	}
	heap_term(&hh);
	return 0;
}
