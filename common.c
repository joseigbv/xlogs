#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "common.h"
#include "zlib.h"

// ficheros de datos (pruebas)
const char *FILES[] = 
{ 
	"w3svc1", "w3svc3"
};

// precarga configuracion por defecto
const char *UNKNOWN[] = 
{ 
	"unknown",
	NULL
};


/*************************
 * varias aux 
 *************************/

// conversion formato ip integer a string
char *ip_str(const uint32_t ip, char *str)
{
	unsigned char o1, o2, o3, o4; 

	o1 = (0xFF000000 & ip) >> 24; 
	o2 = (0x00FF0000 & ip) >> 16; 
	o3 = (0x0000FF00 & ip) >> 8; 
	o4 = 0x000000FF & ip; 

	sprintf(str, "%d.%d.%d.%d", o1, o2, o3, o4);

	return str;
}

// conversion formato ip string a integer
uint32_t ip_int(const char *ip)
{
	int o1, o2, o3, o4;
	
	sscanf(ip, "%d.%d.%d.%d", &o1, &o2, &o3, &o4);
	return (o1 << 24) + (o2 << 16) + (o3 << 8) + o4;
}

// reserva memoria con excepcion
void *xmalloc(size_t sz)
{
	void *ptr;

	if (!(ptr = malloc(sz)))
	{
		fprintf(stderr, "Error de asignacion de memoria\n");
		exit(EXIT_FAILURE);
	}

	return ptr;
}

// gestion memoria
void *xcalloc(size_t num, size_t sz)
{
        void *r;

        if ((r = calloc(num, sz)) == NULL)
        {
                fprintf(stderr, "Error al reservar memoria\n");
                exit(-1);
        }

        return r;
}

// convierte a minuscula
char *strlow(char *s)
{
        int i;

        for(i = 0; i < strlen(s); i++)
                s[i] = tolower(s[i]);

        return s;
}

// funcion hash fnv1
inline int fnv1_hash(const char *key)
{
        const char *s;
        uint hash = 2166136261;

        for (s = key; *s; s++)
        hash = (16777619 * hash) ^ (*s);

        return hash;
}

// funcion hash bernstein
unsigned djb_hash (void *key, int len)
{
	unsigned char *p = key;
	unsigned h = 0;
	int i;

	for ( i = 0; i < len; i++ )
	h = 33 * h + p[i];

	return h;
}

// apertura de ficheros 
FILE *xfopen(const char *fname, const char *mode)
{
	FILE *f;

	// abrimos ficheros
	if ((f = fopen(fname, mode)) == 0)
	{
		fprintf(stderr, "Error al abrir fichero '%s'\n", fname);
		exit(EXIT_FAILURE);
	} 

	return f;
}

// compresion gzlib
size_t compress_gz(void *sbuf, size_t sz, void *zbuf)
{
	size_t sz_z = SZ_ZBUF; 

	// comprimimos
	if (compress(zbuf, &sz_z, sbuf, sz) != Z_OK)
	{
		fprintf(stderr, "Error al comprimir datos\n");
		exit(EXIT_FAILURE);
	}

	return sz_z;
}

// descompresion gzlib
size_t uncompress_gz(void *zbuf, size_t sz_z, void *sbuf)
{
	size_t sz = SZ_SBUF; 

	// descomprimimos
	if (uncompress(sbuf, &sz, zbuf, sz_z) != Z_OK)
	{	
		fprintf(stderr, "Error al descomprimir datos\n");
		exit(EXIT_FAILURE);
	}

	return sz;
}

