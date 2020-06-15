#ifndef TH_H 
#define TH_H

#include "tommy.h"

/**********************
 * tablas hash
 **********************/

// necesario para tabla hash
typedef struct
{
        char *key;
        int val;

	// usado por tommy lib
	tommy_node node;

} t_th_node;

// estructura para tablas hash 
typedef struct
{
	// htab_t h; 
	t_th_node *tbl;
	size_t next_id;
	size_t max_sz;

	// tabla hash 
	tommy_hashtable h;

} t_th;

// creacion tabla hash
void th_init(t_th *th, size_t sz);

// borrado tabla hash
void th_free(t_th *th);

// reiniciamos tabla hash
void th_reset(t_th *th);

// buscamo elto 
int th_search(t_th th, const char *key);

// insertamos en tabla hash
void th_insert(t_th *th, const char *key, int val);

// devuelve nodo
t_th_node *th_node(t_th *th, const char *key);

// reconstruimos 
void th_rebuild(t_th *th);

#endif
