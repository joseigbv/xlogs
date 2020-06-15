#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "idx.h"
#include "common.h"
#include "th.h"


// inicializa array de indices
void idx_init(t_idx *idx, int max_sz)
{
	// inicializamos variables
	idx->next_id = 0;
	idx->max_sz = max_sz;

	// reservamos memoria para ptr a texto
	idx->txt = xmalloc(max_sz * sizeof(char *));

	// inicializamos buffers texto
	sbuf_init(&idx->sbuf, max_sz * SZ_SBUF2);
	
	// creamos tabla hash
	th_init(&idx->th, max_sz);
}


// carga indice desde sbuf
void idx_set(t_idx *idx)
{
	int sz = 0;
	char *ptr = idx->sbuf.txt;

	// indexamos strings (sep = \0)
	while (sz < idx->sbuf.sz) 
	{
		// superado el maximo de registros permitido?
		if (idx->next_id > idx->max_sz)
		{
			fprintf(stderr, "Error: superado el numero maximo de entradas en indice\n");
			exit(EXIT_FAILURE);
		}

		// fin de cadena?
		if (idx->sbuf.txt[sz] == 0)
		{
			// registramos cadena de texto
			idx->txt[idx->next_id] = ptr;

			// insertamos en tabla hash
			th_insert(&idx->th, ptr, idx->next_id);

			// siguiente cadena
			ptr = ++sz + idx->sbuf.txt; 
			idx->next_id ++;

		}

		sz++;
	}
}

// carga sbuf e indexa ...
void idx_load(t_idx *idx, const char *fname)
{
	// cargamos buffers de texto
	sbuf_load(&idx->sbuf, fname);

	// configuramos indices
	idx_set(idx); 
}

// salva a fichero 
void idx_save(t_idx idx, const char *fname)
{
	// pendiente
	sbuf_save(idx.sbuf, fname);
}

// reinicia estructuras
void idx_reset(t_idx *idx)
{
	idx->next_id = 0;

	// reiniciamos buffers
	sbuf_reset(&idx->sbuf);

	// reiniciamos tabla hash
	th_reset(&idx->th);
}

// libera memoria de array indices
void idx_free(t_idx *idx)
{
	// liberamos array de ptr a texto
	free(idx->txt);
	
	// liberamos buffers
	sbuf_free(&idx->sbuf);

	// liberamos tabla hash
	th_free(&idx->th);
}

// devuelve un elto de la tabla
char *idx_txt(t_idx idx, int id)
{
	return idx.txt[id < idx.max_sz ? id : 0];
}

// devuelve el id de un elto
int idx_id(t_idx idx, char *txt, size_t sz)
{
	return th_search(idx.th, txt);
}

// inserta un nuevo elto 
int idx_insert(t_idx *idx, const char *key, size_t sz)
{
	char *ptr;
	uint id;

	// control de errores
	if (idx->next_id > idx->max_sz)
	{	
		// pendiente... 
		return 0; 

		fprintf(stderr, "Error: superado el numero maximo de entradas por indice\n");
		exit(EXIT_FAILURE);
	}

	// no existe el item ?
	if ((id = th_search(idx->th, key)) == -1)
	{
		// guardamos texto en buffer
		ptr  = idx->sbuf.txt + sbuf_offset(&idx->sbuf, key, sz);

		// registramos texto
		idx->txt[idx->next_id] = ptr;

		// generamos hash
		th_insert(&idx->th, ptr, idx->next_id);

		// devolvemos el id 
		id = idx->next_id++;
	}

	return id;
}

// carga indice desde array
void idx_default(t_idx *idx, const char *keys[])
{
	while (*keys)
	{
		idx_insert(idx, *keys, strlen(*keys) + 1);
		keys++;
	}
}

// para debug
void idx_list(t_idx idx)
{
	int i;

	// recorremos array y mostramos
	for (i = 0; i < idx.next_id; i++)
		printf("%d - %s\n", i, idx.txt[i]);
}

