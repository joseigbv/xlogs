#include <stdio.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <regex.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "common.h"
#include "idx.h"
#include "res.h"

// acesso a indices y conjuntos
#define IDX(x)			idx_##x.txt[reg.x]

// estructura datos con logs
#define DAT_DEFN_CHK(x)		t_reg *x
#define DAT_INIT_CHK(x)		xmalloc(SZ_CHK * sizeof(t_reg))
#define DAT_FREE_CHK(x)		free(x)

// indices
#define IDX_DEFN(x)		t_idx idx_##x
#define IDX_LOAD(x)             idx_load(&idx_##x, FILE_##x)
#define IDX_INIT(x, sz)		idx_init(&idx_##x, sz)
#define IDX_FREE(x)		idx_free(&idx_##x)

// buffers texto
#define IDX_DEFN_CHK(x)		IDX_DEFN(x)
#define IDX_INIT_CHK(x)		idx_init(&idx_##x, SZ_CHK)
#define IDX_REST_CHK(x)		idx_reset(&idx_##x)
#define IDX_FREE_CHK(x)		IDX_FREE(x)

// varios
#define	ARGS(x)			(*(t_args *)args).x
#define MAX(x, y) 		(x > y ? x: y)
#define MIN(x, y) 		(x <= y ? x :y)
#define KB(x) 			(x / 1024)

// send chunks 
#define SEND_CHUNK(thread, chunk)		fread(share[thread].zbuf, file_dat.chunks[chunk].sz, 1, f); \
						share[thread].sz = file_dat.chunks[chunk].sz 

#define SEND_CHUNK_IDX(thread, chunk, idx)	fread(share[thread].zbuf_##idx, file_dat.chunks[chunk].sz_##idx, 1, f); \
						share[thread].sz_##idx = file_dat.chunks[chunk].sz_##idx

#define SEND_CHUNK_SETS(thread, chunk, sets)	SEND_CHUNK_IDX(thread, chunk, sets)

// recv chunks
#define RECV_CHUNK(regs) 			recv_chunk(share->zbuf, share->sz, regs)
#define RECV_CHUNK_IDX(idx)			recv_chunk_idx(share->zbuf_##idx, share->sz_##idx, &idx_##idx)
#define RECV_CHUNK_SETS(sets)			recv_chunk_sets(share->zbuf_##sets, share->sz_##sets, &sets_##sets)

// ignoramos buffers texto
#define	SKIP_SEND_CHUNK_IDX(thread, chunk, idx)		fseek(f, file_dat.chunks[chunk].sz_##idx, SEEK_CUR)
#define	SKIP_SEND_CHUNK_SETS(thread, chunk, sets)	SKIP_SEND_CHUNK_IDX(thread, chunk, sets)
#define	SKIP_RECV_CHUNK_IDX(idx)			usleep(1)
#define	SKIP_RECV_CHUNK_SETS(sets)			SKIP_RECV_CHUNK_IDX(sets)



/*************************
 * globales
 *************************/

// indices
IDX_DEFN(s_sitename);
IDX_DEFN(s_computername);
IDX_DEFN(cs_method);
IDX_DEFN(cs_username);
IDX_DEFN(cs_version);
IDX_DEFN(cs_user_agent);


// bloqueo (temporal)
pthread_mutex_t lck[MAX_THREADS];


// cargamos registros comprimidos
inline size_t recv_chunk(char *zbuf, size_t sz, t_reg *regs)
{
	// descomprimimos datos
	return uncompress_gz(zbuf, sz, 
		(char *) regs) / sizeof(t_reg);
}

// cargamos buffers de texto y configuramos indice
inline void recv_chunk_idx(char *zbuf, size_t sz, t_idx *idx)
{
	// descomprimimos 
	sz = uncompress_gz(zbuf, sz, idx->sbuf.txt);

	// configuramos buffer
	sbuf_set(&idx->sbuf, sz);

	// configuramos indice
	idx_set(idx); 
}


/*************************
 * threads
 *************************/

// datos entre threads
typedef struct 
{
	// "eof"
	int pending;

	// ya leido?
	int readed; 

	// num chunk 
	int chunk;

	// chunks regs
	char *zbuf;
	size_t sz;

	// chunks cs_uri
	char *zbuf_cs_uri_stem;
	size_t sz_cs_uri_stem;

	// chunks cs_query
	char *zbuf_cs_uri_query;
	size_t sz_cs_uri_query;

	// chunks cs_referer
	char *zbuf_cs_referer;
	size_t sz_cs_referer;

	// chunks sc_filter_category
	char *zbuf_cs_cokie;
	size_t sz_cs_cokie;

	// chunks r_host
	char *zbuf_cs_host;
	size_t sz_cs_host;

} t_share;

// argumentos thread
typedef struct
{
	// in
	int thread_id;	

	// in - out
	t_share *share; 
	t_res *res;

	// out
	u_long max;
	u_long min;
	u_long cnt;
	u_long sum;

} t_args;


// tarea a paralelizar
void *run(void *args)
{
	int num_regs, nr, chunk;
	int thread_id, id;
	u_long max, min, cnt, sum;
	t_reg *regs, reg;
	t_share *share;
	t_res *res;
	char sbuf[SZ_SBUF1];
	int id_filter, code;
	time_t interval, from, to;
	struct tm tm;

	// varchars
	IDX_DEFN_CHK(cs_uri_stem);
	IDX_DEFN_CHK(cs_uri_query);
	IDX_DEFN_CHK(cs_cokie);
	IDX_DEFN_CHK(cs_referer);
	IDX_DEFN_CHK(cs_host);

	// argumentos (in)
	thread_id = ARGS(thread_id);
	share = ARGS(share);
	res = ARGS(res);

	// argumentos (out)
	max = ARGS(max);
	min = ARGS(min);
	cnt = ARGS(cnt);
	sum = ARGS(sum);

	// inicializa buffers
	IDX_INIT_CHK(cs_uri_stem);
	IDX_INIT_CHK(cs_uri_query);
	IDX_INIT_CHK(cs_cokie);
	IDX_INIT_CHK(cs_referer);
	IDX_INIT_CHK(cs_host);

	// reservamos memoria registros
	regs = DAT_INIT_CHK();

	/********************************************************/
	/* definicion de filtros */
	/********************************************************/

	// definimos id filtro
	id_filter = 0;

	// desde timestamp 
	strptime("2012-11-16 02:00:00", "%Y-%m-%d %H:%M:%S", &tm);
	from = mktime(&tm);

	// hasta timestamp
	strptime("2012-11-16 04:00:00", "%Y-%m-%d %H:%M:%S", &tm);
	to = mktime(&tm);

/*
	int r;
	regex_t regex;
	char re = ""; 

        // compilamos regex
        if ((r = regcomp(&regex, re, REG_EXTENDED | REG_ICASE)))
        {
                regerror(r, &regex, sbuf, sizeof(sbuf));
                printf("ERROR: %s\n", sbuf);
        }
*/

	// leemos tabla chunks fichero
	while (share->pending)
	{
		// esperamos a que este lista
		if (share->readed) 
		{
			usleep(100);
			continue;
		}

		// leemos datos 
		pthread_mutex_lock(&lck[thread_id]);

			// carga datos
			num_regs = RECV_CHUNK(regs);
			
			// carga buffers texto
			RECV_CHUNK_IDX(cs_uri_stem);
			RECV_CHUNK_IDX(cs_uri_query);
			RECV_CHUNK_IDX(cs_cokie);
			RECV_CHUNK_IDX(cs_referer);
			RECV_CHUNK_IDX(cs_host);

			// num chunk
			chunk = share->chunk;

			// marcamos como leido
			share->readed = 1;

		pthread_mutex_unlock(&lck[thread_id]);

		// analiza cada registro 
		for(nr = 0; nr < num_regs; nr++)
		{
			// leemos registro
			reg = regs[nr]; 

			// id registro
			id = chunk *SZ_CHK + nr;

			// control de errores 
			if (reg.magic != 0x99) 
			{
				fprintf(stderr, "Error: registro no valido (%d)\n", id);
				exit(EXIT_FAILURE);
			}


			/***********************************************/
			/* aplicacion de filtros */
			/**********************************************/

			// filtro rango horario 
			if (reg.date_time < from || reg.date_time > to) continue;
			if ((reg.sc_status / 100) * 100 != 500) continue;

/*
			// filtro aqui
			if (regexec(&regex, sbuf, 0, NULL, 0) == 0)
			{
			{
*/

			// filtro
//			if (reg.c_ip == ip_int("186.42.29.139")) continue; 
//			if (strstr(IDX(cs_user_agent), "Havij") == NULL) continue; 
//			if (strstr(IDX(cs_user_agent), "Havij") == NULL) continue; 
//			if (IDX(cs_uri_stem) == ("/club/recursos/productos/")) continue;
//			if (reg.cs_method != idx_id(idx_cs_method, "ACUNETIX", 5)) continue;
//			if (reg.c_ip != ip_int("216.177.210.195")) continue;
//			if (reg.sc_status != 500) continue;
//			res_add(res, ip_str(reg.c_ip, sbuf), 1);
//			res_add(res, IDX(cs_uri_stem), 1);
//			res_add(res, sbuf, 1);
//			sprintf(sbuf, "%d", reg.sc_status);

			// grafico codes por hora
//			code = (reg.sc_status / 100) * 100;
//			interval = (reg.date_time / 3600) * 3600;
//			sprintf(sbuf, "%lu : %d", interval, code);
//			res_add(res, sbuf, 1);

			// lista urls
			ip_str(reg.c_ip, sbuf); 
//			printf("DEBUG: %s : %s?%s\n", sbuf, 
//				IDX(cs_uri_stem), IDX(cs_uri_query));

			res_add(res, sbuf, 1);

			// estadisticas
			min = 0;
			max = 0;
			sum += KB(reg.sc_bytes);

#ifdef DEBUG
			// salida debug
			printf("\n");
			printf("id: %d\n", id);
			printf("date_time: (%lu) %s", reg.date_time, ctime(&reg.date_time));
			printf("s_sitename: %d (%s)\n", reg.s_sitename, IDX(s_sitename));
			printf("s_computername: %d (%s)\n", reg.s_computername, IDX(s_computername));
			printf("s_ip: %u (%s)\n", reg.s_ip, ip_str(reg.s_ip, sbuf));
			printf("cs_method: %d (%s)\n", reg.cs_method, IDX(cs_method));
			printf("cs_uri_stem: %d (%s)\n", reg.cs_uri_stem, IDX(cs_uri_stem));
			printf("cs_uri_query: %d (%s)\n", reg.cs_uri_query, IDX(cs_uri_query));
			printf("s_port: %d\n", reg.s_port);
			printf("cs_username: %d (%s)\n", reg.cs_username, IDX(cs_username));
			printf("c_ip: %u (%s)\n", reg.c_ip, ip_str(reg.c_ip, sbuf));
			printf("cs_version: %d (%s)\n", reg.cs_version, IDX(cs_version));
			printf("cs_user_agent: %d (%s)\n", reg.cs_user_agent, IDX(cs_user_agent));
			printf("cs_cokie: %d (%s)\n", reg.cs_cokie, IDX(cs_cokie));
			printf("cs_referer: %d (%s)\n", reg.cs_referer, IDX(cs_referer));
			printf("cs_host: %d (%s)\n", reg.cs_host, IDX(cs_host));
			printf("sc_status: %d\n", reg.sc_status);
			printf("sc_substatus: %d\n", reg.sc_substatus);
			printf("sc_win32_status: %d\n", reg.sc_win32_status);
			printf("sc_bytes: %u\n", reg.sc_bytes);
			printf("cs_bytes: %u\n", reg.cs_bytes);
			printf("time_taken: %d\n", reg.time_taken);
			printf("\n");
#endif
			// estadisticas
			cnt++;
		}

		// reducimos resultados
		res_reduce(res);

		// limpiamos buffers
		IDX_REST_CHK(cs_uri_stem);
		IDX_REST_CHK(cs_uri_query);
		IDX_REST_CHK(cs_cokie);
		IDX_REST_CHK(cs_referer);
		IDX_REST_CHK(cs_host);
	}

	// libera memoria buffers
	DAT_FREE_CHK(regs);

	// libera buffers texto
	IDX_FREE_CHK(cs_uri_stem);
	IDX_FREE_CHK(cs_uri_query);
	IDX_FREE_CHK(cs_cokie);
	IDX_FREE_CHK(cs_referer);
	IDX_FREE_CHK(cs_host);

/*
        // liberamos regex
        regfree(&regex);
*/

	// restaura argumentos
	ARGS(max) = max;
	ARGS(min) = min;
	ARGS(cnt) = cnt;
	ARGS(sum) = sum;

	// necesario en thread
	return NULL;
}


/*************************
 * main
 *************************/


int main()
{
	FILE *f;
	int err, chunk, thread;
	int file_id, pending;
	char fname[SZ_PATH];
	time_t start, stop;
	t_file_dat file_dat;
	t_res res;

	// arrancamos cronometro
	time(&start);

	// problemas de pila ?
	t_args args[MAX_THREADS];
	pthread_t threads[MAX_THREADS];
	t_share share[MAX_THREADS];
	t_res res_chk[MAX_THREADS];

	// contadores
	u_long max = 0;
	u_long min = 0;
	u_long cnt = 0;
	u_long sum = 0;

        // banner
        printf("\nAnalisis de logs 'IIS'\n");
        printf("Utilidad para consultas de datos\n\n");

	printf("Ejecutando query...\n\n");

	// incializa resultados 
	res_init(&res, SZ_CHK * 0.2 * MAX_THREADS);

	// inicializa indices 
	IDX_INIT(s_sitename, 256);
	IDX_INIT(s_computername, 256);
	IDX_INIT(cs_method, 256);
	IDX_INIT(cs_username, 65536);
	IDX_INIT(cs_version, 256);
	IDX_INIT(cs_user_agent, 65536);

	// carga indices
	IDX_LOAD(s_sitename);
	IDX_LOAD(s_computername);
	IDX_LOAD(cs_method);
	IDX_LOAD(cs_username);
	IDX_LOAD(cs_version);
	IDX_LOAD(cs_user_agent);

	// prepara argumentos
	for (thread = 0; thread < MAX_THREADS; thread++)
	{
		// inicializamos resultados parciales
		res_init(&res_chk[thread], SZ_CHK * 1.1);

		// reservamos memoria para descompresion
		share[thread].zbuf = xmalloc(SZ_ZBUF);
		share[thread].zbuf_cs_uri_stem = xmalloc(SZ_ZBUF);
		share[thread].zbuf_cs_uri_query = xmalloc(SZ_ZBUF);
		share[thread].zbuf_cs_cokie = xmalloc(SZ_ZBUF);
		share[thread].zbuf_cs_referer = xmalloc(SZ_ZBUF);
		share[thread].zbuf_cs_host = xmalloc(SZ_ZBUF);

		// estado inicial
		share[thread].pending = 1;
		share[thread].readed = 1;

		// argumentos (in)
		args[thread].thread_id = thread;
		args[thread].share = &share[thread];
		args[thread].res = &res_chk[thread];

		// argumentos (out)
		args[thread].max = max;
		args[thread].min = min;
		args[thread].cnt = cnt;
		args[thread].sum = sum;

		printf("Lanzando tarea %d...\n", thread);

		// ejecuta thread
		if ((err = pthread_create(&threads[thread], NULL, run, &args[thread])))
		{
			fprintf(stderr, "Error de creacion de hilo\n");
			exit(EXIT_FAILURE);
		}
	}

	printf("\n");

	// leemos cada fichero  y procesamos
	for(file_id = 0; file_id < NUM_FILES; file_id++)
	{
		// calculamos nombre fichero
		sprintf(fname, FILE_dat, FILES[file_id]);
		printf("Procesando '%s'...\n", fname);

		// abrimos fichero
		f = xfopen(fname, "rb");

		// leemos tabla chunks
		fread(&file_dat, sizeof(t_file_dat), 1, f);

		// procesamos fichero .dat (cada chunk a una tarea)
		for (chunk = 0, thread = 0; chunk < file_dat.num_chunks;
			chunk++, thread = (thread + 1) % MAX_THREADS)
		{
			// buscamos thread libre
			while (! share[thread].readed) 
			{
				thread = (thread + 1) % MAX_THREADS;
				usleep(100);
			}

			// ahora enviamos datos
			pthread_mutex_lock(&lck[thread]);

			// marcamos como pendiente
			share[thread].readed = 0;

			// enviamos chunk
			share[thread].chunk = chunk;

			// leemos chunk datos y enviamos a thread
			SEND_CHUNK(thread, chunk);
			SEND_CHUNK_IDX(thread, chunk, cs_uri_stem);
			SEND_CHUNK_IDX(thread, chunk, cs_uri_query);
			SEND_CHUNK_IDX(thread, chunk, cs_cokie);
			SEND_CHUNK_IDX(thread, chunk, cs_referer);
			SEND_CHUNK_IDX(thread, chunk, cs_host);

			pthread_mutex_unlock(&lck[thread]);
		}

		// cerramos fichero
		fclose(f);
	}

	// espera a que termine
	for (pending = MAX_THREADS; pending;)
	{
		// siguiente tarea
		thread = (thread + 1) % MAX_THREADS;

		// primero esperamos hasta leido
		if (! share[thread].readed) 
		{
			usleep(100);
			continue;
		}

		// cerramos ?
		if (share[thread].pending)
		{
			// ahora marcamos como eof
			share[thread].pending = 0;

			// liberamos memoria
			free(share[thread].zbuf);
			free(share[thread].zbuf_cs_uri_stem);
			free(share[thread].zbuf_cs_uri_query);
			free(share[thread].zbuf_cs_cokie);
			free(share[thread].zbuf_cs_referer);
			free(share[thread].zbuf_cs_host);

			// ha terminado bien?
			if ((err = pthread_join(threads[thread], NULL)))
			{
				fprintf(stderr, "Error de cierre de hilo\n");
				exit(EXIT_FAILURE);
			}

			// decrementamos tareas pendientes
			pending--;
		}
	}

	// consolidamos resultados 

	max = args[0].max;
	min = args[0].min;
	cnt = args[0].cnt;
	sum = args[0].sum;

	res_merge(*args[0].res, &res);
	res_free(args[0].res);

	for (thread = 1; thread < MAX_THREADS; thread++)
	{
		max = max > args[thread].max ? args[thread - 1].max : args[thread].max;
		min = min < args[thread].min ? args[thread - 1].min : args[thread].min;
		cnt = cnt + args[thread].cnt;
		sum = sum + args[thread].sum;

		res_merge(*args[thread].res, &res);
		res_free(args[thread].res);
	}
	
	// ordena y reduce
	res_reduce(&res);

	printf("\n\n* Resultados:\n\n");

	// muestra resultados ordenados
	res_show(res, 25);

	// muestra estadisticas 
	printf("\n\n* Estadisticas:\n\n");
	printf("- Maximo: %lu\n", max);
	printf("- Minimo: %lu\n", min);
	printf("- Media: %lu\n", sum / (cnt? cnt: 1));
	printf("- Total: %lu\n", sum);
	printf("- Registros: %lu\n", cnt);

	// borramos arbol resultados	
	res_free(&res);

	// libera indices
        IDX_FREE(s_sitename);
        IDX_FREE(s_computername);
        IDX_FREE(cs_method);
        IDX_FREE(cs_username);
        IDX_FREE(cs_version);
        IDX_FREE(cs_user_agent);

	// arrancamos cronometro
	time(&stop);

	printf("\n(Tiempo: %lu segs.)\n\n", 
		stop - start);

	exit(EXIT_SUCCESS);
}
