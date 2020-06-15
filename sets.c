#include <stdio.h>
#include <string.h>

#include "common.h"
#include "sets.h"
#include "idx.h"

#define MAX_ITEMS 255

// inicializa estructura
void sets_init(t_sets *sets, int max_sz, t_idx *idx)
{
	sbuf_init(&sets->sbuf, SZ_CHK * MAX_ITEMS);
	memset(sets->set, 0, SZ_CHK * sizeof(t_set));
	sets->idx = idx;
	sets->next_id = 0;
	sets->max_sz = max_sz;
}

// reset estructura
void sets_reset(t_sets *sets)
{
	sbuf_reset(&sets->sbuf);
	memset(sets->set, 0, SZ_CHK * sizeof(t_set));
	sets->next_id = 0;
}

// liberamos estructuras
void sets_free(t_sets *sets)
{
	sbuf_free(&sets->sbuf);
}

// convierte cadena a conjunto
void sets_conv(t_sets *sets, char *txt)
{
	char *ptr, sbuf[MAX_ITEMS];
	int id = 0, item = 0, sz;

	// apuntamos principio cadena
	ptr = txt;

	while (txt[id]) 
	{
		// calculamos id para cadena
		if (txt[id] == ';')
		{
			sz = txt + id - ptr + 1;
			txt[id] = 0;
			sbuf[item++] = idx_insert(sets->idx, ptr, sz);
			txt[id] = ';';
			ptr = txt + id + 1;
		}
	
		// siguiente caracter
		id++;
	}

	// ultimo elemento ?
	if (id) 
	{
		sz = txt + id - ptr + 1;
		sbuf[item++] = idx_insert(sets->idx, ptr, sz);
	}

	// terminamos en nulo
	sbuf[item++] = 0;

	// guardamos en buffer
	sbuf_offset(&sets->sbuf, sbuf, item);
}

// guardar a disco
void sets_save(t_sets sets, const char *fname)
{
	sbuf_save(sets.sbuf, fname);
}

void sets_debug(t_set set)
{
	int i;

	for (i = 32; i ; i--) 
		printf("%02x", set[i - 1]);
	
	printf("\n");
}

// configura conjunto con datos en buffer
inline void sets_set(t_sets *sets)
{
        char *ptr;
	uint8_t id;
        int i;

        // cargamos buffers de texto
        ptr = sets->sbuf.txt;

        // recorremos buffer
        for (i = 0; i < sets->sbuf.sz; i++)
        {
		// leemos siguiente byte
		id = sets->sbuf.txt[i];

                // fin de cadena? siguiente reg
                if (id == 0)
                {
                        // superado el maximo de registros permitido?
                        if (sets->next_id == sets->max_sz)
                        {
                                fprintf(stderr, "Error: superado el numero maximo de items\n");
                                exit(EXIT_FAILURE);
                        }

			sets->next_id++;
                        ptr = i + sets->sbuf.txt + 1;
                }
		else 
			// construimos conjunto
			sets->set[sets->next_id][id / 8] = 
				sets->set[sets->next_id][id / 8] | 
				(1 << ((id % 8) - 1));
        }
}

// carga datos conjunto y descomprime
void sets_load(t_sets *sets, const char *fname)
{
        // cargamos buffers de texto
	sbuf_load(&sets->sbuf, fname);

	// configuramos conjuntos
	sets_set(sets);
}

// strs from bitmap
char *sets_str(t_sets sets, int id, char *txt, int sz)
{
	int i, j; 

	// inicializamos str
	*txt = 0;

	// comprobamos 255 bits
	for (i = 0; i < sizeof(t_set); i++)
		for  (j = 0; j < 8; j++)

			if (sets.set[id][i] & (1 << j))
			{
				strncat(txt, sets.idx->txt[8 * i + j + 1], sz);
				strncat(txt, ";", 1);
			}

	// por defecto desconocido
	if (*txt == 0) strncat(txt, "unknown", sz);
	
	return txt;
}

