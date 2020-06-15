#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "sbuf.h"
#include "res.h"

/*

 manejamos los resultados utilizando una tabla hash

 */

// creamos contenedor de resultados
void res_init(t_res *res, size_t max_sz)
{
	th_init(&res->th, max_sz);
	sbuf_init(&res->sbuf, max_sz * SZ_SBUF1);
}

// liberamos contenedor resultados
void res_free(t_res *res)
{
	th_free(&res->th);
	sbuf_free(&res->sbuf);
}

// reset resultados
void res_reset(t_res *res)
{
	th_reset(&res->th);
	sbuf_reset(&res->sbuf);
}

// actualizamos valor asociado a resultado 
void res_add(t_res *res, const char *key, int val)
{
	t_th_node *node; 
	char *ptr = (char *) key;

	// existia key en buffer ? 
	if (th_search(res->th, ptr) == -1)

		// no? la creamos
		ptr = res->sbuf.txt + 
			sbuf_offset(&res->sbuf, key, 
			strlen(key) + 1);

	// buscamos entrada hash
	node = th_node(&res->th, ptr);
	node->val += val;
}

#define VAL(x)  (((t_th_node *)x)->val)

// aux comparacion por valor
inline int cmp(const void *n1, const void *n2)
{
	return (VAL(n2) - VAL(n1));
}

// ordena por valor
void res_reduce(t_res *res)
{
	int sz, limit;
	t_th_node *pn;

	// recogemos tabla key/val
 	sz = res->th.next_id;
	pn  = res->th.tbl;

	// ordenamos array por valor
	qsort(pn, sz, sizeof(t_th_node), cmp);

	// reducimos al 10% por optimizar
	limit = res->th.max_sz * 0.1;

	// estamos por debajo de limite?
	if (sz < limit) limit = sz;

	// truncamos tabla
	res->th.next_id = limit;

	// reconstruimos tabla
	th_rebuild(&res->th);
}

// actualiza array global de resultados
void res_merge(t_res res_chk, t_res *res)
{
	int i, val; 
	char *key;
	t_th_node *pn;

	for (i = 0; i < res_chk.th.next_id; i++)
	{
		// leemos de origen
		key = res_chk.th.tbl[i].key; 
		val = res_chk.th.tbl[i].val; 
		
		// existe key ?
		if (th_search(res->th, key) == -1) 

			// no? la creamos
			key = res->sbuf.txt + 
				sbuf_offset(&res->sbuf, key, 
				strlen(key) + 1);

		// insertamos en destino
		pn = th_node(&res->th, key);
		pn->val += val;
	}
}

// mostramos resultados 
void res_show(t_res res, int limit)
{
	int i; 
	t_th_node node;

	// control errores
	if (limit > res.th.next_id) 
		limit = res.th.next_id;

	// pendiente mejorar
	for (i = 0; i < limit; i++)
	{
		node = res.th.tbl[i]; 
		printf ("%d) %s : %d\n", i, node.key, node.val);
	}
}

