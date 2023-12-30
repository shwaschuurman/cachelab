//#define _XOPEN_SOURCE
//Josh Schuurman - joshschuurman

#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <getopt.h>
#include <stdlib.h>
#include "cachelab.h"
#include <string.h>

int hits;
int misses;
int evictions;

//struct for an individual cache line
struct line {
	//valid bit - 0 or 1
	unsigned int v;
	//tag of block
	unsigned long int tag;

	//counter of when this line was last accessed
	unsigned int lastUsed;
};

//initialize a new cache by initializing a 2D array of struct lines (a collection of sets of lines)
struct line **initializeCache(unsigned int S, unsigned int E){
	int i;
	int j;
	
	//malloc a new cache (double array of lines, since sets will be arrays of lines themselves
	struct line ** newCache = (struct line **)malloc(S * sizeof(struct line*)); //S sets, so sizeof(S * the size of a set)
	
	//allocate each individual set
	for(i=0; i<S; i++){
		newCache[i] = (struct line*)malloc(E * sizeof(struct line)); //E lines, so sizeof(E * size of a line)
		//initialize each individual line with 0 for all fields
		for(j=0; j<E; j++){
			newCache[i][j] = (struct line){(unsigned int)0, (unsigned long int)0, (unsigned int)0};
		}
	}
	return newCache;
}

//free all parts of the cache
void freeWholeCache(unsigned int S, struct line **cache){
	int i;

	//free all cache sets
	for(i=0; i<S; i++){
		free(cache[i]);
	}
	
	//free the entire cache
	free(cache);
}

//simulate a cache request
void simulateCache(struct line **cache, unsigned long set, unsigned long tag, unsigned int S, unsigned int E){
	int i;

	int hit = 0; //tracker of whether or not we've hit - set to 1 if we hit=
	int emptyLine = -1; //index of empty line if we have a cold miss

	//iterate through each line in set 
	for(i=0; i<E; i++){
		struct line curLine = cache[set][i]; //the current line we're checking

		//if the line contains a valid block
		if(curLine.v){
			//block is the correct block
			if(curLine.tag == tag){
				hits++;
				hit = 1;
				cache[set][i].lastUsed = 0; //rest the counter so it won't be evicted
				break; 	//leave loop since cache hit
			}
			//block isn't stored in this line
			else{
				cache[set][i].lastUsed++; //increment lastUsed for this block, since we've made a cache request and this isn't the correct block.
			}
		}
		//cold miss - remember where this empty line is
		else{
			emptyLine = i;
		}
	}

	//miss
	if (!hit){
		misses++;
		//cold miss, no block in emptyLine, so we can just write to the cache without evicting.
		if(emptyLine != -1){
			cache[set][emptyLine] = (struct line){1, tag, 0};
		}
		//conflict/capacity miss, need to evict
		else{
			evictions++;
			int maxLRU = 0; //keeps track of the maximum lastUsed we've seen
			int maxLRUindex = -1;

			//find the LRU line
			for(i=0; i<E; i++){
				//LRU line has to be valid
				if(cache[set][i].v && cache[set][i].lastUsed >= maxLRU){
					maxLRU = cache[set][i].lastUsed;
					maxLRUindex = i;
				}
			}

			//evict the old block and write the new one
			cache[set][maxLRUindex] = (struct line){1, tag, 0};
		}
	}
}

//parse the trace file passed as a parameter
void parseTrace(char traceFile[], unsigned int b, unsigned int s, unsigned int S, unsigned int E, struct line ** cache){
	FILE * file = fopen(traceFile, "r");
	char op;
	unsigned long addr;
	unsigned int size;

	//while not at the EOF, parse the lines to get the operation, address, and size
	while(fscanf(file, " %c %lx,%d", &op, &addr, &size) != EOF){
		//do nothing if it's 'I'
		if(op == 'I') continue;
		//cases 'M', 'L', and 'S'
		else{
			if(op == 'M') hits++; //'M' is write and read, you know it will be a hit on the read after the write, so just increment it manually and then fall through to cases 'S' and 'L''
			unsigned long tag = addr >> s;			//get the tag by shifting
			tag = tag >> b;					// ""
			unsigned long index = addr >> b & (S - 1);	//get the index by shifting and masking
			simulateCache(cache, index, tag, S, E);		//simulate a cache request
		}
	}
	fclose(file); 
}


int main(int argc, char **argv)
{
	hits = 0;
	misses = 0;
	evictions = 0;

	//s, E, b stand for what they usually do
	unsigned int s = 0;
	unsigned int E = 0;
	unsigned int b = 0;

	//empty string for now
	char file[100] = "";
	int init = 0;

	//get the parameters
	while((init = getopt(argc, argv, "s:E:b:t:")) != -1){
		switch(init){
			case 's':
				s = atoi(optarg);
				break;
			case 'E':
				E = atoi(optarg);
				break;
			case 'b':
				b = atoi(optarg);
				break;
			case 't':
				strcpy(file, optarg);
				break;
			default:
				break;
		}
	}

	//S = 2^s
	unsigned int S = 1 << s;
	//initialize a new cache
	struct line **thisCache = initializeCache(S, E);
	//parse the trace file - this will also simulate the cache
	parseTrace(file, b, s, S, E, thisCache);
	//clean up the cache
	freeWholeCache(S, thisCache);

	printSummary(hits, misses, evictions);
	return 0;
}
