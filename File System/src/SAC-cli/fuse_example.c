#include "fuse_example.h"


// Dentro de los argumentos que recibe nuestro programa obligatoriamente
// debe estar el path al directorio donde vamos a montar nuestro FS
//int main(int argc, char *argv[]) {

//	 struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
//
//	// Limpio la estructura que va a contener los parametros
//	memset(&runtime_options, 0, sizeof(struct t_runtime_options));
//
//	// Esta funcion de FUSE lee los parametros recibidos y los intepreta
//	if (fuse_opt_parse(&args, &runtime_options, fuse_options, NULL) == -1) {
//		/** error parsing options */
//		perror("Invalid arguments!");
//		return EXIT_FAILURE;
//	}
//
//	// Si se paso el parametro --welcome-msg
//	// el campo welcome_msg deberia tener el
//	// valor pasado
//	if (runtime_options.welcome_msg != NULL) {
//		printf("%s\n", runtime_options.welcome_msg);
//	}
//
//	// Esta es la funcion principal de FUSE, es la que se encarga
//	// de realizar el montaje, comuniscarse con el kernel, delegar todo
//	// en varios threads
//
//	//return fuse_main(args.argc, args.argv, &hello_oper, NULL);
int main(){
	int socket_servidor = create_socket();
	char* ip_server = "127.0.0.1";
	char* mensaje = malloc(20);
	int port = 4445;


	//conecto el socket
	if(-1 == connect_socket(socket_servidor, ip_server, PORT_SOCKET)){
		printf("Error connect ::NOT FOUND %d \n", PORT_SOCKET);
	}else {
		printf("EL connect anda bien ::E \n");
	}

	printf("Escriba un mensaje a enviar: ");
	//Ingreso el mensaje a enviar
	scanf("%s",mensaje);

	t_paquete *package = crear_paquete(ABC);
	agregar_a_paquete(package, (void*) mensaje, strlen(mensaje) + 1);

	if(send_package(package, socket_servidor) == -1){
		printf("Error en el envio...\n");
	} else {
		printf("Mensaje enviado\n");
	}

	//Recibo datos del servidor
	//Primero recibo el encabezado y el tamanio ya que son datos de tamaño fijo
	MessageHeader* buffer_header = malloc(sizeof(MessageHeader));
	if(-1 == receive_header(socket_servidor, buffer_header)){
		printf("Error al recibir header ::NOT FOUND\n");
	}else {
		printf("Header recibido:\n type: %d\n size : %d ::E\n", buffer_header->type,buffer_header->data_size);
	}

	t_list *cosas = receive_package(socket_servidor, buffer_header);

	switch (buffer_header->type){
		case ABC:
		{
			;
			char *mensaje = (char*)list_get(cosas, 0);
			printf("Mensaje recibido:%s\n", mensaje);
			break;
		}
		default:
		{
			printf("Operacion desconocida. No quieras meter la pata");
			break;
		}
	}

	//Libero el socket
	close_socket(socket_servidor);

//    free(buffer_data);
	//free(mensaje);

	return 0;
}