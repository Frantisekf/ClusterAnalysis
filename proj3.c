/**
 * Kostra programu pro 3. projekt IZP 2015/16
 *
 * Jednoducha shlukova analyza: 2D nejblizsi soused.
 * Single linkage
 * http://is.muni.cz/th/172767/fi_b/5739129/web/web/slsrov.html
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h> // sqrtf
#include <limits.h> // INT_MAX
#include <errno.h>


/*****************************************************************
 * Ladici makra. Vypnout jejich efekt lze definici makra
 * NDEBUG, napr.:
 *   a) pri prekladu argumentem prekladaci -DNDEBUG
 *   b) v souboru (na radek pred #include <assert.h>
 *      #define NDEBUG
 */
#ifdef NDEBUG
#define debug(s)
#define dfmt(s, ...)
#define dint(i)
#define dfloat(f)
#else

// vypise ladici retezec
#define debug(s) printf("- %s\n", s)

// vypise formatovany ladici vystup - pouziti podobne jako printf
#define dfmt(s, ...) printf(" - "__FILE__":%u: "s"\n",__LINE__,__VA_ARGS__)

// vypise ladici informaci o promenne - pouziti dint(identifikator_promenne)
#define dint(i) printf(" - " __FILE__ ":%u: " #i " = %d\n", __LINE__, i)

// vypise ladici informaci o promenne typu float - pouziti
// dfloat(identifikator_promenne)
#define dfloat(f) printf(" - " __FILE__ ":%u: " #f " = %g\n", __LINE__, f)

#endif

/*****************************************************************
 * Deklarace potrebnych datovych typu:
 *
 * TYTO DEKLARACE NEMENTE
 *
 *   struct obj_t - struktura objektu: identifikator a souradnice
 *   struct cluster_t - shluk objektu:
 *      pocet objektu ve shluku,
 *      kapacita shluku (pocet objektu, pro ktere je rezervovano
 *          misto v poli),
 *      ukazatel na pole shluku.
 */

struct obj_t {
    int id;
    float x;
    float y;
};

struct cluster_t {
    int size;
    int capacity;
    struct obj_t *obj;
};

/*****************************************************************
 * Deklarace potrebnych funkci.
 *
 * PROTOTYPY FUNKCI NEMENTE
 *
 * IMPLEMENTUJTE POUZE FUNKCE NA MISTECH OZNACENYCH 'TODO'
 *
 */

/*
 Inicializace shluku 'c'. Alokuje pamet pro cap objektu (kapacitu).
 Ukazatel NULL u pole objektu znamena kapacitu 0.
*/
void init_cluster(struct cluster_t *c, int cap)
{
    assert(c != NULL);
    assert(cap >= 0);

    c->size = 0;
    c->capacity = cap;
    c->obj = malloc(sizeof(struct obj_t) * cap);
    
    if(c->obj == NULL){
		c->capacity = 0;
	}
    

}

/*
 Odstraneni vsech objektu shluku a inicializace na prazdny shluk.
 */
void clear_cluster(struct cluster_t *c)
{
    free(c->obj);
    c->size = 0;
    c->capacity = 0;
    c->obj = NULL;
    
}

/// Chunk of cluster objects. Value recommended for reallocation.
const int CLUSTER_CHUNK = 10;

/*
 Zmena kapacity shluku 'c' na kapacitu 'new_cap'.
 */ 
struct cluster_t *resize_cluster(struct cluster_t *c, int new_cap)
{
    // TUTO FUNKCI NEMENTE
    assert(c);
    assert(c->capacity >= 0);
    assert(new_cap >= 0);

    if (c->capacity >= new_cap)
        return c;

    size_t size = sizeof(struct obj_t) * new_cap;

    void *arr = realloc(c->obj, size);
    if (arr == NULL)
        return NULL;

    c->obj = arr;
    c->capacity = new_cap;
    return c;
}

/*
 Prida objekt 'obj' na konec shluku 'c'. Rozsiri shluk, pokud se do nej objekt
 nevejde.
 */
void append_cluster(struct cluster_t *c, struct obj_t obj)
{
	assert(c);
	if(c->size == c->capacity){
		c = resize_cluster(c, c->capacity + CLUSTER_CHUNK);
	}
	assert(c);
    c->obj[c->size] = obj;
    c->size++;
    
}

/*
 Seradi objekty ve shluku 'c' vzestupne podle jejich identifikacniho cisla.
 */
void sort_cluster(struct cluster_t *c);

/*
 Do shluku 'c1' prida objekty 'c2'. Shluk 'c1' bude v pripade nutnosti rozsiren.
 Objekty ve shluku 'c1' budou serazny vzestupne podle identifikacniho cisla.
 Shluk 'c2' bude nezmenen.
 */
void merge_clusters(struct cluster_t *c1, struct cluster_t *c2)
{
    assert(c1 != NULL);
    assert(c2 != NULL);
	
	for(int i = 0; i < c2->size; i++){
		
		append_cluster(c1, c2->obj[i]);
	}
	
	sort_cluster(c1);
}

/**********************************************************************/
/* Prace s polem shluku */

/*
 Odstrani shluk z pole shluku 'carr'. Pole shluku obsahuje 'narr' polozek
 (shluku). Shluk pro odstraneni se nachazi na indexu 'idx'. Funkce vraci novy
 pocet shluku v poli.
*/
int remove_cluster(struct cluster_t *carr, int narr, int idx)
{
    assert(idx < narr);
    assert(narr > 0);

    struct cluster_t temp_cluster;
    
    temp_cluster = carr[narr -1];
    carr[narr-1] = carr[idx];
    carr[idx] = temp_cluster;
    
    clear_cluster(&carr[narr - 1]);
    
    return narr - 1;
}

/*
 Pocita Euklidovskou vzdalenost mezi dvema objekty.
 */
float obj_distance(struct obj_t *o1, struct obj_t *o2)
{
    assert(o1 != NULL);
    assert(o2 != NULL);

    float dx , dy, result;
    
    dx = o1->x - o2->x;
    dy = o1->y - o2->y;
    
    
    result = sqrt((dx*dx) + (dy*dy));
    //printf("vzdialenost medzi objektom %d a objektom %d je %f\n", o1->id, o2->id, result);
    
    return result;
}

/*
 Pocita vzdalenost dvou shluku. Vzdalenost je vypoctena na zaklade nejblizsiho
 souseda.
*/
float cluster_distance(struct cluster_t *c1, struct cluster_t *c2)
{
    assert(c1 != NULL);
    assert(c1->size > 0);
    assert(c2 != NULL);
    assert(c2->size > 0);
	
	float minimum = 1000 * sqrt(2); //maximalna vzdialenost medzi dvoma sluhkmi
	float tmp = 0;
    
    for(int i = 0; i < c1->size; i++){
	
		for(int j = 0; j < c2->size; j++){
			
			tmp = obj_distance(&c1->obj[i], &c2->obj[j]);
			if(tmp < minimum){			
				minimum = tmp;
			}
		}
	
	}
	
	return minimum;
	
    
}

/*
 Funkce najde dva nejblizsi shluky. V poli shluku 'carr' o velikosti 'narr'
 hleda dva nejblizsi shluky (podle nejblizsiho souseda). Nalezene shluky
 identifikuje jejich indexy v poli 'carr'. Funkce nalezene shluky (indexy do
 pole 'carr') uklada do pameti na adresu 'c1' resp. 'c2'.
*/
void find_neighbours(struct cluster_t *carr, int narr, int *c1, int *c2)
{
    assert(narr > 0);	
	float minimum = 1000 * sqrt(2);
	float dist;
	int index1 = 0, index2 = 0;

    for(int i = 0; i < narr; i++){
		
		for(int j = i+1; j < narr; j++){
			dist = cluster_distance(&carr[i], &carr[j]);
			//printf("vzdalenost shluku %d a shluku %d je %f\n",i, j, dist);
			if(dist <= minimum){				
				minimum = dist;	
				index1 = i;
				index2 = j;
			}
			
			
		}
	}
    
    *c1 = index1;
    *c2 = index2;
}

// pomocna funkce pro razeni shluku
static int obj_sort_compar(const void *a, const void *b)
{
    // TUTO FUNKCI NEMENTE
    const struct obj_t *o1 = a;
    const struct obj_t *o2 = b;
    if (o1->id < o2->id) return -1;
    if (o1->id > o2->id) return 1;
    return 0;
}

/*
 Razeni objektu ve shluku vzestupne podle jejich identifikatoru.
*/
void sort_cluster(struct cluster_t *c)
{
    // TUTO FUNKCI NEMENTE
    qsort(c->obj, c->size, sizeof(struct obj_t), &obj_sort_compar);
}

/*
 Tisk shluku 'c' na stdout.
*/
void print_cluster(struct cluster_t *c)
{
    // TUTO FUNKCI NEMENTE
    for (int i = 0; i < c->size; i++)
    {
        if (i) putchar(' ');
        printf("%d[%g,%g]", c->obj[i].id, c->obj[i].x, c->obj[i].y);
    }
    putchar('\n');
}

/*
 Ze souboru 'filename' nacte objekty. Pro kazdy objekt vytvori shluk a ulozi
 jej do pole shluku. Alokuje prostor pro pole vsech shluku a ukazatel na prvni
 polozku pole (ukalazatel na prvni shluk v alokovanem poli) ulozi do pameti,
 kam se odkazuje parametr 'arr'. Funkce vraci pocet nactenych objektu (shluku).
 V pripade nejake chyby uklada do pameti, kam se odkazuje 'arr', hodnotu NULL.
*/
int load_clusters(char *filename, struct cluster_t **arr)
{
	int count, id, x, y;
	
    assert(arr != NULL);
    FILE *file = fopen(filename, "r");
    
    if(file == NULL){
		*arr = NULL;
		return 0;
	}
	
	if(fscanf(file, "count=%d\n", &count) != 1){
		fprintf(stderr, "prvy riadok neobsahuje count = cislo\n");
		*arr = NULL;
		fclose(file);
		return 0;
		
	}
	
	*arr = malloc(sizeof(struct cluster_t) * count);
	 for(int i = 0; i < count; i++){
		 
		 if(fscanf(file, "%d %d %d", &id, &x, &y) == 3){
			struct obj_t new_object;
			
			for(int j = 0; j < i; j++){
				if(id == (*arr)[j].obj->id){
					fprintf(stderr, "subor obsahuje viackrat rovnake ID objektu\n");
					 for(int j = i - 1; j >= 0; j--){
						clear_cluster(&(*arr)[j]);
					}
					free(*arr);
					*arr = NULL;
					fclose(file);
					return 0;
				}
			}
			new_object.id = id;
			new_object.x = x;
			new_object.y = y;
			
			
			init_cluster(&(*arr)[i], CLUSTER_CHUNK);
			
			append_cluster(&(*arr)[i], new_object);
		 }else{
			 for(int j = i - 1; j >= 0; j--){
				clear_cluster(&(*arr)[j]);
			}
			 free(*arr);
			 *arr = NULL;
			 fclose(file);
			 return 0;
		 }		 
	 }
	
	fclose(file);
	return count; 
	
	    
}

/*
 Tisk pole shluku. Parametr 'carr' je ukazatel na prvni polozku (shluk).
 Tiskne se prvnich 'narr' shluku.
*/
void print_clusters(struct cluster_t *carr, int narr)
{
    printf("Clusters:\n");
    for (int i = 0; i < narr; i++)
    {
        printf("cluster %d: ", i);
        print_cluster(&carr[i]);
    }
}

int main(int argc, char *argv[])
{
    struct cluster_t *clusters;
    int final_number_of_clusters;
    int cluster_count;
    int index1, index2;
    long number;
    char *end_ptr;
    
    
    
		
	if(argc == 2){
		final_number_of_clusters = 1;
	
	}else if(argc == 3){		
		errno = 0;
		number = strtol(argv[2], &end_ptr, 10);
		
		if(errno){
			fprintf(stderr,"Nastal overflow vstupu zadajte argument N znova.\n");
			return 1;
		}
		else if(number < 0){
			fprintf(stderr,"Bolo zadane zaporne cislo\n");
			return 1;
		}
		else if( *end_ptr != '\0' ){
			fprintf(stderr,"Na vstup bolo zadane bud cislo typu float alebo nejaky string\n");
			return 1;			
		}else{	
			final_number_of_clusters = number;				
		}
		
		
	}else{
		fprintf(stderr, "Nespravny tvar parametrov! Pre spustenie programu zadajte parametre vo formate ./proj3 SUBOR N\n");
		return 1;
	}
	
	
	cluster_count = load_clusters(argv[1], &clusters);
	if(clusters == NULL){
		fprintf(stderr, "Chyba v nacitanom subore\n");
		return 1;
	}
	if(cluster_count < final_number_of_clusters){
		fprintf(stderr, "bol zadany vacsi pocet zhlukov ako pocet nacitanych objektov\n");
			
		for(int i = 0; i < cluster_count; i++){
			clear_cluster(&clusters[i]);
		}
		
		free(clusters);
		return 1;
	}
	
	
	while(cluster_count != final_number_of_clusters){
	
	find_neighbours(clusters, cluster_count, &index1, &index2);
	
	
	merge_clusters(&clusters[index1], &clusters[index2]);
	cluster_count = remove_cluster(clusters, cluster_count, index2);
	
	}
	
	
	print_clusters(clusters, cluster_count);
	for(int i = 0; i < final_number_of_clusters; i++){
		clear_cluster(&clusters[i]);
	}
	free(clusters);
	return 0;
    
}
