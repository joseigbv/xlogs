#ifndef IDX_H
#define IDX_H

#include "common.h"
#include "sbuf.h"
#include "th.h"


/*************************
 * indices
 *************************/

// indices  en array
typedef struct
{
        char **txt;		// array strings
	int max_sz;		// tamanio maximo 
	int next_id;		// ???
	int max_len;		// longitud texto
	t_sbuf sbuf;		// buffer texto
	t_th th;		// tabla hash

} t_idx;

// inicializa array de indices
void idx_init(t_idx *idx, int max_sz);

// carga buffer
void idx_load(t_idx *idx, const char *fname);

// salva buffer
void idx_save(t_idx idx, const char *fname);

// devuelve un elto de la tabla
char *idx_txt(t_idx idx, int id);

// devuelve el id de un elto de la tabla
int idx_id(t_idx idx, char *txt, size_t sz);

// inserta un nuevo elto 
int idx_insert(t_idx *idx, const char *key, size_t sz);

// carga indice desde array
void idx_default(t_idx *idx, const char *keys[]);

// reinicia estructuras
void idx_reset(t_idx *idx);

// libera memoria de array indices
void idx_free(t_idx *idx);

// para debug
void idx_list(t_idx idx);

// aux config idx
void idx_set(t_idx *idx);

#endif
