#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#define PATH 				"../logs/"

#define FILE_dat 			PATH "%s.dat"

#define FILE_s_sitename 		PATH "s_sitename.idx"
#define FILE_s_computername 		PATH "s_computername.idx"
#define FILE_cs_method 			PATH "cs_method.idx"
#define FILE_cs_username 		PATH "cs_username.idx"
#define FILE_cs_version			PATH "cs_version.idx"
#define FILE_cs_user_agent 		PATH "cs_user_agent.idx"

#define SZ_PATH 			256
#define SZ_CHK 				65536
#define SZ_SBUF1 			4096
#define SZ_SBUF2 			512
#define SZ_FILE_ID 			7

#define SZ_SBUF         		SZ_CHK * SZ_SBUF2 
#define SZ_ZBUF         		SZ_SBUF * 1.1

#define SZ_MAX_CHUNKS 			10000

#define MAX_THREADS 			15
#define NUM_FILES 			2

//#define DEBUG 				1


/*************************
 * registro
 *************************/

typedef struct
{
	uint8_t magic;				//: 0x99
	time_t date_time;			//: 2012-06-20 03:38:17
	uint8_t s_sitename;			//: (idx) W3SVC3
	uint8_t s_computername;			//: (idx) P2396083
	uint32_t s_ip;				//: (ip) 216.157.75.24
	uint8_t cs_method;			//: (idx) GET
	uint16_t cs_uri_stem;			//: (var) /
	uint16_t cs_uri_query;			//: (var) - 
	uint16_t s_port;			//: 80 
	uint16_t cs_username;			//: (idx) - 
	uint32_t c_ip;				//: (ip) 66.223.57.120
	uint8_t cs_version;			//: (idx) HTTP/1.1
	uint16_t cs_user_agent;			//: (idx) Mozilla/5.0+(Windows+NT+5.1)+AppleWebKit/536.5+(KHTML,+like+Gecko)+Chrome/19.0.1084.56+Safari/536.5
	uint16_t cs_cokie;			//: (var) - 
	uint16_t cs_referer;			//: (var) -
	uint16_t cs_host;			//: (var) 216.157.75.24 
	uint16_t sc_status;			//: 200
	uint16_t sc_substatus;			//: 0
	uint16_t sc_win32_status;		//: 0 
	uint16_t sc_bytes;			//: 426
	uint16_t cs_bytes;			//: 366 
	int time_taken;				//: 1279

} t_reg;


/************************
 * estructura fichero logs
 ************************/

// meta info chunk 
typedef struct
{
	// tamanio chunk
	size_t sz;

	// tamanio chunks texto
	size_t sz_cs_uri_stem;
	size_t sz_cs_uri_query;
	size_t sz_cs_cokie;
	size_t sz_cs_referer;
	size_t sz_cs_host;

} t_chunk;

// tipo fichero datos
typedef struct
{
	// control de errores 
	int magic;

	// numero total de chunks
	int num_chunks;

	// tabla chunks 
	t_chunk chunks[SZ_MAX_CHUNKS];

} t_file_dat;


// arrays
extern const char *FILES[];
extern const char *UNKNOWN[];


// conversion formato ip integer a string
char *ip_str(const uint32_t ip, char *str);

// conversion formato ip string a integer
uint32_t ip_int(const char *ip);

// reserva memoria con excepcion
void *xmalloc(size_t sz);

// gestion memoria
void *xcalloc(size_t num, size_t sz);

// convierte a minuscula
char *strlow(char *s);

// funcion hash fnv1
int fnv1_hash(const char *key);

// funcion hash bernstein
unsigned djb_hash (void *key, int len);

//  no warnings ?????
char *strptime(const char *s, const char *format, struct tm *tm);

// apertura de ficheros 
FILE *xfopen(const char *fname, const char *mode);

// descomprime zbuf
size_t uncompress_snappy(void *zbuf, size_t sz_z, void *sbuf);

// comprime sbuf
size_t compress_snappy(void *sbuf, size_t sz, void *zbuf);

// compresion gzlib
size_t compress_gz(void *sbuf, size_t sz, void *zbuf);

// descompresion gzlib
size_t uncompress_gz(void *zbuf, size_t sz_z, void *sbuf);

#endif
