/*
    Compilacion:
    Windows/Linux: gcc AFD.c -o AFD
    
    Ejecucion:
    ./AFD <NombredelArchivo>.txt

    Ejemplo de txt:
    4 2          -> 4 estados 2 Simbolos
    a b          -> Alfabeto
    2            -> Numero de Estados fonales
    0 2          -> Posiciones d estados finales 
    0 a 2 0 1    -> 0 Posicion del estado a Simbolo donde se encuentra 2 Posibles estados a los cuales ir 0 y 1
    0 b 1 0
    1 b 1 2

*/

#include <stdio.h>
#include <stdlib.h>

size_t cestados;
size_t csimbolos;
size_t **estados;
char *simbolos;
size_t *finales;

size_t cfinales;
size_t **tabla;
size_t *conteo;

#define MAX_ESTADOS_AFD 256
#define NoHay MAX_ESTADOS_AFD

typedef struct{
    size_t estado;
    char simbolo;
} Paso;

void EClausura(size_t *conj, size_t *n);
size_t pAlfabeto(char c);

/*
    Carga el archivo 
    Lee las dimensiones del AFN
    Lee el alfabeto, edos Finales y crea la tabla de transiciones
    Lee las transiciones 
*/
void cAFN(const char *nombre_archivo){
    FILE *f = fopen(nombre_archivo, "r");
    if(!f){
        fprintf(stderr, "No se encuentra el archivo", nombre_archivo);
        exit(1);
    }

    fscanf(f, "%zu %zu", &cestados, &csimbolos);
    
    simbolos = malloc((csimbolos + 1) *sizeof(char));

    for(size_t i = 0; i < csimbolos; i++){
        char buf[8];
        fscanf(f, "%s", buf);
        simbolos[i] = buf[0];
    }
    simbolos[csimbolos] = '\0';

    fscanf(f, "%zu", &cfinales);
    
    finales = malloc(cfinales *sizeof(size_t));
    for(size_t i = 0; i < cfinales; i++)
    fscanf(f, "%zu", &finales[i]);

    size_t total = cestados *(csimbolos + 1);
    tabla = calloc(total, sizeof(size_t*));
    conteo = calloc(total, sizeof(size_t));

    size_t estado, NoEsDest;
    char buf[8];
    while(fscanf(f, "%zu %s %zu", &estado, buf, &NoEsDest) == 3){
        char sim = (buf[0] == 'e') ? '\0' : buf[0];
        size_t sym = pAlfabeto(sim);

        if(sym == (size_t) - 1){
            fprintf(stderr, "Simbolo desconocido '%c' en el archivo \n", buf[0]);
            exit(1);
        }
        size_t index = estado *(csimbolos + 1) + sym;
        tabla[index] = malloc(NoEsDest *sizeof(size_t));
        conteo[index] = NoEsDest;
        for(size_t i = 0; i < NoEsDest; i++)
        fscanf(f, "%zu", &tabla[index][i]); 
    }
    fclose(f);
}

//Libera memoria del AFN
void fAFN(void){
    size_t total = cestados *(csimbolos + 1);
    for(size_t i = 0; i < total; i++)
    if(tabla[i]) 
    free(tabla[i]);
    free(tabla);
    free(conteo);
    free(simbolos);
    free(finales);
}

/*
    Pasa el caracter a un indice dentro del arreglo "simbolos".
    Si el caracter es '\0' devuelve el ultimo simbolo 
*/
size_t pAlfabeto(char c){
    if(c == '\0')
    return csimbolos;

    for(size_t i = 0; i < csimbolos; i++)
    if(simbolos[i] == c)
        return i;
    return(size_t) - 1;
}

/*
    Verificacion de un estado de aceptacion 
    recoore el arreglo finales comparando cada elemento con estado,
    si este encuentra una coincidencia devuelve i sin seguir buscando 
*/
int edoFinal(size_t estado){
    for(size_t i = 0; i < cfinales; i++)
    if(estado == finales[i])
        return 1;
    return 0;
}

/*
    Recibe un conjunto deestados de tamanio *n y lo expande agregando lo estados 
    alcanzables por transiciones e.
    Usa una pila para ir porcesando cada estado pendiente por cada estado 
    que saca, busca su columna e en la tabla y por cada destino que encentra ahi
    agrega a conj y a la pila si no estaba. 
*/
void EClausura(size_t *conj, size_t *n){
    size_t pila[cestados];
    size_t top = 0;

    for(size_t i = 0; i < *n; i++)
    pila[top++] = conj[i];
    
    while (top > 0){
        size_t e = pila[--top];
        size_t index = e *(csimbolos + 1) + csimbolos;

        for(size_t i = 0; i < conteo[index]; i++){
            size_t destino = tabla[index][i];

            int encontrado = 0;
            for(size_t j = 0; j < *n; j++)
            if(conj[j] == destino){
                encontrado = 1; 
                break;
            } 
            if(!encontrado){
                conj[(*n)++] = destino;
                pila[top++] = destino;
            }
        }
    }
}

/*
    Recorre la cadena e entrada c por c validando cada uno con pAlfabeto. 
    Los caracteres validos se van reescribiendo al inicio de la misma adena 
    y los ivalidos se reportan en stderr 
*/
int manejoError(char *cadena, size_t *largo){
    int error = 0;
    size_t newlargo = 0;

    for(size_t i = 0; i < *largo; i++){
        if(pAlfabeto(cadena[i]) != (size_t) - 1){
            cadena[newlargo++] = cadena[i];
        } else {
            fprintf(stderr, "'%c' invalido \n", cadena[i]);
            error = 1;
        }
    }

    cadena[newlargo] = '\0';
    *largo = newlargo;
    return error;
}

/*
    Recorre todos los caminos posibles para el AFN sobre la cadea
    imprimiendo los caminos que dan TRUE
    Explora todas las transiciones disponibles desde estado. 
    Cuando pos llega al final de la cadena, verifica si es true o false
    si es true imprime el camino recorrido 
*/
void AFNRecursivo(const char *cadena, size_t largo, size_t pos, size_t estado, Paso *camino, size_t npasos, int *eps_visitados, int *encontrado){
    if(!eps_visitados[estado]){
        eps_visitados[estado] = 1;

        size_t index_eps = estado *(csimbolos + 1) + csimbolos;
        for(size_t i = 0; i < conteo[index_eps]; i++){
            size_t destino = tabla[index_eps][i];

            camino[npasos] = (Paso){
                estado, '\0'
            };
            AFNRecursivo(cadena, largo, pos, destino, camino, npasos + 1, eps_visitados, encontrado);
        }
    }

    if(pos == largo){
        if(edoFinal(estado)){
            *encontrado = 1;

            printf(" q%zu", camino[0].estado);
            for(size_t i = 1; i < npasos; i++){
                if(camino[i].simbolo == '\0')
                    printf(" -- e --> q%zu", camino[i].estado);
                else    
                    printf(" -- %c --> q%zu", camino[i].simbolo, camino[i].estado);
            }

            if(npasos > 0){
                Paso ultimo = camino[npasos - 1];
                if(ultimo.simbolo == '\0')
                    printf(" -- e --> q%zu \n", estado);
                else
                    printf("-- %c --> q%zu \n", ultimo.simbolo, estado);
            } else {
                printf(" --> q%zu \n", estado);
            }
        }
    }

    char c = cadena[pos];
    size_t sym = pAlfabeto(c);
    if(sym == (size_t) - 1)
    return;

    size_t index = estado *(csimbolos + 1) + sym;
    for(size_t i = 0; i < conteo[index]; i++){
        size_t destino = tabla[index][i];

        camino[npasos] = (Paso){
            estado, c
        };

        int nuevos_eps[cestados];
        for(size_t j = 0; j < cestados; j++)
        nuevos_eps[j] = 0;

        AFNRecursivo(cadena, largo, pos + 1, destino, camino, npasos + 1, nuevos_eps, encontrado);
    }
}

/*
    Lee la cadena caracter por caracter desde stdio hasta encintrar '\0'
    crea un arreglo para camino que registra el paso recorrido y lo reserva
    dinamicamente.
*/
int AFN(void){
    /*
        Aqui va el afd 
        edoInicial 
        c x c
        buscar la col del simbolo 
        si es simbolo no valido x
        construccion de la matriz
        -> para un afd, no un afn
    */

    /*
        size_t activos[cestados];
        size_t nactivos = 1;
        activos[0] = 0;

        EClausura(activos, &nactivos);
    */

    /*
    printf("Incio: { ");
    for(size_t i = 0; i < nactivos; i++)
        printf(i ? ", q%zu" : "q%zu", activos[i]);
    printf("} \n");
    */

    char cadena[512];
    size_t largo = 0;

    int c;
    while((c = getc(stdin)) != '\n' && c != EOF)
        cadena[largo++] = (char) c;
    cadena[largo] = '\0';

    manejoError(cadena, &largo);

    size_t mPasos = (largo + 1) *cestados * 2 + 1;
    Paso *camino = malloc(mPasos *sizeof(Paso));
    if(!camino){
        fprintf(stderr, "Error al reservar memoria para el camino");
        exit(0);
    }

    int eps_visitado[cestados];
    for(size_t i = 0; i < cestados; i++)
        eps_visitado[i] = 0;
    
        camino[0] = (Paso){
            0, '\0'
        };

    int encontrado = 0;
    AFNRecursivo(cadena, largo, 0, 0, camino, 1, eps_visitado, &encontrado);
    
    free(camino);
    return encontrado;
}

void mover(size_t *conj, size_t n, size_t sym, size_t *dest, size_t *ndest){
    *ndest = 0;

    for(size_t i = 0; i < n; i++){
        size_t index = conj[i] *(csimbolos + 1) + sym;
        for(size_t j = 0; j <conteo[index]; j++){
            size_t d = tabla[index][j];
            int existe = 0;
            for(size_t k = 0; k < *ndest; k++)
            if(dest[k] == d){
                existe = 1;
                break;
            }
            if(!existe)
            dest[(*ndest)++] = d;
        } 
    }
}

int SubConjIguales(size_t *a, size_t na, size_t *b, size_t nb){
    if(na != nb)
    return 0;
    for(size_t i = 0; i < na; i++){
        int enc = 0;
        for(size_t j = 0; j < nb; j++)
        if(a[i] == b[j]){
            enc = 1;
            break;
        }
        if(!enc)
        return 0;
    }
    return 1;
}

void Subconjuntos(size_t *conj, size_t n){
    printf("{ ");
    for(size_t i = 0; i < n; i++)
        printf("q%zu%s", conj[i], (i < n - 1) ? ", " : "");
    printf(" }");
} 

void construccionAFD(void){
    size_t subconj[MAX_ESTADOS_AFD][MAX_ESTADOS_AFD];
    size_t tamanios[MAX_ESTADOS_AFD];
    size_t trans[MAX_ESTADOS_AFD][MAX_ESTADOS_AFD];
    size_t total_afd = 0;

    for(size_t i = 0; i < MAX_ESTADOS_AFD; i++)
        tamanios[i] = 0;
    for(size_t i = 0; i < MAX_ESTADOS_AFD; i++)
        for(size_t j = 0; j < MAX_ESTADOS_AFD; j++)
                trans[i][j] = NoHay;
    
    subconj[0][0] = 0;
    tamanios[0] = 1;
    EClausura(subconj[0], &tamanios[0]);
    total_afd = 1;

    size_t cola[MAX_ESTADOS_AFD];
    size_t frente = 0, fin = 0;
    cola[fin++] = 0;
    
    while(frente < fin){
        size_t actual = cola[frente++];

        for(size_t sym = 0;  sym < csimbolos; sym++){
            size_t dest[MAX_ESTADOS_AFD];
            size_t ndest = 0;

            mover(subconj[actual], tamanios[actual], sym, dest, &ndest);
            if(ndest == 0)
            continue;
            EClausura(dest, &ndest);

            int ya_existe = -1;
            for(size_t i = 0; i < total_afd; i++){
                if(SubConjIguales(subconj[i], tamanios[i], dest, ndest)){
                    ya_existe = (int)i;
                    break;
                }
            }

            if(ya_existe == -1){
                for(size_t i = 0; i < ndest; i++)
                    subconj[total_afd][i] = dest[i];
                tamanios[total_afd] = ndest;
                ya_existe = (int)total_afd;
                cola[fin++] = total_afd;
                total_afd++;
            }
            trans[actual][sym] = (size_t)ya_existe;
        }
    }

    printf("AFD (%zu estados) \n", total_afd);
    printf(" %-4s  %-30s", "Est.", "Subconjunto AFN");
    for(size_t sym = 0; sym < csimbolos; sym++)
        printf(" '%c' ", simbolos[sym]);
    printf("Final? \n");

    printf(" %-4s %-30s", "---", "-----------");
    for(size_t sym = 0; sym < csimbolos; sym++)
        printf(" ---");
    printf(" ----- \n");

    for(size_t i = 0; i < total_afd; i++){
        int es_final = 0;
        for(size_t j = 0; j < tamanios[i]; j++)
            if(edoFinal(subconj[i][j])){
                es_final = 1;
                break;
            }
        char prefijo[4] = "   ";
        if(i == 0)
            prefijo[0] = '>';
        if(es_final)
            prefijo[1] = '*';

        printf(" %s%-2zu ", prefijo, i);

        char buf[64] = "";
        int bpos = 0;
        bpos += sprintf(buf + bpos, "{ ");
        for(size_t j = 0; j < tamanios[i]; j++)
            bpos += sprintf(buf + bpos, "q%zu%s", subconj[i][j], (j < tamanios[i] - 1) ? ", " : "");
        bpos += sprintf(buf + bpos, " }");
        printf("%-30s", buf);
        
        for(size_t sym = 0; sym < csimbolos; sym++){
            if(trans[i][sym] == NoHay)
                printf(" %-5s", "'\0'");
            else
                printf(" Q%-4zu", trans[i][sym]);
        }
        printf(" %s \n", es_final ? "SI" : "No");
    }

     printf("\n  Leyenda:  > estado inicial   * estado final\n\n");

    printf("  Transiciones del AFD\n");
    for (size_t i = 0; i < total_afd; i++) {
        for (size_t sym = 0; sym < csimbolos; sym++) {
            if (trans[i][sym] == NoHay) continue;
            size_t dest_id = trans[i][sym];
            printf("  Q%zu ", i);
            Subconjuntos(subconj[i], tamanios[i]);
            printf("  --%c-->  Q%zu ", simbolos[sym], dest_id);
            Subconjuntos(subconj[dest_id], tamanios[dest_id]);
            printf("\n");
        }
    }
    printf("\n");
}

/*
    Es el main bro que mas quieres saber?
*/
int main(int argc, char *argv[]){
    
    if(argc !=2){
        fprintf(stderr, "Archivo: %s <archivo.txt \n>", argv[0]);
        return 1;
    }
    cAFN(argv[1]);

    printf("Alfabeto: \n");
    for(size_t i = 0; i < csimbolos; i++)
        printf(" '%c'%s ", simbolos[i], (i < csimbolos - 1) ? ", " : "");
    printf("\n");

    printf("Estados finales AFN: ");
    for(size_t i = 0; i < cfinales; i++)
        printf("q%zu%s", finales[i], (i < cfinales - 1) ? ", " : "");
    printf("\n");

    construccionAFD();

    printf("AFN \n");
        

    int c;
    while((c = getc(stdin)) != EOF){
        ungetc(c, stdin);
        if(AFN())
            printf(" \n True \n");
        else
            printf("\n False \n");
    }
    
    fAFN();
    return 0;
}
