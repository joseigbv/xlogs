#include <stdio.h>
#include <string.h>

#include "th.h"
#include "tommy.h"
#include "tommyhashtbl.h"
#include "common.h"


//#define hash(x) djb_hash((void *)x, strlen(x))
//#define hash(x) fnv1_hash(x)
#define hash(x) tommy_hash_u32(0x99, x, strlen(x))
//#define hash(x) tommy_hash_u64(0x99, x, strlen(x))

// creacion tabla hash
void th_init(t_th *th, size_t max_sz)
{
	// inicializamos valores
	th->next_id = 0;
	th->max_sz = max_sz;

	// inicializamos tabla hash 
        tommy_hashtable_init(&th->h, th->max_sz * 1.25);

	// reservamos memoria para tabla nodos
	th->tbl = xmalloc(max_sz * sizeof(t_th_node));
}

// borrado tabla hash
void th_free(t_th *th)
{
	// liberamos tabla hashes
	tommy_hashtable_done(&th->h);

	// liberamos tabla nodos
	free(th->tbl);
}

// aux reset
inline void tommy_hashtable_reset(t_th *th)
{
	tommy_hashtable_done(&th->h);
        tommy_hashtable_init(&th->h, th->max_sz * 1.25);
}

// reiniciamos tabla hash
void th_reset(t_th *th)
{
	// limpiamos tabla hashes
	tommy_hashtable_reset(th);

	// reiniciamoso valores
	th->next_id = 0; 
}

// funcion comparacion aux
inline int compare(const void *key, const void *node)
{
        return strcmp((char *)key, 
		((t_th_node *)node)->key);
}

// buscamo elto 
int th_search(t_th th, const char *key)
{
	t_th_node *pn;

	// buscamos
	pn = tommy_hashtable_search(&th.h, 
		compare, key, hash(key));

	// enconrado ? no, devuelve -1
	return pn ? pn->val : -1;
}

// insertamos en tabla hash
void th_insert(t_th *th, const char *key, int val)
{
	t_th_node *pn;

        // buscamos nodo o creamos
	pn = th_node(th, key);
	pn->val = val;
}

// devuelve nodo
t_th_node *th_node(t_th *th, const char *key)
{
	t_th_node *pn;

	// control de errores
	if (th->next_id > th->max_sz)
	{
		printf("DEBUG: %s - %lu, - %lu\n", key, th->next_id, th->max_sz);
		fprintf(stderr, "Error: superado el numero maximo de hashes permitido\n");
		exit(EXIT_FAILURE);
	}

	// existe ya ?
	if ((pn = tommy_hashtable_search(&th->h,
		compare, key, hash(key))) == NULL)
	{
		// asignamos nodo
		pn = &th->tbl[th->next_id++];

		// inicializamos nodo
		pn->key = (char *) key;
		pn->val = 0;

		// insertamos en tabla hash
		tommy_hashtable_insert(&th->h, &pn->node,
			pn, hash(key));
	}

	return pn;
}

// reconstruye hash
void th_rebuild(t_th *th)
{
	int i; 
	t_th_node *pn;

	// primero limpiamos 
	tommy_hashtable_reset(th);

	// ahora reconstruimos hash
	for (i = 0; i < th->next_id; i++)
	{
		// asignamos nodo
		pn = &th->tbl[i];
	
		// registramos en tabla hash
		tommy_hashtable_insert(&th->h, &pn->node,
			pn, hash(pn->key));
	}	
}
