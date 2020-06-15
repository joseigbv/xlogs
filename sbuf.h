#ifndef SBUF_H
#define SBUF_H

// para tratamiento vartext
typedef struct
{
        char *txt;		// puntero con texto
        size_t sz;		// tamanio actual string
        size_t max_sz;		// maximo regs perm
	char *zbuf;		// usado por compresion

} t_sbuf;

// inicializa estructuras
void sbuf_init(t_sbuf *sbuf, size_t max_sz);

// reinicializa estructuras
void sbuf_reset(t_sbuf *sbuf);

// elimina estructuras
void sbuf_free(t_sbuf *sbuf);

// carga strings a memoria
void sbuf_load(t_sbuf *sbuf, const char *fname);

// salva buffer con strincs comprimidos a fichero
void sbuf_save(t_sbuf sbuf, const char *fname);

// offset a la siguiente cadena comprimida
int sbuf_offset(t_sbuf *sbuf, const char *txt, size_t sz);

// aux config sbuf
void sbuf_set(t_sbuf *sbuf, size_t sz);

#endif
