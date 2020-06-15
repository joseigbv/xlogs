/* 

#Fields: date time s-sitename s-computername s-ip cs-method cs-uri-stem cs-uri-query s-port cs-username c-ip cs-version cs(User-Agent) cs(Cookie) cs(Referer) cs-host sc-status sc-substatus sc-win32-status sc-bytes cs-bytes time-taken

Ej.

2012-06-20 03:38:17 W3SVC3 P2396083 216.157.75.24 GET / - 80 - 66.223.57.120 HTTP/1.1 Mozilla/5.0+(Windows+NT+5.1)+AppleWebKit/536.5+(KHTML,+like+Gecko)+Chrome/19.0.1084.56+Safari/536.5 - - 216.157.75.24 200 0 0 426 366 1279
2012-06-20 03:38:17 W3SVC3 P2396083 216.157.75.24 GET /favicon.ico - 80 - 66.223.57.120 HTTP/1.1 Mozilla/5.0+(Windows+NT+5.1)+AppleWebKit/536.5+(KHTML,+like+Gecko)+Chrome/19.0.1084. 56+Safari/536.5 - - 216.157.75.24 404 0 2 1405 317 15

*/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <regex.h>
#include <ctype.h>
#include <unistd.h>

#include "common.h"
#include "idx.h"

#define OFFSET_date_time 0
#define OFFSET_s_sitename 1
#define OFFSET_s_computername 2
#define OFFSET_s_ip 3
#define OFFSET_cs_method 4
#define OFFSET_cs_uri_stem 5
#define OFFSET_cs_uri_query 6
#define OFFSET_s_port 7 
#define OFFSET_cs_username 8 
#define OFFSET_c_ip 9
#define OFFSET_cs_version 10
#define OFFSET_cs_user_agent 11
#define OFFSET_cs_cokie 12
#define OFFSET_cs_referer 13
#define OFFSET_cs_host 14
#define OFFSET_sc_status 15
#define OFFSET_sc_substatus 16
#define OFFSET_sc_win32_status 17
#define OFFSET_sc_bytes 18
#define OFFSET_cs_bytes 19
#define OFFSET_time_taken 20

#define SZ_FIELDS 			21

#define LOG(x)				log[OFFSET_##x]
#define SZ(x)				sz[OFFSET_##x]

#define LOWER(x)			strlow(LOG(x))
#define IP(x) 				ip_int(LOG(x))
#define INT(x) 				atoi(LOG(x))

#define IDX(x)				idx_insert(&idx_##x, LOG(x), SZ(x))

#define IDX_DEFN(x)			t_idx idx_##x
#define IDX_INIT(x, sz)			idx_init(&idx_##x, sz)
#define IDX_DEFL(x, y)			idx_default(&idx_##x, y)
#define IDX_FREE(x)			idx_free(&idx_##x)
#define IDX_LOAD(x)			idx_load(&idx_##x, FILE_##x)
#define IDX_SAVE(x)			idx_save(idx_##x, FILE_##x)

#define IDX_DEFN_CHK(x)			IDX_DEFN(x)
#define IDX_INIT_CHK(x)			idx_init(&idx_##x, SZ_CHK)
#define IDX_REST_CHK(x)			idx_reset(&idx_##x)
#define IDX_FREE_CHK(x)			IDX_FREE(x)
#define IDX_SAVE_CHK(x)			file_dat.chunks[chunk].sz_##x = \
						save_chunk(f, zbuf, idx_##x.sbuf.txt, idx_##x.sbuf.sz)


/*************************
 * funciones aux
 *************************/

// cargamos chunk de logs
size_t save_chunk(FILE *f, char *zbuf, void *sbuf, size_t sz)
{
	size_t sz_z;
	
	// comprimimos
	sz_z = compress_gz(sbuf, sz, zbuf);

	// salvamos y devolvemos sz_z
	return fwrite(zbuf, 1, sz_z, f);
}


/*************************
 * main 
 *************************/

int main(int argc, char *argv[])
{
	struct tm tm;
	t_reg *reg, *regs;
	int i, r, chunk = 0;
	int cnt = 0, id = 0;
	char sbuf[SZ_SBUF1];
	char file_id[SZ_FILE_ID];
	regex_t regex;
	regmatch_t match[SZ_FIELDS + 1];
	char *log[SZ_FIELDS];
	size_t sz[SZ_FIELDS];
	time_t start, stop;
	char *zbuf;
	char fname[SZ_PATH];
	t_file_dat file_dat;
	FILE *f;

	// 2012-06-20 03:38:17 W3SVC3 P2396083 216.157.75.24 GET / - 80 - 66.223.57.120 HTTP/1.1 Mozilla/5.0+(Win... - - 216.157.75.24 200 0 0 426 366 1279

	// expresion regular parser
	char *re_parse =

		// date_time: 2012-06-20 03:38:17
		"^([[:digit:]]{4}\\-[[:digit:]]{2}\\-[[:digit:]]{2} [[:digit:]]{2}:[[:digit:]]{2}:[[:digit:]]{2}) "\

		// s_sitename: W3SVC3
		"([[:alnum:]]+) "\

		// s_computername: P2396083
		"([[:alnum:]]+) "\
	
		// s_ip: 216.157.75.24
		"([[:digit:]]+\\.[[:digit:]]+\\.[[:digit:]]+\\.[[:digit:]]+) "\

		// cs_method: GET
		"([[:alpha:]]+) "\

		// cs_uri_stem: /
		"([^ ]+) "\

		// cs_uri_query: - 
		"([^ ]+) "\

		// s_port: 80 
		"([[:digit:]]+) "\

		// cs_username: - 
		"([^ ]+) "\

		// c_ip: 66.223.57.120
		"([[:digit:]]+\\.[[:digit:]]+\\.[[:digit:]]+\\.[[:digit:]]+) "\

		// cs_version: HTTP/1.1
		"([^ ]+) "\
		
		// cs_user_agent: Mozilla/5.0+(Windows+NT+5.1)+AppleWebKit/536.5+(KHTML,+like+Gecko)+Chrome/19.0.1084.56+Safari/536.5
		"([^ ]+) "\

		// cs_cokie: - 
		"([^ ]+) "\

		// cs_referer: -
		"([^ ]+) "\

		// cs_host: 216.157.75.24 
		"([^ ]+) "\

		// sc_status: 200
		"([[:digit:]]{3}) "\
		
		// sc_substatus: 0
		"([[:digit:]]+) "\

		// sc_win32_status: 0 
		"([[:digit:]]+) "\
		
		// sc_bytes: 426
		"([[:digit:]]+) "\

		// cs_bytes: 366 
		"([[:digit:]]+) "\

		// time_taken: 1279
		"([[:digit:]]+)\r\n$";


	// indices para norm 
	IDX_DEFN(s_sitename);
	IDX_DEFN(s_computername);
	IDX_DEFN(cs_method);
	IDX_DEFN(cs_username);
	IDX_DEFN(cs_version);
	IDX_DEFN(cs_user_agent);

	// varchars
	IDX_DEFN_CHK(cs_uri_stem);
	IDX_DEFN_CHK(cs_uri_query);
	IDX_DEFN_CHK(cs_cokie);
	IDX_DEFN_CHK(cs_referer);
	IDX_DEFN_CHK(cs_host);

	// iniciamos cronometro
	time(&start);

	// banner
	printf("\nAnalisis de logs 'IIS'\n");
	printf("Utilidad para normalizacion de la informacion\n\n");

	// parametros de entrada
	if (argc != 2) 
	{
		fprintf(stderr, "Usa: %s <file_id>\n\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	// captura file id
	strncpy(file_id, argv[1], SZ_FILE_ID);

	// inicializamos fichero
	sprintf(fname, FILE_dat, file_id);
	f = xfopen(fname, "wb");

	printf("Generando fichero '%s'\n", fname);

	// control errores
	file_dat.magic = 0x99;

	// reservamos espacio en disco
	fwrite(&file_dat, sizeof(t_file_dat), 1, f);

	// compilamos regex
	if ((r = regcomp(&regex, re_parse, REG_EXTENDED | REG_ICASE)))
	{
		regerror(r, &regex, sbuf, sizeof(sbuf));
		printf("ERROR: %s\n", sbuf);
	}

	// buffer aux compresion
	zbuf = xmalloc(SZ_CHK * SZ_SBUF2);

	// array de registros	
	regs = xmalloc(SZ_CHK * sizeof(t_reg));

	// inicializamos indices 
	IDX_INIT(s_sitename, 256);
	IDX_INIT(s_computername, 256);
	IDX_INIT(cs_method, 256);
	IDX_INIT(cs_username, 65536);
	IDX_INIT(cs_version, 256);
	IDX_INIT(cs_user_agent, 65536);

	// varchars
	IDX_INIT_CHK(cs_uri_stem);
	IDX_INIT_CHK(cs_uri_query);
	IDX_INIT_CHK(cs_cokie);
	IDX_INIT_CHK(cs_referer);
	IDX_INIT_CHK(cs_host);

	// existen indices previos ?
	if (access(FILE_cs_username, F_OK) == 0)
	{
		printf("Cargando indices...\n");

		// cargamos
		IDX_LOAD(s_sitename);
		IDX_LOAD(s_computername);
		IDX_LOAD(cs_method);
		IDX_LOAD(cs_username);
		IDX_LOAD(cs_version);
		IDX_LOAD(cs_user_agent);
	}
	else
	{
		// por defecto desconocido 
		IDX_DEFL(s_sitename, UNKNOWN);
		IDX_DEFL(s_computername, UNKNOWN);
		IDX_DEFL(cs_method, UNKNOWN);
		IDX_DEFL(cs_username, UNKNOWN);
		IDX_DEFL(cs_version, UNKNOWN);
		IDX_DEFL(cs_user_agent, UNKNOWN);
	}

        printf("Procesando entrada estandar...\n\n");

	// primer registro
	chunk = 0; 
	id = 0;

	// logs por entrada estandar
	while (!feof(stdin))
	{
		// leemos cada linea
		if (fgets(sbuf, SZ_SBUF1, stdin))
		{
			// ejecutamos expresion regular
			if (regexec(&regex, sbuf, SZ_FIELDS + 1, match, 0) == 0)
			{
				// siguiente registro
				reg = &regs[id]; 

				// generamos campos ...
				for (i = 0; i < SZ_FIELDS; i++)
				{
					log[i] = sbuf + match[i + 1].rm_so;
					sbuf[match[i + 1].rm_eo] = 0;
					sz[i] = match[i + 1].rm_eo - match[i + 1].rm_so + 1;
				}

				// calculo fecha + hora
                                strptime(LOG(date_time), "%Y-%m-%d %H:%M:%S", &tm);

				// creacion registro
				reg->magic = 0x99;
				reg->date_time = mktime(&tm);
				reg->s_sitename = IDX(s_sitename);
				reg->s_computername = IDX(s_computername);
				reg->s_ip = IP(s_ip);
				reg->cs_method = IDX(cs_method);
				reg->cs_uri_stem = IDX(cs_uri_stem);
				reg->cs_uri_query = IDX(cs_uri_query);
				reg->s_port = INT(s_port);
				reg->cs_username = IDX(cs_username);
				reg->c_ip = IP(c_ip);
				reg->cs_version = IDX(cs_version);
				reg->cs_user_agent = IDX(cs_user_agent);
				reg->cs_cokie = IDX(cs_cokie);
				reg->cs_referer = IDX(cs_referer);
				reg->cs_host = IDX(cs_host);
				reg->sc_status = INT(sc_status);
				reg->sc_substatus = INT(sc_substatus);
				reg->sc_win32_status = INT(sc_win32_status);
				reg->sc_bytes = INT(sc_bytes);
				reg->cs_bytes = INT(cs_bytes);
				reg->time_taken = INT(time_taken);
#ifdef DEBUG
				// debug 
				printf("\n");
				printf("id: %d\n", id);
				printf("date_time: %lu (%s)\n", reg->date_time, LOG(date_time));
				printf("s_sitename: %d (%s)\n", IDX(s_sitename), LOG(s_sitename));
				printf("s_computername: %d (%s)\n", IDX(s_computername), LOG(s_computername));
				printf("s_ip: %u (%s)\n", reg->s_ip, LOG(s_ip));
				printf("cs_method: %d (%s)\n", IDX(cs_method), LOG(cs_method));
				printf("cs_uri_stem: %d (%s)\n", IDX(cs_uri_stem), LOG(cs_uri_stem));
				printf("cs_uri_query: %d (%s)\n", id, LOG(cs_uri_query));
				printf("s_port: %d\n", reg->s_port);
				printf("cs_username: %d (%s)\n", reg->cs_username, LOG(cs_username));
				printf("c_ip: %u (%s)\n", reg->c_ip, LOG(c_ip));
				printf("cs_version: %d (%s)\n", reg->cs_version, LOG(cs_version));
				printf("cs_user_agent: %d (%s)\n", IDX(cs_user_agent), LOG(cs_user_agent));
				printf("cs_cokie: %d (%s)\n", reg->cs_cokie, LOG(cs_cokie));
				printf("cs_referer: %d (%s)\n", reg->cs_referer, LOG(cs_referer));
				printf("cs_host: %d (%s)\n", reg->cs_host, LOG(cs_host));
				printf("sc_status: %d\n", reg->sc_status);
				printf("sc_substatus: %d\n", reg->sc_substatus);
				printf("sc_win32_status: %d\n", reg->sc_win32_status);
				printf("sc_bytes: %d\n", reg->sc_bytes);
				printf("cs_bytes: %d\n", reg->cs_bytes);
				printf("time_taken: %d\n", reg->time_taken);
				printf("\n");
#endif
				// hemos alcanzado final chunk ?
				if (++id == SZ_CHK)
				{
					printf("Salvando chunk '%d'...\n", chunk);

					// creamos nuevo fichero y guardamos registros
					file_dat.chunks[chunk].sz = 
						save_chunk(f, zbuf, regs, SZ_CHK * sizeof(t_reg));

					// salvamos buffers
					IDX_SAVE_CHK(cs_uri_stem);
					IDX_SAVE_CHK(cs_uri_query);
					IDX_SAVE_CHK(cs_cokie);
					IDX_SAVE_CHK(cs_referer);
					IDX_SAVE_CHK(cs_host);

					// limpiamos buffers
					IDX_REST_CHK(cs_uri_stem);
					IDX_REST_CHK(cs_uri_query);
					IDX_REST_CHK(cs_cokie);
					IDX_REST_CHK(cs_referer);
					IDX_REST_CHK(cs_host);

					// siguiente chunk
					id = 0; 
					chunk++; 
				}
			}

			else fprintf(stderr, "%s", sbuf);

			// estadisticas
			cnt++;
		}
	}

	// pendiente salvar ?
	if (id)
	{
		printf("Salvando chunk '%d'...\n", chunk);

		// salvamos datos
		file_dat.chunks[chunk].sz = 
			save_chunk(f, zbuf, regs, id * sizeof(t_reg));

		// salvamos buffers
		IDX_SAVE_CHK(cs_uri_stem);
		IDX_SAVE_CHK(cs_uri_query);
		IDX_SAVE_CHK(cs_cokie);
		IDX_SAVE_CHK(cs_referer);
		IDX_SAVE_CHK(cs_host);
	}

	// control numero ficheros
	file_dat.num_chunks = ++chunk;

	// volvemos al principio
	rewind(f);

	// y escribimos tabla de chunks
	fwrite(&file_dat, sizeof(t_file_dat), 1, f);

	// cerramos fichero
	fclose(f);

	// salvamos indices
	IDX_SAVE(s_sitename);
	IDX_SAVE(s_computername);
	IDX_SAVE(cs_method);
	IDX_SAVE(cs_username);
	IDX_SAVE(cs_version);
	IDX_SAVE(cs_user_agent);

	// liberamos indices
	IDX_FREE(s_sitename);
	IDX_FREE(s_computername);
	IDX_FREE(cs_method);
	IDX_FREE(cs_username);
	IDX_FREE(cs_version);
	IDX_FREE(cs_user_agent);

	// liberamos buffers
	IDX_FREE_CHK(cs_uri_stem);
	IDX_FREE_CHK(cs_uri_query);
	IDX_FREE_CHK(cs_cokie);
	IDX_FREE_CHK(cs_referer);
	IDX_FREE_CHK(cs_host);

        // liberamos regex
        regfree(&regex);

	// liberamos memoria array regs
	free(regs);

	// liberamos memoria zbuf
	free(zbuf);

        printf("\n\n* Estadisticas:\n\n");
        printf("- Registros: %d\n", cnt);

	// arrancamos cronometro
	time(&stop);

        printf("\n(Tiempo: %lu segs.)\n\n", 
		stop - start);

	exit(EXIT_SUCCESS);
}

