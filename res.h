#ifndef RES_H
#define RES_H

#include "th.h"
#include "sbuf.h"

// contenedor de resultados
typedef struct
{
	t_th th;	// tabla hash
	t_sbuf sbuf;	// buffer texto
	
} t_res;

// creamos contenedor de resultados
void res_init(t_res *res, size_t max_sz);

// liberamos contenedor resultados
void res_free(t_res *res);

// reset resultados
void res_reset(t_res *res);

// actualizamos valor asociado a resultado 
void res_add(t_res *res, const char *key, int val);

// ordena por valor
void res_order(t_res *res);
	
// actualiza array global de resultados
void res_merge(t_res res_chk, t_res *res);

// mostramos resultados 
void res_show(t_res res, int limit);

// reducimos y ordenamos resultados
void res_reduce(t_res *res);

#endif
