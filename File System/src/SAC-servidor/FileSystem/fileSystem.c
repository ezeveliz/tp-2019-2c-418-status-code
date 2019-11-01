/* Tareas
 * 1.Crear un archivo
 * 2.Eliminar un archivo
 * 3.Listar los archivos
 * 4.Leer archivo
 * 5.
 *
 */
#include "fileSystem.h"

GBloque* particion = NULL;

int obtenerTamanioArchivo (char* file ){
    //Si devuelve null seguro esta en una carpeta arriba el archivo
    FILE* fd = fopen(file,"r");

    if(fd == NULL){
        return -1;
    }

    fseek(fd, 0L, SEEK_END);
    int32_t dfSize = ftell(fd);

    fclose(fd);
    return dfSize;
}

int obtenerNBloquesBitMap(int disco_size){
        return ((disco_size/BLOQUE_TAMANIO))/8;
}

void escribirHeader(GBloque* puntero_disco, int bitmap_size){
    GHeader* newHeader = (GHeader *) puntero_disco;
    memcpy(newHeader->identificador, "SAC",IDENTIFICADOR_TAMANIO);
    newHeader->version = 1;
    newHeader->bitmap_inicio = 1;
    newHeader->bitMap_tamanio = bitmap_size;
}

char* crearBitMap(int bitmap_count_bloques){

    char* bitarray = malloc(bitmap_count_bloques * BLOQUE_TAMANIO);
    int cantidad_bits = bitmap_count_bloques * 8;


    t_bitarray* bitmap = bitarray_create_with_mode(bitarray, bitmap_count_bloques, MSB_FIRST);

    //Inicializo todos los bits en 0
    for(int pos = 0; pos <= cantidad_bits; pos++){;
        bitarray_clean_bit(bitmap, pos);
    }
    //Seteo los bloques usados en el bitarray
    for(int pos = 0; pos < 12; pos++){
        bitarray_set_bit(bitmap, pos);
    }

    return bitmap->bitarray;
}

void escribirBitMap(GBloque* puntero_disco, int  bitmap_count){

    GBloque* disco = (GBloque *)puntero_disco;
    bitMapPosition= disco;
    char* bitmap = crearBitMap(bitmap_count);

    //Voy copiando el bitmap por partes en los distintos bloques
    for(int bloque = 0; bloque < bitmap_count ;bloque++){
        memcpy(disco[bloque].bytes, bitmap + (bloque * BLOQUE_TAMANIO), BLOQUE_TAMANIO);
    }
}

void escribirNodeTabla (GBloque* puntero_disco){
    GFile* nodo = (GFile *) puntero_disco;
    tablaNodosPosition =nodo;
    //Inicializo toda la tabal de nodos con estado vacio
    for(int numero_archivo = 0; numero_archivo < CANTIDAD_ARCHIVOS_MAX; numero_archivo++){
        nodo[numero_archivo].estado = 0;
    }

}

int formatear (char* nombre_particion, t_log* logger){

    int disco_size = obtenerTamanioArchivo(nombre_particion);
    if(disco_size == -1){
        log_error(logger, "Error al abrir el archivo");
        return -1;
    }

    int bitmap_count_bloques = obtenerNBloquesBitMap(disco_size);

    //Uso open porque necesito la direccion de memoria para memcopy
    int disco_file = open(nombre_particion, O_RDWR, 0);

    //Asigna archivos a memoria (Mapea el .bin a memoria ram)
    GBloque* disco = mmap(NULL, disco_size, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED,disco_file, 0);

    escribirHeader(disco,bitmap_count_bloques);
    log_trace(logger, "Se termino de escribir el header en la direccion:%p",disco );

    escribirBitMap(disco + 1, bitmap_count_bloques);
    log_trace(logger, "Se termino de escribir el BitMap en la direccion:%p",disco + 1 );

    escribirNodeTabla(disco + 1 + bitmap_count_bloques);
    log_trace(logger, "Se termino de escribir la tabla de nodos en la direccion:%p",disco + 1 + bitmap_count_bloques );

    log_trace(logger, "Se hizo un munmap de la memoria" );

    return 0;
}

void mostrarHeader(GBloque* disco ){
    GHeader* header = (GHeader *) disco;
    printf("Header \n\n");
    printf("Identificador: %s\nVersion: %d\nInicio BitMap: %d\nTamaño BitMap: %d\n", header->identificador, header->version, header->bitmap_inicio, header->bitMap_tamanio);
    printf("\n");
}

void mostrarBitMap(GBloque* disco, int bitmap_count_bloques){
    char* bitArray = malloc(bitmap_count_bloques * BLOQUE_TAMANIO);
    t_bitarray* bitmap = bitarray_create_with_mode(bitArray, bitmap_count_bloques, MSB_FIRST);
    memcpy(bitArray, disco, bitmap_count_bloques*BLOQUE_TAMANIO);
    printf("BitMap\n\n");
    int bits_usados = 0;
    int bits_libres = 0;
    int pos_bit;
    for(pos_bit = 0; pos_bit < bitmap_count_bloques*8; pos_bit++){
        int valor_bit = bitarray_test_bit(bitmap, pos_bit);
        if(valor_bit > 0){
            bits_usados++;
        }else{
            bits_libres++;
        }
        printf("%d",bitarray_test_bit(bitmap, pos_bit));
    }
    printf("\n \n");
    printf("Bloques: %d\t Bloques usados:%d\t Bloques libres: %d\n", pos_bit, bits_usados, bits_libres);
    printf("\n");

}

void mostrarTablaNodos(GBloque* disco){
        GFile* nodo = (GFile *) disco;
        printf("Tabal de nodos \n\n");
        for(int file = 1; file <= CANTIDAD_ARCHIVOS_MAX; file ++){
            printf("Numero:%d\t Estado:%d \n", file, nodo[file].estado);
        }
}

void mostrarParticion(char* nombre_particion){
    int disco_size = obtenerTamanioArchivo(nombre_particion);
    int bitmap_count_bloques = obtenerNBloquesBitMap(disco_size);
    int disco_file = open(nombre_particion, O_RDWR, 0);
    GBloque* disco = mmap(NULL, disco_size, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED,disco_file, 0);

    mostrarHeader(disco);

    mostrarBitMap(disco + 1,bitmap_count_bloques);

    mostrarTablaNodos(disco + 1 + bitmap_count_bloques);

    munmap(disco,disco_size);
}


GHeader* obtenerHeader(){
    return (GHeader *) particion;
}

//Acordate de liberar esta funcion
t_bitarray* obtenerBitMap(){
    t_bitarray* bitmap = bitarray_create_with_mode(bitArray, bitmap_count_bloques, MSB_FIRST);
    memcpy(bitArray, disco, bitmap_count_bloques*BLOQUE_TAMANIO);

    return bitmap;
}

GFile* obtenerTablaNodos(){
    GHeader* header = obtenerHeader();

}

int buscarBloque(char * ruta){


}

int obtenerPunteroArchivo(GBloque* bloque,char* nombre){

}

int buscarPadre(char* padre){

}

int buscarBloqueMemoriaLibre(){

}

int sac_getattr (const char *, struct stat *, struct fuse_file_info *fi){

}

// No se que es mode_t
int sac_mkdir (const char* ruta, mode_t){

}

//  La funcion real toma estos dos parametros tambien mode_t, dev_t
int sac_mknod (const char * ruta){

}

int sac_unlink ) ( const  char *){

}

int rmdir ( const  char *){


}

int open (const char *, struct fuse_file_info *){


}

int read (const char *, char *, size_t, off_t, struct fuse_file_info *){


}

int write (const char *, const char *, size_t, off_t, struct fuse_file_info *){


}





int main(){
  t_log* logger = log_create("sac-server.logger", "fileSystem", 1, LOG_LEVEL_TRACE);

    formatear("pueba.bin", logger);
    mostrarParticion("../prueba.bin");


    return 0;
}