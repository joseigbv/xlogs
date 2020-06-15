#ifndef SET_H
#define SET_H

#include "common.h"
#include "sbuf.h"
#include "idx.h"

/* definicion de libreria para conjunto binario de hasta 256 items */

typedef uint8_t t_set[32];

typedef struct
{
	t_idx *idx;		// conv. id -> string
	t_set set[SZ_CHK];	// conjunto
	t_sbuf sbuf;		// buffer texto dinamico
	int next_id;
	int max_sz;

} t_sets;

// inicializa estructura
void sets_init(t_sets *sets, int max_sz, t_idx *idx);

// reset estructuras
void sets_reset(t_sets *sets);

// carga datos en conjunto
void sets_load(t_sets *sets, const char *fname);

// salva datos en conjunto
void sets_save(t_sets sets, const char *fname);

// crea conjunto a partir de strs; ...
void sets_conv(t_sets *sets, char *str);

// strs from bitmap
char *sets_str(t_sets sets, int id, char *str, int sz);

// libera recursos
void sets_free(t_sets *sets);

// configura conjunto con datos en sbuf
void sets_set(t_sets *sets);

#endif
