
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include <sys/time.h>
#include <fcntl.h>


#define HASH_BITS	12
#define	HASH_SIZE	(1 << HASH_BITS)
#define	MHASHAND	(HASH_SIZE -1)


long get_time(void)
{
        struct timeval tv;
        gettimeofday(&tv, (struct timezone *)NULL);
        return ((tv.tv_sec*1000) + (tv.tv_usec / 1000));
}


/*
 * hash function used in original implementation
 * - is not much strong
 * - lenght of symbol is only implicitly reflected by shifting, but if you shift bits out and keep zeros hashes will likely match for different symbols
 * - if lenghts is above threshold hashes will be the same i.e. every symbol with the same postfix longer than 5 chars get the same hash
 */
unsigned int hash1(const char *str)
{
    unsigned int result = 0;
    while (*str) {
	result = (result << 2) ^ *str++;
    }
    return result & MHASHAND;
}

/*
 * proposal #1, shift only 1 instead of two, length of common postfix increased to 10, i.e. chance that two symbols have the same postfix is less, but collisions are much more
 */
unsigned int hash1_mod1(const char *str)
{
    unsigned int result = 0;
    while (*str) {
	result = (result << 1) ^ *str++;
    }
    return result & MHASHAND;
}


static const uint8_t bitrev_array[256] = {
                 0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0, 0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
             0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8, 0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
             0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4, 0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
             0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec, 0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
             0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2, 0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
             0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea, 0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
             0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6, 0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
             0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee, 0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
             0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1, 0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
             0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9, 0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
             0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5, 0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
             0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed, 0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
             0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3, 0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
             0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb, 0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
             0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7, 0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
             0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef, 0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
};


uint16_t bitreversed(uint16_t v)
{
        uint16_t m = 1;
        uint16_t r = 0;

        while (m < 0x8000)  {
                r <<= 1;
                if ((m & v) > 0) {
                        r |= 1;
                }
                m <<= 1;
        }
        return  r;
}


unsigned int fletcher_mod1(const char *str)
{
    uint16_t a = 0;
    uint16_t b = 0;
    while (*str) {
	a += *str++;
	b += a;
    }
//    return (a+b) & MHASHAND;
//    uint16_t c = bitreversed((a & 0xFF00) ^ (b & 0xFF00)) ;
    return ((((a << 8) & 0xFF00) | (b & 0xFF))  ) & MHASHAND;
//    return ((a + b) ^ bitreversed(b)) & MHASHAND;
//    return ((bitreversed(b) >> (16 - HASH_BITS)) ^ a) & MHASHAND;
}



/*
 * proposal #2, add current step, this results in adding Gauss¹ sum of string length     (¹) Carl Friedrich
 */
unsigned int hash1_mod2(const char *str)
{
    unsigned int result = 0;
    int n = 0;

    while (*str) {
	result = (result << 2) ^ *str++;
	result += n;
	n++;
    }
    return result & MHASHAND;
}


/*
 * proposal #3, same as above but more shift, seems to has the least # of collisions but 27% slower
 */
unsigned int hash1_mod3(const char *str)
{
    unsigned int result = 0;
    int n = 0;
    while (*str) {
	result = (result << 3) ^ *str++;
	result += n;
	n++;
    }
    return result & MHASHAND;
}


/*
 * proposal #4, use fletcher checksum, fewer collisions, fastest (8% faster than original)
 */
unsigned int fletcher(const char *str)
{
    uint16_t a = 0;
    uint16_t b = 0;
    while (*str) {
	a += *str++;
	b += a;
    }
    return (((b << 8) & 0xFF00) | (a & 0xFF)) & MHASHAND;
}

typedef unsigned int (*t_hash_handler)(const char *str);


typedef struct {
    t_hash_handler	handler;
    char name[32];
    long time;
    int8_t postfix;
} t_hash_function;


#define		nHashTests	6

t_hash_function hash_function[nHashTests] = {
     {hash1	,  "hash1_orig", 0,-1}
    ,{hash1_mod1,  "hash1_mod1",0,-1}
    ,{hash1_mod2,  "hash1_mod2",0,-1}
    ,{hash1_mod3,  "hash1_mod3",0,-1}
    ,{fletcher,	   "fletcher",0,-1}
    ,{fletcher_mod1,   "fletcher_m1",0,-1}
};




int main(int argc, char *argv[])
{
    /*
	for (int i=0; i<32; i++) {
	    printf("0x%04x  0x%04x\n", i, bitreversed(i) );
	}
	
	exit(0);
    */
	printf("HASH_SIZE: %d, MHASHAND: 0x%04x\n", HASH_SIZE, MHASHAND);
	
	
	for (int k=0; k<nHashTests; k++) {
	    int last = -1;
	    for (int i=0; i<20; i++) {
		char buf[32];
		memset(buf, 0, 32);
		for (int j=0; j<i; j++) {
			buf[j] = 'w';
		}
		int h = hash_function[k].handler(buf);
		printf("i:%d %s(%s) 0x%04x\n", i, buf, hash_function[k].name, h );
		if (last == h) {
		    hash_function[k].postfix = i-1;
		    break;
		}
		last = h;
	    }
	}

	for (int k=0; k<nHashTests; k++) {
	    long start = get_time();
	    for(int i=0; i<10000000; i++) {
		hash_function[k].handler("AntonBertaCaesar");
	    }
	    long delta = get_time();
	    delta -= start;
	    printf("time spent: %ld ms %s\n", delta, hash_function[k].name);
	    hash_function[k].time = delta;
	}


	for (int k=0; k<nHashTests; k++) {
	    fprintf(stderr, "Testing all mnemonics with hash function %s\n", hash_function[k].name);

	    char mnemTxt[4096];
	    memset(mnemTxt, 0, 4096);
	    
	    uint16_t hashtable[HASH_SIZE];
	    memset(hashtable, 0, sizeof(uint16_t)*HASH_SIZE);

	    char symbols[HASH_SIZE][32];
	    memset(symbols, 0, HASH_SIZE*32);
    

	    int fd = open("all_mnemonics.txt", O_RDONLY);
	    if (fd < 0) {
		break;
	    }
	    int r = read(fd, mnemTxt, 4096-1);
	    if (r < 1) {
		break;
	    }
	    char *mnem = &mnemTxt[0];
	    while(mnem != NULL) {
		char *lf = strchr(mnem,'\n');
		if (lf != NULL) {
		    *lf = '\0';
		    uint16_t h = hash_function[k].handler(mnem);
		    if (hashtable[h] > 0) {
			fprintf(stderr,"\thash collision @ 0x%04x [%s][%s]\n", h, symbols[h], mnem);
		    }
		    strcpy(symbols[h], mnem);
		    hashtable[h]++;
		    mnem = lf +1;
		} else {
		    mnem = NULL;
		}
	    }

	    int unused = 0;
	    int singles = 0;
	    int collisions = 0;
	    int maxcol = 0;
	    for (int n=0; n<HASH_SIZE; n++) {
		switch(hashtable[n]) {
		    case 0:	unused++;	break;
		    case 1:	singles++;	break;
		    default:	collisions++;	if (maxcol < hashtable[n]) { maxcol = hashtable[n]; }  break;
		}
	    }

	    printf("\t\tresult[%s]:\t time %ld, postfix:%d,  used %d %d%%, free: %d collisions: %d, worst: %d\n", hash_function[k].name, 
			hash_function[k].time, hash_function[k].postfix,
			singles, (singles*100)/HASH_SIZE, unused, collisions, maxcol);
	}

	return 0;
}




