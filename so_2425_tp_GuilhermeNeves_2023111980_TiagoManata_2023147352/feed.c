#include "util.h"

// variaveis globais
char username[TAM];
char fifo_cliente[TAM];
int fd_server;
int fd_cliente;

void envia_pedido(char *comando, char *topico, char *mensagem, int dura)
{
	PEDIDO p;

	strcpy(p.comando, comando);
	strcpy(p.topics.nome, topico);
	strcpy(p.msg.mensagem, mensagem);
	strcpy(p.user.username, username);

	p.msg.dura = dura;
	p.user.pid = getpid();

	if (write(fd_server, &p, sizeof(PEDIDO)) == -1)
	{
		printf("Ocorreu um erro, não foi possivel enviar a informação para o manager\n");
	}
}

void trata_comando(char *comando, FEED *ptrf)
{
	if (strlen(comando) == 0)
	{
		// Ignora comandos vazios para evitar loops repetitivos
		return;
	}

	char topico[TAM];
	char mensagem[TAM_MENSAGEM];
	int dura;
	if (strcmp(comando, "exit") == 0)
	{
		printf("Comando 'exit' recebido. A encerrar...\n");
		envia_pedido(comando, "", "", 0);
		ptrf->running = false;

		pthread_exit(NULL);
	}
	else if (strcmp(comando, "topics") == 0)
	{
		envia_pedido(comando, "", "", 0);
	}
	else
	{
		printf("Comando Desconhecido: '%s'\n", comando);
	}
}
int numero(const char *str)
{
	char *ptr;
	strtol(str, &ptr, 10); // converter o character para int de base 10
	return (*str != '\0' && *ptr == '\0');
}

//------------------------------------------------------------THREAD PARA INPUTS ---------------------------------------------//

void sair(int num, siginfo_t *si, void *uc)
{
}

void *thread_input_f(void *arg)
{
	FEED *ptrf = (FEED *)arg;
	char comando[400];

	struct sigaction act;

	act.sa_flags = SA_SIGINFO;
	act.sa_sigaction = sair;
	sigaction(SIGUSR1, &act, NULL);

	while (ptrf->running)
	{
		printf("[%s] => ", username);
		fflush(stdout); // Garantir que o prompt é exibido imediatamente

		// Lê o comando do utilizador
		if (fgets(comando, 400, stdin) == NULL)
		{
			sigaction(SIGUSR1, &act, NULL);
			break;
		}
		// Remover o caractere de nova linha e verificar se o comando for vazio

		comando[strcspn(comando, "\n")] = 0;
		if (strlen(comando) == 0)
		{
			// Se o comando for vazio, ignora e pede novamente
			continue;
		}

		char *pal[5];
		int i = 0;
		comando[strcspn(comando, "\n")] = 0;
		pal[i] = strtok(comando, " ");
		while (i < 2)
		{
			i++;

			pal[i] = strtok(NULL, " ");
		}
		pal[3] = strtok(NULL, "\0");

		printf("\nComando lido: '%s'\n", comando); // debugs
		if (strcmp(pal[0], "msg") == 0)
		{
			if (pal[1] != NULL && pal[2] != NULL && pal[3] != NULL && numero(pal[2]))
			{

				int dura = atoi(pal[2]);
				if (dura >= 0)
				{
					envia_pedido(pal[0], pal[1], pal[3], atoi(pal[2]));
				}
				else
					printf("A duração tem de ser um número positivo\n");
			}
			else
				printf("Tens de inserir o comando msg da seguinte forma=> <msg><topico><duração em número><mensagem> \n");
		}
		else if (strcmp(pal[0], "subscribe") == 0)

		{
			if (pal[1] != NULL)

				envia_pedido(pal[0], pal[1], "nada", 1);
			else
				printf("Tens de indicar um tópico para subscrever!\n");
		}
		else if (strcmp(pal[0], "unsubscribe") == 0)
		{

			if (pal[1] != NULL)
				envia_pedido(pal[0], pal[1], "nada", 1);
			else
				printf("Tens de indicar um tópico para cancelar a sua inscrição!\n");
		}
		else if (strcmp(pal[0], "comandos") == 0)
		{
			printf("[---------|Lista de Comandos|---------]: \n");
			printf("->msg <tópico><duração da mensagem><mensagem>\n");
			printf("->topics \n");
			printf("->subscribe <tópico> \n");
			printf("->unsubscribe <tópico> \n");
			printf("->exit \n");
			printf("[---------|-----------------|---------]: \n");
		}

		else
		{
			// Processa o comando

			trata_comando(comando, ptrf);
		}
	}
	pthread_kill(ptrf->receive_thread, SIGUSR1);
	printf("\nThread Input de  dados a terminar\n");

	pthread_exit(NULL);
	return NULL;
}

//------------------------------------------------------------THREAD PARA RECEBER ---------------------------------------------//
void *thread_receive_f(void *arg)
{
	FEED *ptrf = (FEED *)arg;

	struct sigaction act;

	act.sa_flags = SA_SIGINFO;
	act.sa_sigaction = sair;
	while (ptrf->running)
	{
		RESPOSTA r;

		int total_bytes = 0;
		while (total_bytes < sizeof(RESPOSTA))
		{
			int n = read(fd_cliente, ((char *)&r) + total_bytes, sizeof(RESPOSTA) - total_bytes);

			total_bytes += n;
		}

		// Processa a resposta recebida
		if (total_bytes == sizeof(RESPOSTA))
		{

			if (strcmp(r.comando, "mensagem1") == 0)
			{
				printf("\n[Mensagem de %s para o tópico %s]: %s\n", r.msgUser, r.topics[0].nome, r.msg[1].mensagem);
			}
			else if (strcmp(r.comando, "mensagem") == 0)
			{
				printf("\n[Mensagem de %s para o tópico %s]: %s\n", r.msgUser, r.topics[0].nome, r.msg[0].mensagem);
			}

			else if (strcmp(r.comando, "info") == 0)
			{
				if (strcmp(r.msg[0].mensagem, "Já existe um utilizador com este nome") == 0)
				{
					printf("\n[Info]: %s\n", r.msg[0].mensagem);
					unlink(fifo_cliente);
					exit(9);
				}
				else if (strcmp(r.msg[0].mensagem, "O servidor já chegou ao limite de utilizadores") == 0)
				{
					printf("\n[Info]: %s\n", r.msg[0].mensagem);
					unlink(fifo_cliente);
					exit(9);
				}
				printf("\n[Info]: %s\n", r.msg[0].mensagem);
			}

			else if (strcmp(r.comando, "[ERRO]") == 0)
			{
				printf("\n[Erro]: %s\n", r.msg[0].mensagem);
			}
			else if (strcmp(r.comando, "topics") == 0)
			{

				printf("\n[Tópicos]: \n");
				for (int i = 0; i < MAX_TOPICS; i++)
				{

					if (strcmp(r.topics[i].nome, "") == 0)
						break;
					printf(" %s (%s) (%d mensagens persistentes)\n", r.topics[i].nome, r.topics[i].bloqueado ? "bloqueado" : "desbloqueado", r.topics[i].numMensagem);
				}
				printf("[%s] =>", username);
				fflush(stdout);
			}
			else if (strcmp(r.comando, "exit") == 0)
			{

				printf("Foste removido pelo manager\n");
				ptrf->running = false;
				pthread_kill(ptrf->input_thread, SIGUSR1);

				break;
			}
		}
	}

	printf("Thread Receber dados a terminar");

	pthread_exit(NULL);
	return NULL;
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("Forneça um nome de utilizador: ./feed <username>\n");
		exit(1);
	}

	strcpy(username, argv[1]); // guarda formato nome fifo
	int pid = getpid();
	sprintf(fifo_cliente, FIFO_CLIENTE, pid);

	if (access(FIFO_SERVER, F_OK) != 0)
	{ // verifica se tem alguem para falar

		perror("O manager não se encontra em execução\n");
		exit(1);
	}

	if (mkfifo(fifo_cliente, 0666) != 0)
	{
		perror("Erro ao criar o fifo do cliente\n");
		unlink(fifo_cliente);
		exit(1);
	}

	fd_server = open(FIFO_SERVER, O_RDWR); // Abre o fifo do servidor para escrita
	if (fd_server == -1)
	{
		perror("Erro ao abrir o fifo do servidor\n");
		unlink(fifo_cliente);
		exit(1);
	}
	envia_pedido("register", "", "", 0);	   // pedido de registo
	fd_cliente = open(fifo_cliente, O_RDONLY); // Abre o fifo do cliente para leitura
	if (fd_cliente == -1)
	{
		perror("Erro ao abrir o fifo do cliente\n");
		close(fd_server);
		unlink(fifo_cliente);
		exit(1);
	}
	pthread_t thread_input;
	pthread_t thread_receive;

	FEED cliente;

	cliente.running = true;

	printf("Bem vindo ao Fórum:\n Para saber que comandos existem escreva 'comandos'\n");

	if (pthread_create(&thread_input, NULL, thread_input_f, (void *)&cliente) != 0)
	{ // Cria a thread de entrada do user
		perror("Erro ao criar a thread de entrada\n");
		close(fd_server);
		close(fd_cliente);
		unlink(fifo_cliente);
		exit(1);
	}
	cliente.input_thread = thread_input;

	if (pthread_create(&thread_receive, NULL, thread_receive_f, (void *)&cliente) != 0)
	{ // Cria a thread de recepção de mensagens 1- variavel criada, atributos normais por isso null, função que queremos que as threads façam,argumentos 0 logo null

		perror("Erro ao criar a thread de recepção\n");
		pthread_join(thread_input, NULL); // corrige
		close(fd_server);
		close(fd_cliente);
		unlink(fifo_cliente);
		exit(1);
	}
	cliente.receive_thread = thread_receive;

	pthread_join(thread_input, NULL); // Aguarda as threads terminarem
	pthread_join(thread_receive, NULL);

	close(fd_server); // Liberta recursos
	close(fd_cliente);
	unlink(fifo_cliente);

	printf("\n\t ||FIM DO PROGRAMA, VOLTE SEMPRE||\n");

	return 0;
}