#include <stdio.h>
#include <string.h>

#include "common.h"
#include "sbuf.h"


// inicializa estructuras
void sbuf_init(t_sbuf *sbuf, size_t max_sz)
{
	sbuf->sz = 0; 
	sbuf->max_sz = max_sz;
        sbuf->txt = xmalloc(sbuf->max_sz);

	// reservamos buffer compresion
	sbuf->zbuf = xmalloc(sbuf->max_sz);
}

// reinicializa estructuras
void sbuf_reset(t_sbuf *sbuf)
{
	sbuf->sz = 0; 
}

// elimina estructuras
void sbuf_free(t_sbuf *sbuf)
{
	free(sbuf->txt);
	free(sbuf->zbuf);
}

// offset a la siguiente cadena comprimida
int sbuf_offset(t_sbuf *sbuf, const char *txt, size_t sz)
{
	int r = sbuf->sz;
	char *ptr;

	// superada longitud total ?
	if (sbuf->sz > sbuf->max_sz)
	{
		fprintf(stderr, "Error: superada longitud maxima de buffer\n");
		exit(EXIT_FAILURE);
	}

	ptr = sbuf->txt + sbuf->sz; 
	strncpy(ptr, txt, sz);
	sbuf->sz += sz;

	return r;
}

// configura buffer
inline void sbuf_set(t_sbuf *sbuf, size_t sz)
{
	// configuramos long
	sbuf->sz = sz; 

	// control de errores
	if (sbuf->sz > sbuf->max_sz)
	{
		fprintf(stderr, "Error: superada longitud maxima de buffer\n");
		exit(EXIT_FAILURE);
	}
}

// carga strings a memoria
void sbuf_load(t_sbuf *sbuf, const char *fname)
{
        FILE *f;
	size_t sz;

	// cargamos datos
	f = xfopen(fname, "rb");
	sz = fread(sbuf->zbuf, 1, sbuf->max_sz, f);
	fclose(f);

	// descomprimimos
	sz = uncompress_gz(sbuf->zbuf, sz, sbuf->txt);

	// configuramos buffer
	sbuf_set(sbuf, sz);
}

// salva buffer con strincs comprimidos a fichero
void sbuf_save(t_sbuf sbuf, const char *fname)
{
	FILE *f;
	size_t sz;

	// comprimimos 
	sz = compress_gz(sbuf.txt, sbuf.sz, sbuf.zbuf);

	// salvamos a disco
	f = xfopen(fname, "wb");
	fwrite(sbuf.zbuf, 1, sz, f);
	fclose(f);
}

