#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/select.h>
#include <signal.h>

#define MAX_USERS 10
#define MAX_TOPICS 20
#define MAX_MSG_TOPIC 5
#define TAM 20
#define TAM_MENSAGEM 300
#define FIFO_SERVER "fifo_server"
#define FIFO_CLIENTE "fifo_cliente_%d" // Template para depois inserir o PID

// Definição de USER
typedef struct
{
    char username[TAM];
    int pid;
} USER;

// Definição de MENSAGEM
typedef struct
{

    char topico[TAM];
    char mensagem[TAM_MENSAGEM];
    char username[TAM];
    size_t dura; // Duração em segundos
} MENSAGEM;

// Definição de TOPICO
typedef struct TOPICO
{

    char nome[TAM];
    bool bloqueado;
    int num_subscritores;
    int numMensagem;
    USER subscritores[MAX_USERS];
    MENSAGEM msg[5]; // Até 5 mensagens persistentes
} TOPICO;

// Definição de SUBSCRICAO   // POSSIVELMENTE APAGAR
typedef struct
{
    char username[TAM];
    char topico[TAM];
    int pid;
} SUBSCRICAO; // ATE AQUI APAGAR

// Definição de PEDIDO
typedef struct
{
    SUBSCRICAO subscricao; // Detalhes da subscrição
    MENSAGEM msg;          // Mensagem enviada pelo cliente
    TOPICO topics;         // Informações sobre o tópico
    USER user;             // Dados do utilizador
    char comando[TAM];     // Comando enviado
    char msgUser[20];      // Mensagem do cliente

} PEDIDO;

// Definição de RESPOSTA
typedef struct
{
    // SUBSCRICAO subscricao[MAX_USERS * MAX_TOPICS];
    MENSAGEM msg[MAX_TOPICS * 100];
    TOPICO topics[MAX_TOPICS];
    USER user[MAX_USERS];
    char comando[TAM];
    char msgUser[20];
    char userExpulso[20];

} RESPOSTA;

// Definição de ADMIN
typedef struct
{
    TOPICO topicos[MAX_TOPICS];
    int usersPID[10];
    char users[MAX_USERS][TAM];
    char comando;
    int num_users;
    bool running;
    pthread_mutex_t mutex;
    int fd_server;
    int num_topics;
    int num_mensagens;
    int num_subscricoes;

} ADMIN;

typedef struct
{
    bool running;
    pthread_t input_thread;
    pthread_t receive_thread;
} FEED;

#endif