#ifndef SUSE_SUSE_H
#define SUSE_SUSE_H

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/dictionary.h>
#include <altaLibreria/connections.h>
#include <altaLibreria/structures.h>
#include "suseStructures.h"

/**
 * Inicializo el log
 */
void start_log();

/**
 * Inicializo semaforos, listas de estados comunes y la estructura de configuracion
 */
void initialize_structures();

/**
 * Leo las configuraciones del archivo y las cargo en el struct SUSEConfig
 */
void read_config_options();

/**
 * Funcion encargada de definir las tres funciones para el servidor(new, incoming y lost)
 */
void server_function();

/**
 * Creo un nuevo proceso y lo cargo en la lista de procesos
 * @param ip
 * @param port
 * @param fd socket del cliente
 */
void create_new_program(char* ip, int port, int fd);

/**
 * Creo las estructuras necesarias para representar al tid suministrado por hilolay. En esta operacion hay varios casos
 * posibles:
 * * Grado de multiprogramacion >= limite:
 * * * Si el programa ya estaba en ejecucion(exec = true), se agrega el hilo a NEW y se retorna 1.
 * * * Si el programa no estaba en ejecucion, se agrega hilo a NEW y se retorna -1, de esta manera, el cliente se cuelga
 * * * * y se queda esperando una nueva confirmacion, ya que la funcion init, coloca al primer hilo en ejecucion(cliente)
 * * * * logrando una inconsistencia, estaria ejecutando en cliente pero en NEW en SUSE, si lo bloqueamos, logramos
 * * * * que no se produzca tal incosistencia.
 * * Grado de multiprogramacion < limite:
 * * * Si el programa ya estaba en ejecucion, se verifica si  el estado exec es distinto de NULL, en cuyo caso, el hilo
 * * * * se agregara a la lista de readys.
 * * * Si el programa no estaba en ejecucion, se agrega el hilo directo al estado de ejecucion del programa
 * @param fd
 * @param ip
 * @param port
 * @param received(tid)
 */
void suse_create(int fd, char* ip, int port, t_list* received);

/**
 * Retorna el proximo TID a ejecutar segun el algoritmo de planificacion de SUSE. En esta operacion hay varios casos
 * posibles:
 * * Lista de ready del programa > 0:
 * * * exec != NULL, se debe llamar al planificador para que me diga el proximo hilo a ejecutar, actualizar los
 * * * * intervalos de listo y de ejecucion, mover el proximo hilo a ejecutar a exec y al que estaba en exec a la lista
 * * * * de listos, retorna el proximo tid a ejecutar.
 * * * exec == NULL, se llama al planificador, se obtiene el proximo hilo a ejecutar, se actualizan los intervalos
 * * * * correspondientes y se coloca el hilo a ejecutar en exec, retorna el proximo tid a ejecutar.
 * * Lista de ready del programa = 0:
 * * * exec != NULL, se actualizan los intervalos de ejecucion, y se retorna el mismo hilo, ya que no hay otros
 * * * * disponibles, retorna el proximo tid a ejecutar.
 * * * exec == NULL, se coloca al programa en una lista de espera y se bloquea al cliente hasta que haya algun hilo
 * * * * disponible, retorna -1.
 * @param fd
 * @param ip
 * @param port
 * @param received
 * @note el valor de los tids es siempre >= 0, de ahi que el codigo de error sea -1.
 */
void suse_schedule_next(int fd, char * ip, int port, t_list* received);

/**
 * Realiza las tareas administrativas como actualizar los intervalos y mover los hilos de una cola a la otra luego de
 * replanificar el proceso
 * @param program
 * @return int tid a ejecutar a continuacion, o -1 si el programa no esta en ejecucion
 */
int schedule_next(t_program* program);

/**
 * Retorno el sgte hilo a ejecutarse utilizando el algoritmo SJF-E
 * @param program, programa al que le busco el nuevo hilo
 * @return t_thread*, sgte hilo a ejecutarse
 */
t_thread* schedule_new_thread(t_program* program);

/**
 * Bloqueo un thread esperando que el mismo termine. El thread actual pasara a estar en BLOCKED y saldra solo cuando el
 * hilo que lo bloqueo termine su ejecucion con suse_close. Es posible bloquear a un hilo con un hilo ya finalizado.
 * @param fd
 * @param ip
 * @param port
 * @param received
 */
void suse_join(int fd, char * ip, int port, t_list* received);

/**
 * Da por finalizado el TID indicado, el thread actual pasa a EXIT, llamo a la funcion de las
 * metricas y elimino la estructura del programa
 * @param fd
 * @param ip
 * @param port
 * @param received
 */
void suse_close(int fd, char * ip, int port, t_list* received);

/**
 * Cada vez que se ejecuta suse_close, se libera una posicion de las limitadas por la multiprogramacion, esta funcion
 * se ocupa de darle hilos a los programas que hayan solicitado uno(van a estar en la lista asking_for_thread), los
 * mismos van a tener proridad con respecto a los programas que todavia no hayan entrado a la planificacion, una vez
 * que se vacia la lista, se empiezan a repartir los hilos de la lista NEW.
 */
void distribute_new_thread();

//--Genera una operación de wait sobre el semáforo dado
void* suse_wait(void* newComm);

//--Genera una operación de signal sobre el semáforo dado
void* suse_signal(void* newComm);

/**
 * Funcion que ejecuta a la funcion que produce las metricas, corre en un hilo paralelo
 * @param arg
 * @return Este return esta solo para cumplir con la firma de las funcion que acepta pthread, no devuelve nada
 */
void* metrics_function(void* arg);

/**
 * Funcion que genera y loggea las metricas
 * @return Este return esta solo para cumplir con la firma de las funcion que acepta pthread, no devuelve nada
 */
void* generate_metrics(void* arg);

/**
 * Genero las metricas de cada hilo
 * @return char* metricas
 */
char* generate_thread_metrics();

/**
 * Genero las metricas de cada programa
 * @return char* metricas
 */
char* generate_program_metrics();

/**
 * Genero las metricas del sistema
 * @return char* metricas
 */
char* generate_system_metrics();

//--HELPERS

/**
 * Genero un identificador de proceso en base a la ip y al puerto desde los que se conecta el cliente
 * @param ip
 * @param port
 * @return PID pid
 */
PID generate_pid(char* ip, int port);

/**
 * Hallo el grado de multiprogramacion total del sistema(cant de hilos que no estan en new)
 * @return int grado
 */
int multiprogramming_grade();

/**
 * Retorno una estructura que representa al tiempo en segundos y microsegundos
 * @return
 */
struct timespec get_time();

/**
 * Wrapper para liberar una lista, nombre mas corto
 * @param received Lista
 * @param element_destroyer
 */
void free_list(t_list* received, void(*element_destroyer)(void*));

/**
 * Retorno el programa al que le corresponde un PID dado
 * @param pid
 * @return t_program*
 */
t_program* find_program(PID pid);

/**
 * Retorno el hilo al que pertenece un tid y pid dados
 * @param program
 * @param tid
 * @return t_thread*
 */
t_thread* find_thread(t_program* program, TID tid);

/**
 * Creo un hilo para responderle al cliente
 * @param fd
 * @param response
 * @param header
 */
void create_response_thread(int fd, int response, MessageType header);

/**
 * Creo un paquete de respuesta con los datos dados(para enviar al cliente)
 * @param fd
 * @param response
 * @param header
 * @return
 */
void* create_response_package(int fd, int response, MessageType header);

/**
 * Funcion encargada de enviar la respuesta al cliente
 * @param response_package
 * @return
 */
void* response_function(void* response_package);

/**
 * Hallo el ultimo intervalo de la lista de ejecucion
 * @param thread
 * @return interval* interval
 */
t_interval* last_exec(t_thread* thread);

/**
 * Hallo el ultimo intervalo de la lista de listos
 * @param thread
 * @return interval* interval
 */
t_interval* last_ready(t_thread* thread);

/**
 * Creo un nuevo intervalo con su memoria ya alocada
 * @return interval* interval
 */
t_interval* new_interval();

/**
 * Destruyo el hilo a cerrar
 * @param thread, hilo a destruir
 */
void destroy_thread(t_thread* thread);

/**
 * Verifico si un programa dado no posee mas hilos en ready, blocked y new
 * @param program, programa sobre el que se verifica
 * @return true si no tiene mas hilos, false si tiene mas hilos
 */
bool no_more_threads(t_program* program);

/**
 * Retorna la cantidad de hilos en el estado NEW de un programa especifico
 * @param program
 * @return int, cant de hilos en NEW
 */
int threads_in_new(t_program* program);

/**
 * Retorna la cantidad de hilos en el estado READY de un programa especifico
 * @param program
 * @return int, cant de hilos en READY
 */
int threads_in_ready(t_program* program);

/**
 * Retorna la cantidad de hilos en el estado BLOCKED de un programa especifico
 * @param program
 * @return int, cant de hilos en BLOCKED
 */
int threads_in_blocked(t_program* program);

/**
 * Retorna la cantidad de hilos en join_block de un programa especifico
 * @param program
 * @return
 */
int threads_in_join_block(t_program* program);

/**
 * Retorna la cantidad de hilos en semaphore_block de un programa especifico
 * @param program
 * @return
 */
int threads_in_semaphore_block(t_program* program);

/**
 * Retorna la cantidad de hilos en el estado EXEC de un programa especifico
 * @param program
 * @return int, cant de hilos en EXEC
 */
int threads_in_exec(t_program* program);

/**
 * Destruyo el programa dado junto a todas sus estructuras asociadas
 * @param pid, pid del programa a destruir
 */
void destroy_program(PID pid);

/**
 * Destruyo todos los hilos de EXIT que pertenezcan a un programa dado
 * @param pid
 */
void destroy_exit_threads(PID pid);

/**
 * Verifico si el hilo esta muerto(en la cola de EXIT)
 * @param thread, hilo que voy a revisar
 * @return bool, true para muerto, false para vivo
 */
bool blocking_thread_is_dead(t_thread* thread);

/**
 * Libero los posibles join_blocks que pueda llegar a tener un hilo
 * @param thread, hilo del que voy a liberar los blocks
 * @param program
 */
void free_join_blocks(t_thread* thread, t_program* program);

#endif //SUSE_SUSE_H