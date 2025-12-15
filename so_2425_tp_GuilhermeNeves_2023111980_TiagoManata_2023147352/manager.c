#include "util.h"

void envia_resposta(RESPOSTA *resposta, int flag, char *topico, ADMIN *ptra, int flagClose, int exit)
{
    char fifo_cliente[TAM];
    sprintf(fifo_cliente, FIFO_CLIENTE, resposta->user->pid);

    if (flag == 0)
    {

        int fd_cliente = open(fifo_cliente, O_WRONLY);
        if (fd_cliente == -1)
        {
            printf("[Manager] Erro ao abrir FIFO do cliente para enviar resposta,%d \n", resposta->user->pid);
            return;
        }

        if (write(fd_cliente, resposta, sizeof(RESPOSTA)) == -1)
        {
            perror("[Manager] Erro ao enviar resposta ao cliente\n");
        }
        else
        {
        }
        close(fd_cliente);
    }
    else if (flag == 1)
    {
        for (int i = 0; i < ptra->num_topics; i++)
        {
            if (strcmp(ptra->topicos[i].nome, topico) == 0)
            {
                for (int j = 0; j < MAX_USERS; j++)
                {
                    if (ptra->topicos[i].subscritores[j].pid != 0)
                    {

                        if (ptra->usersPID[j] != 0 || ptra->usersPID[j] == ptra->topicos[i].subscritores[j].pid)

                        {
                            char fifo_cliente[TAM];
                            sprintf(fifo_cliente, FIFO_CLIENTE, ptra->topicos[i].subscritores[j].pid);
                            int fd_cliente = open(fifo_cliente, O_WRONLY);

                            if (write(fd_cliente, resposta, sizeof(RESPOSTA)) == -1)
                            {
                                perror("[Manager] Erro ao enviar resposta ao cliente\n");
                            }
                            close(fd_cliente);
                        }
                    }
                }
            }
        }
    }
    else if (flag == 2)
    {
        if (flagClose == 0)
        {
            for (int i = 0; i < MAX_USERS; i++)
            {
                if (ptra->usersPID[i] != 0 && ptra->usersPID[i] != resposta->user->pid)
                {

                    if (exit == 0)
                    {

                        char fifo_cliente[TAM];
                        strcpy(resposta->comando, "info");
                        snprintf(resposta->msg[0].mensagem, sizeof(resposta->msg[0].mensagem),
                                 "O user %s foi removido pelo manager", resposta->userExpulso);
                        sprintf(fifo_cliente, FIFO_CLIENTE, ptra->usersPID[i]);
                        int fd_cliente = open(fifo_cliente, O_WRONLY);
                        write(fd_cliente, resposta, sizeof(RESPOSTA));
                        close(fd_cliente);
                    }
                    else
                    {

                        char fifo_cliente[TAM];
                        strcpy(resposta->comando, "info");
                        snprintf(resposta->msg[0].mensagem, sizeof(resposta->msg[0].mensagem),
                                 "O user %s saiu", resposta->userExpulso);
                        sprintf(fifo_cliente, FIFO_CLIENTE, ptra->usersPID[i]);
                        int fd_cliente = open(fifo_cliente, O_WRONLY);
                        write(fd_cliente, resposta, sizeof(RESPOSTA));
                        close(fd_cliente);
                    }
                }
                else if (ptra->usersPID[i] == resposta->user->pid)
                {

                    strcpy(resposta->comando, "exit");
                    // strcpy(resposta->msg[0].mensagem, "foste removido");
                    char fifo_cliente[TAM];
                    sprintf(fifo_cliente, FIFO_CLIENTE, resposta->user->pid);
                    int fd_cliente = open(fifo_cliente, O_WRONLY);
                    write(fd_cliente, resposta, sizeof(RESPOSTA));
                    close(fd_cliente);
                }
            }
        }
        else if (flagClose == 1)
        {

            //  for (int i = 0; i < MAX_USERS; i++)
            //{

            //  if (ptra->usersPID[i] != 0)
            //    {
            strcpy(resposta->comando, "exit");
            // strcpy(resposta->msg[0].mensagem, "foste removido");
            char fifo_cliente[TAM];
            sprintf(fifo_cliente, FIFO_CLIENTE, resposta->user->pid);
            int fd_cliente = open(fifo_cliente, O_WRONLY);

            write(fd_cliente, resposta, sizeof(RESPOSTA));

            close(fd_cliente);
            //      }
            //}
        }
    }
}

void adicionar_user(char *username, int pid, ADMIN *ptra)
{

    if (ptra->num_users == MAX_USERS)
    {
        printf("Número máximo de users atingido.\n");
        return;
    }
    pthread_mutex_lock(&ptra->mutex);
    for (int i = 0; i < MAX_USERS; i++)
    {
        if (strcmp(ptra->users[i], username) == 0)

        {
            pthread_mutex_unlock(&ptra->mutex);
            return;
        }
    }
    pthread_mutex_unlock(&ptra->mutex);

    pthread_mutex_lock(&ptra->mutex);
    for (int i = 0; i < MAX_USERS; i++)
    {
        if (strcmp(ptra->users[i], "") == 0 || ptra->usersPID[i] == 0)
        {
            strcpy(ptra->users[i], username);
            ptra->usersPID[i] = pid;
            ptra->num_users++;
            pthread_mutex_unlock(&ptra->mutex);
            return;
        }
    }
    pthread_mutex_unlock(&ptra->mutex);
}
void remover_user(char *username, RESPOSTA *resp, ADMIN *ptra, int flagClose, int exit)
{
    int flag = 0;
    if (flagClose == 1)
    {
        pthread_mutex_lock(&ptra->mutex);
        for (int i = 0; i < MAX_USERS; i++)
        {
            if (ptra->usersPID[i] != 0)
            {
                strcpy(resp->comando, "exit");

                char user[20];
                strcpy(user, username);
                resp->user->pid = ptra->usersPID[i];

                envia_resposta(resp, 2, "", ptra, 1, 0);

                // Remove o user
                ptra->usersPID[i] = 0;
                strcpy(ptra->users[i], "");
                flag = 1;
                ptra->num_users--;
            }
        }

        pthread_mutex_unlock(&ptra->mutex);
        if (flag == 0)
        {
            printf(" \n[Manager] Sem necessidade de remover users.\n");
        }
        else
        {
            printf(" \n[Manager] Todos os users removidos.\n");
        }
        return;
    }
    else
    {
        pthread_mutex_lock(&ptra->mutex);

        if (exit != 1)
        {
            for (int i = 0; i < MAX_USERS; i++)
            {
                if (strcmp(ptra->users[i], username) == 0)
                {
                    strcpy(resp->comando, "exit");

                    char user[20];
                    strcpy(user, username);
                    resp->user->pid = ptra->usersPID[i];

                    envia_resposta(resp, 2, "", ptra, 0, 0);
                    // Remove o user
                    ptra->usersPID[i] = 0;
                    strcpy(ptra->users[i], "");

                    ptra->num_users--;

                    printf(" \t [User]-> %s removido.\n", user);
                    pthread_mutex_unlock(&ptra->mutex);
                    return;
                }
            }
        }
        else
        {
            for (int i = 0; i < MAX_USERS; i++)
            {
                if (strcmp(ptra->users[i], username) == 0)
                {
                    strcpy(resp->comando, "exit");

                    char user[20];
                    strcpy(user, username);
                    resp->user->pid = ptra->usersPID[i];

                    envia_resposta(resp, 2, "", ptra, 0, 1);
                    // Remove o user
                    ptra->usersPID[i] = 0;
                    strcpy(ptra->users[i], "");

                    ptra->num_users--;

                    printf(" \t [User]-> %s removido.\n", user);
                    pthread_mutex_unlock(&ptra->mutex);
                    return;
                }
            }
        }
        pthread_mutex_unlock(&ptra->mutex);
    }
    printf("User não foi encontrado \n");
}

bool user_ativo(char *username, PEDIDO *pedido, ADMIN *ptra)
{
    pthread_mutex_lock(&ptra->mutex);
    for (int i = 0; i < ptra->num_users; i++)
    {
        if (strcmp(ptra->users[i], username) == 0 && (ptra->usersPID[i]) != (pedido->user.pid))
        {
            pthread_mutex_unlock(&ptra->mutex);
            return true;
        }
    }
    pthread_mutex_unlock(&ptra->mutex);
    return false;
}

void adicionar_topico(char *nome, ADMIN *ptra, RESPOSTA *resp)
{

    if (strlen(nome) > 20)
    {
        strcpy(resp->comando, "info");
        strcpy(resp->msg[0].mensagem, "Não podem haver tópicos com mais do que 20 caracteres");
        envia_resposta(resp, 0, "nada", ptra, 0, 0);
        return;
    }

    pthread_mutex_lock(&ptra->mutex);
    for (int i = 0; i < MAX_TOPICS; i++)
    {
        if (strcmp(ptra->topicos[i].nome, nome) == 0)
        {
            pthread_mutex_unlock(&ptra->mutex);
            return; // ja foi verificado já pode sair
        }
    }

    // se n existe cria um novo

    if (ptra->num_topics < MAX_TOPICS)
    {

        strcpy(ptra->topicos[ptra->num_topics].nome, nome);

        ptra->num_topics++;
        printf("Tópico %s criado\n", nome);
    }

    else
    {

        printf("Atingido o limite de tópicos\n");
    }
    pthread_mutex_unlock(&ptra->mutex);
}

void set_bloqueio_topico(char *nome, bool bloqueado, RESPOSTA *resp, ADMIN *ptra)
{
    pthread_mutex_lock(&ptra->mutex);
    for (int i = 0; i < MAX_TOPICS; i++)
    {
        if (strcmp(ptra->topicos[i].nome, nome) == 0)
        {

            ptra->topicos[i].bloqueado = bloqueado;

            printf("Tópico %s %s.\n", nome, ptra->topicos[i].bloqueado ? "bloqueado" : "desbloqueado");

            if (bloqueado == true)
            {
                strcpy(resp->topics[i].nome, ptra->topicos[i].nome);
                strcpy(resp->comando, "info");
                snprintf(resp->msg[0].mensagem, sizeof(resp->msg[0].mensagem),
                         "O tópico %s foi bloqueado", ptra->topicos[i].nome);

                envia_resposta(resp, 1, ptra->topicos[i].nome, ptra, 0, 0);
            }
            else
            {
                strcpy(resp->topics[i].nome, ptra->topicos[i].nome);
                strcpy(resp->comando, "info");
                snprintf(resp->msg[0].mensagem, sizeof(resp->msg[0].mensagem),
                         "O tópico %s foi desbloqueado", ptra->topicos[i].nome);

                envia_resposta(resp, 1, ptra->topicos[i].nome, ptra, 0, 0);
            }
            pthread_mutex_unlock(&ptra->mutex);
            return;
        }
    }
    pthread_mutex_unlock(&ptra->mutex);
    printf("Tópico %s não encontrado.\n", nome);
}

void adicionar_subscricao(char *username, char *topico, PEDIDO *pedido, ADMIN *ptra, RESPOSTA *resp)
{
    int flag = 0;
    pthread_mutex_lock(&ptra->mutex);

    for (int i = 0; i < MAX_TOPICS; i++)
    {
        if (strcmp(ptra->topicos[i].nome, topico) == 0)
        {

            for (int j = 0; j < MAX_USERS; j++)
            {
                if (ptra->topicos[i].subscritores[j].pid == 0)
                {
                    strcpy(ptra->topicos[i].subscritores[j].username, pedido->user.username);
                    ptra->topicos[i].subscritores[j].pid = pedido->user.pid;
                    printf("User %s  %d subscreveu o tópico %s.\n", username, pedido->user.pid, topico);
                    flag = 1;
                    break;
                }
            }
        }
    }
    pthread_mutex_unlock(&ptra->mutex);
    if (flag == 0)
    {

        printf("Limite de subscrições atingido.\n");
    }
    return;
}

int remover_subscricao(char *username, char *topico, RESPOSTA *resp, ADMIN *ptra, PEDIDO *pedido)
{

    pthread_mutex_lock(&ptra->mutex);
    for (int i = 0; i < MAX_TOPICS; i++)
    {
        if (strcmp(ptra->topicos[i].nome, topico) == 0)
        {
            for (int j = 0; j < MAX_USERS; j++)
            {
                if (strcmp(ptra->topicos[i].subscritores[j].username, username) == 0)
                {
                    strcpy(resp->comando, "info");
                    strcpy(ptra->topicos[i].subscritores[j].username, "");
                    ptra->topicos[i].subscritores[j].pid = 0;
                    printf("User %s cancelou a sua inscrição ao tópico %s.\n", username, topico);
                    strcpy(resp->msg[0].mensagem, "Subscrição removida com sucesso");
                    envia_resposta(resp, 0, 0, ptra, 0, 0);
                    pthread_mutex_unlock(&ptra->mutex);

                    return 1;
                }
            }
        }
    }
    pthread_mutex_unlock(&ptra->mutex);
    strcpy(resp->comando, "info");
    strcpy(resp->msg[0].mensagem, "Não existe o tópico para dar cancelar a subscrição.");
    envia_resposta(resp, 0, 0, ptra, 0, 0);
    printf("Subscrição não encontrada para o user %s no tópico %s.\n", username, topico);
    return 0;
}

bool topico_existe(char *nome, RESPOSTA *resp, ADMIN *ptra) // recebe o nome do topico
{
    pthread_mutex_lock(&ptra->mutex);
    for (int i = 0; i < MAX_TOPICS; i++) // corre o array dos topicos já criados e verifica se há algum igual
    {
        if (strcmp(ptra->topicos[i].nome, nome) == 0)
        {
            pthread_mutex_unlock(&ptra->mutex);
            return true; // se sim retorna que o tópico já existe, e neste caso só subscreve
        }
    }
    pthread_mutex_unlock(&ptra->mutex);
    return false; // se não retorna só que n existe e neste caso cria e subscreve
}

bool topico_bloqueado(char *nome, RESPOSTA *resp, ADMIN *ptra)
{
    pthread_mutex_lock(&ptra->mutex);
    for (int i = 0; i < MAX_TOPICS; i++)
    {
        if (strcmp(ptra->topicos[i].nome, nome) == 0)
        {
            bool bloqueado = ptra->topicos[i].bloqueado;
            pthread_mutex_unlock(&ptra->mutex);
            return bloqueado;
        }
    }
    pthread_mutex_unlock(&ptra->mutex);
    return false;
}

void processar_pedido(PEDIDO *pedido, ADMIN *ptra)
{
    RESPOSTA resp;
    memset(&resp, 0, sizeof(RESPOSTA));
    resp.user[0].pid = pedido->user.pid;

    printf("[Manager] Pedido recebido do utilizador: %s, comando: %s\n", pedido->user.username, pedido->comando);

    if (strcmp(pedido->comando, "register") == 0)
    {
        // Responde ao cliente com confirmação
        strcpy(resp.comando, "info");
        int buff;
        if (ptra->num_users == MAX_USERS)
        {
            strcpy(resp.msg[0].mensagem, "O servidor já chegou ao limite de utilizadores");
            printf("Utilizador não registado por falta de espaço");
            envia_resposta(&resp, 0, "", ptra, 0, 0);

            return;
        }
        for (int i = 0; i < MAX_USERS; i++)
        {
            if (strcmp(ptra->users[i], pedido->user.username) == 0 && (ptra->usersPID[i]) != (pedido->user.pid))
            {
                strcpy(resp.msg[0].mensagem, "Já existe um utilizador com este nome");
                break;
            }
        }
        strcpy(resp.msg[0].mensagem, "Registo efetuado com sucesso.");

        if (user_ativo(pedido->user.username, pedido, ptra) == false)
        {
            adicionar_user(pedido->user.username, pedido->user.pid, ptra);
            printf("[Manager] Utilizador %s registado com sucesso.\n", pedido->user.username);
            strcpy(resp.msg[0].mensagem, "Registo efetuado com sucesso.");
        }
        else
        {

            strcpy(resp.msg[0].mensagem, "Já existe um utilizador com este nome");
        }
        envia_resposta(&resp, 0, "", ptra, 0, 0);
        printf("[Manager] Resposta de registo enviada para o utilizador:['%s']  com o PID: ['%d']  \n", pedido->user.username, pedido->user.pid);
    }

    else if (strcmp(pedido->comando, "exit") == 0)
    {
        strcpy(resp.userExpulso, pedido->user.username);
        remover_user(pedido->user.username, &resp, ptra, 0, 1);
        // Responde ao cliente com confirmação
        strcpy(resp.comando, "info");
        strcpy(resp.msg[0].mensagem, "User removido com sucesso.");

        //  envia_resposta(&resp, 0, 0, ptra, 0, 0);
        printf("[Manager] Utilizador %s removido com sucesso.\n", pedido->user.username);
    }
    else if (strcmp(pedido->comando, "subscribe") == 0)
    {
        printf("[Manager] Processando subscrição para o utilizador %s ao tópico %s\n", pedido->user.username, pedido->topics.nome);
        if (ptra->num_topics < MAX_TOPICS && topico_existe(pedido->topics.nome, &resp, ptra) == false)
        {

            adicionar_topico(pedido->topics.nome, ptra, &resp);
            strcpy(resp.msg[0].mensagem, "Tópico criado e subscrito com sucesso.");
            printf("[Manager] Tópico %s criado e subscrito pelo utilizador %s\n", pedido->topics.nome, pedido->user.username);
        }

        else
        {

            if (ptra->num_topics == MAX_TOPICS)
            {
                strcpy(resp.comando, "info");
                strcpy(resp.msg[0].mensagem, "Não há espaço para criar esse tópico");
                envia_resposta(&resp, 0, "nada", ptra, 0, 0);
                return;
            }

            else
            {
                strcpy(resp.comando, "info");
                strcpy(resp.msg[0].mensagem, "Subscrição efetuada com sucesso.");
                printf("[Manager] Subscrição efetuada pelo utilizador %s ao tópico %s\n", pedido->user.username, pedido->topics.nome);
            }
        }
        adicionar_subscricao(pedido->user.username, pedido->topics.nome, pedido, ptra, &resp);
        strcpy(resp.comando, "info");
        envia_resposta(&resp, 0, 0, ptra, 0, 0);

        pthread_mutex_lock(&ptra->mutex);
        for (int l = 0; l < MAX_TOPICS; l++)
        {
            if (strcmp(ptra->topicos[l].nome, pedido->topics.nome) == 0)
            {
                for (int k = 0; k < MAX_MSG_TOPIC; k++)
                {

                    if (strcmp(ptra->topicos[l].msg[k].mensagem, "") != 0)
                    {
                        strcpy(resp.comando, "mensagem1");
                        strcpy(resp.msgUser, ptra->topicos[l].msg[k].username);
                        resp.user->pid = pedido->user.pid;
                        strcpy(resp.msg[1].mensagem, ptra->topicos[l].msg[k].mensagem);

                        envia_resposta(&resp, 0, "", ptra, 0, 0);
                    }
                }
            }
        }
        pthread_mutex_unlock(&ptra->mutex);
    }
    else if (strcmp(pedido->comando, "unsubscribe") == 0)
    {

        int temp = remover_subscricao(pedido->user.username, pedido->topics.nome, &resp, ptra, pedido);

        if (temp == 1)
        {
            printf("[Manager] Subscrição removida do utilizador %s ao tópico %s\n", pedido->user.username, pedido->topics.nome);

            pthread_mutex_lock(&ptra->mutex);
            for (int i = 0; i < MAX_MSG_TOPIC; i++)
            {
                if (strcmp(ptra->topicos[i].nome, pedido->topics.nome) == 0)
                {
                    for (int k = 0; k < MAX_MSG_TOPIC; k++)

                    {
                        if (strcmp(ptra->topicos[i].msg[k].mensagem, "") != 0)
                        {
                            pthread_mutex_unlock(&ptra->mutex);
                            return; // ver se existe alguma mensagem no topico, se sim sai da func
                        }
                    }
                    for (int k = 0; k < MAX_USERS; k++)
                    {
                        if (strcmp(ptra->topicos[i].subscritores[k].username, "") != 0)
                        {
                            pthread_mutex_unlock(&ptra->mutex);
                            return; // se nao houver mensagens no topico mas ainda houver mais subscritores no mesmo, sai da func
                        }
                    }
                    strcpy(ptra->topicos[i].nome, ""); // se nao houver mensagens nem subs , apaga o topico
                    ptra->num_topics--;
                }
            }

            pthread_mutex_unlock(&ptra->mutex);
        }
    }
    else if (strcmp(pedido->comando, "msg") == 0)
    {

        if (ptra->num_topics < MAX_TOPICS)
        {
            if (topico_existe(pedido->topics.nome, &resp, ptra) == false)
            {

                adicionar_topico(pedido->topics.nome, ptra, &resp);
                printf("Tópico  %s criado com sucesso", pedido->topics.nome);
                strcpy(resp.comando, "info");
                strcpy(resp.msg[0].mensagem, "Tópico criado com sucesso");
                envia_resposta(&resp, 0, pedido->topics.nome, ptra, 0, 0);
            }

            int flag = 0;
            pthread_mutex_lock(&ptra->mutex);
            for (int i = 0; i < MAX_TOPICS; i++)
            {
                if (strcmp(ptra->topicos[i].nome, pedido->topics.nome) == 0)
                {
                    for (int j = 0; j < MAX_USERS; j++)
                    {
                        if (strcmp(ptra->topicos[i].subscritores[j].username, pedido->user.username) == 0)
                        {
                            flag = 1;
                            break;
                        }
                    }
                }
            }
            pthread_mutex_unlock(&ptra->mutex);
            if (flag == 0)
            {

                adicionar_subscricao(pedido->user.username, pedido->topics.nome, pedido, ptra, &resp);
                pthread_mutex_lock(&ptra->mutex);
                for (int l = 0; l < MAX_TOPICS; l++)
                {
                    if (strcmp(ptra->topicos[l].nome, pedido->topics.nome) == 0)
                    {
                        for (int k = 0; k < MAX_MSG_TOPIC; k++)
                        {
                            if (strcmp(ptra->topicos[l].msg[k].mensagem, "") != 0)
                            {
                                strcpy(resp.comando, "mensagem1");
                                strcpy(resp.msgUser, ptra->topicos[l].msg[k].username);
                                resp.user->pid = pedido->user.pid;
                                strcpy(resp.msg[1].mensagem, ptra->topicos[l].msg[k].mensagem);
                                envia_resposta(&resp, 0, "", ptra, 0, 0);
                            }
                        }
                    }
                }
                pthread_mutex_unlock(&ptra->mutex);
            }

            printf("Mensagem de [USER]: %d => %s %s %d\n", pedido->user.pid, pedido->topics.nome, pedido->msg.mensagem, pedido->msg.dura);
        }

        else
        {
            strcpy(resp.comando, "[ERRO]");
            strcpy(resp.msg[0].mensagem, "Número máximo de tópicos atingido.");
            pthread_mutex_lock(&ptra->mutex);
            envia_resposta(&resp, 0, 0, ptra, 0, 0);
            pthread_mutex_unlock(&ptra->mutex);
            printf("[Manager] Erro: Número máximo de tópicos atingido ao tentar enviar mensagem de %s.\n", pedido->user.username);
            return;
        }

        if (topico_bloqueado(pedido->topics.nome, &resp, ptra))
        {
            strcpy(resp.comando, "[ERRO]");
            strcpy(resp.msg[0].mensagem, " Não foi possível enviar mensagem pois o tópico está bloqueado.");
            pthread_mutex_lock(&ptra->mutex);
            envia_resposta(&resp, 0, 0, ptra, 0, 0);
            pthread_mutex_unlock(&ptra->mutex);
            printf("[Manager] Tópico %s está bloqueado. Mensagem de %s não foi enviada.\n", pedido->topics.nome, pedido->user.username);
            return;
        }

        // Envia a mensagem para todos os subscritos

        pthread_mutex_lock(&ptra->mutex);
        for (int i = 0; i < MAX_TOPICS; i++)
        {
            if (strcmp(ptra->topicos[i].nome, pedido->topics.nome) == 0)
            {

                RESPOSTA msg_resp;
                memset(&msg_resp, 0, sizeof(RESPOSTA));
                strcpy(msg_resp.comando, "mensagem");
                strcpy(msg_resp.topics[0].nome, pedido->topics.nome);
                strcpy(msg_resp.msg[0].mensagem, pedido->msg.mensagem);
                strcpy(msg_resp.msgUser, pedido->user.username);

                envia_resposta(&msg_resp, 1, pedido->topics.nome, ptra, 0, 0);
            }
        }
        pthread_mutex_unlock(&ptra->mutex);

        // Se for mensagem persistente, adiciona à lista
        if (pedido->msg.dura > 0)
        {

            pthread_mutex_lock(&ptra->mutex);
            for (int i = 0; i < MAX_TOPICS; i++)
            {
                if (strcmp(ptra->topicos[i].nome, pedido->topics.nome) == 0)
                {
                    for (int j = 0; j < MAX_MSG_TOPIC; j++)
                    {
                        if (strcmp(ptra->topicos[i].msg[j].mensagem, "") == 0)
                        {

                            strcpy(ptra->topicos[i].msg[j].mensagem, pedido->msg.mensagem);
                            strcpy(ptra->topicos[i].msg[j].topico, pedido->topics.nome);
                            strcpy(ptra->topicos[i].msg[j].username, pedido->user.username);
                            int len = strlen(pedido->msg.mensagem);
                            ptra->topicos[i].msg[j].mensagem[len] = '\0';
                            ptra->topicos[i].msg[j].dura = pedido->msg.dura;
                            ptra->topicos[i].numMensagem++;
                            break;
                        }
                    }
                }
            }
            pthread_mutex_unlock(&ptra->mutex);
            printf("[Manager] Mensagem persistente adicionada no tópico %s.\n", pedido->topics.nome);
        }
    }
    else if (strcmp(pedido->comando, "topics") == 0)
    {
        strcpy(resp.comando, "topics");

        char lista[TAM_MENSAGEM] = "";
        pthread_mutex_lock(&ptra->mutex);
        for (int i = 0; i < ptra->num_topics; i++)
        {

            strcpy(resp.topics[i].nome, ptra->topicos[i].nome);

            resp.topics[i].numMensagem = ptra->topicos[i].numMensagem;
            resp.topics[i].bloqueado = ptra->topicos[i].bloqueado;
            strcat(lista, resp.topics[i].nome);
            strcat(lista, resp.topics[i].bloqueado ? " (bloqueado)\n" : "\n");
        }

        strncpy(resp.msg[0].mensagem, lista, TAM_MENSAGEM - 1);

        envia_resposta(&resp, 0, 0, ptra, 0, 0);
        pthread_mutex_unlock(&ptra->mutex);
        printf("[Manager] Lista de tópicos enviada para o utilizador %s\n", pedido->user.username);
    }
    else
    {
        printf("[Manager] Comando desconhecido: %s\n", pedido->comando);
        strcpy(resp.comando, "[ERRO]");
        strcpy(resp.msg[0].mensagem, "Comando desconhecido.");

        envia_resposta(&resp, 0, 0, ptra, 0, 0);
    }
}

// Thread para comunicação com os clientes
void *thread_clientes_func(void *arg)
{
    PEDIDO pedido;
    ADMIN *ptra = (ADMIN *)arg;
    while (ptra->running) // le o que os clientes mandam, thread
    {

        int n = read(ptra->fd_server, &pedido, sizeof(PEDIDO));
        if (n == sizeof(PEDIDO))
        {
            processar_pedido(&pedido, ptra);
        }
        printf("[Admin]=>");
        fflush(stdout);
    }
    printf("\t\nThread Cliente a sair\n");
    pthread_exit(NULL);
    return NULL;
}
void inicializar_msgs(ADMIN *ptra, RESPOSTA *resp)
{
    const char *f = getenv("MSG_FICH");
    if (f == NULL)
    {
        printf("Não há variavel MSG_FICH definida\n");
    }
    FILE *temp = fopen(f, "r");
    if (temp == NULL)
    {
        printf("[ERRO AO ABRIR FICHEIRO]\n");
    }
    int size = 0;
    char testBuffer[300];
    while (fgets(testBuffer, sizeof(testBuffer), temp) != NULL)
    {
        size++;
    }
    fclose(temp);
    if (size == 0)
    {

        pthread_mutex_lock(&ptra->mutex);
        for (int i = 0; i < MAX_TOPICS; i++)
        {

            for (int j = 0; j < MAX_MSG_TOPIC; j++)
            {

                strcpy(ptra->topicos[i].msg[j].mensagem, "");
            }
        }
        pthread_mutex_unlock(&ptra->mutex);
        return;
    }

    FILE *file = fopen(f, "a+");
    if (f == NULL)
    {
        printf("[ERRO AO ABRIR FICHEIRO]\n");
    }
    char buffer[400];
    MENSAGEM msg[size];
    int i = 0;
    while (fgets(buffer, sizeof(buffer), file) != NULL)
    {

        buffer[strcspn(buffer, "\n")] = '\0'; // strcspn vai procurar o primeiro \n
        if (sscanf(buffer, "%s %s %d %[^\n]", msg[i].username, msg[i].topico, &msg[i].dura, msg[i].mensagem) != 4)
        {
            printf("Formato inválido \n");
        }

        i++;
    }
    int cycle = size - 1;
    pthread_mutex_lock(&ptra->mutex);
    for (int i = 0; i < MAX_TOPICS; i++)
    {

        for (int j = 0; j < MAX_MSG_TOPIC; j++)
        {

            strcpy(ptra->topicos[i].msg[j].mensagem, "");
        }
    }
    pthread_mutex_unlock(&ptra->mutex);
    do
    {

        if (!topico_existe(msg[cycle].username, NULL, ptra) == true)
        {

            adicionar_topico(msg[cycle].topico, ptra, resp);
        }

        pthread_mutex_lock(&ptra->mutex);
        for (int i = 0; i < MAX_TOPICS; i++)
        {
            if (strcmp(ptra->topicos[i].nome, msg[cycle].topico) == 0)
            {
                for (int j = 0; j < MAX_MSG_TOPIC; j++)
                {
                    if (strcmp(ptra->topicos[i].msg[j].mensagem, "") == 0)
                    {
                        strcpy(ptra->topicos[i].msg[j].mensagem, msg[cycle].mensagem);
                        strcpy(ptra->topicos[i].msg[j].topico, msg[cycle].topico);
                        strcpy(ptra->topicos[i].msg[j].username, msg[cycle].username);
                        ptra->topicos[i].msg[j].dura = msg[cycle].dura;
                        ptra->topicos[i].numMensagem++;
                        break;
                    }
                }
            }
        }
        pthread_mutex_unlock(&ptra->mutex);
        cycle--;
    } while (cycle != -1);
    fclose(file);
}

void escrever_ficheiro(ADMIN *ptra) // func para escrever no ficheiro as mensagems restantes

{

    const char *f = getenv("MSG_FICH");
    FILE *file = fopen(f, "w");

    if (f == NULL)
    {
        printf("[ERRO AO ABRIR FICHEIRO]\n");
    }

    pthread_mutex_lock(&ptra->mutex);
    for (int i = 0; i < MAX_TOPICS; i++)
    {

        for (int j = 0; j < MAX_MSG_TOPIC; j++)
        {
            if (strcmp(ptra->topicos[i].msg[j].mensagem, "") != 0)
            {
                fprintf(file, "%s %s %d %s\n", ptra->topicos[i].msg[j].username, ptra->topicos[i].msg[j].topico,
                        ptra->topicos[i].msg[j].dura, ptra->topicos[i].msg[j].mensagem);
            }
        }
    }
    pthread_mutex_unlock(&ptra->mutex);

    fclose(file);
}

void *thread_admin_func(void *arg) // Thread para interação com o administrador
{
    ADMIN *ptra = (ADMIN *)arg;

    for (int i = 0; i < MAX_USERS; i++)
    {
        strcpy(ptra->users[i], "");
    }

    RESPOSTA resp;
    char comando[TAM + 6];
    while (ptra->running)
    {
        printf("[Admin]=> ");
        fflush(stdout);
        fgets(comando, TAM + 6, stdin);
        comando[strcspn(comando, "\n")] = 0; // tira o \n
        printf("A processar comando '%s'\n", comando);

        if (strcmp(comando, "users") == 0)
        {

            printf("User ativos:\n");
            for (int i = 0; i < MAX_USERS; i++)
            {
                if (ptra->usersPID[i] != 0)
                    printf("- %s (PID: %d)\n", ptra->users[i], ptra->usersPID[i]);
            }
        }
        else if (strncmp(comando, "remove ", 7) == 0)
        {
            char username[TAM];
            sscanf(comando + 7, "%s", username);
            strcpy(resp.userExpulso, username);
            remover_user(username, &resp, ptra, 0, 0);
        }
        else if (strcmp(comando, "topics") == 0)
        {
            pthread_mutex_lock(&ptra->mutex);
            printf("Tópicos existentes:\n");
            for (int i = 0; i < ptra->num_topics; i++)
            {

                printf("- %s (%s) ( %d mensagens persistentes) \n", ptra->topicos[i].nome, ptra->topicos[i].bloqueado ? "bloqueado" : "desbloqueado", ptra->topicos[i].numMensagem);
            }
            pthread_mutex_unlock(&ptra->mutex);
        }
        else if (strncmp(comando, "show ", 5) == 0)
        {
            char nome_topico[TAM];
            sscanf(comando + 5, "%[^\n]", nome_topico);

            if (topico_existe(nome_topico, &resp, ptra))
            {
                printf("Mensagens do tópico %s:\n", nome_topico);
            }
            else
            {
                printf("Tópico não existe\n");
            }
            pthread_mutex_lock(&ptra->mutex);
            for (int i = 0; i < MAX_TOPICS; i++)
            {
                if (strcmp(ptra->topicos[i].nome, nome_topico) == 0)
                {
                    for (int j = 0; j < MAX_MSG_TOPIC; j++)
                    {
                        if (strcmp(ptra->topicos[i].msg[j].mensagem, "") != 0)
                            printf(" %s %s %s \n", ptra->topicos[i].msg[j].topico, ptra->topicos[i].msg[j].username, ptra->topicos[i].msg[j].mensagem);
                    }
                }
            }
            pthread_mutex_unlock(&ptra->mutex);
        }
        else if (strncmp(comando, "lock ", 5) == 0)
        {
            char nome_topico[TAM];
            sscanf(comando + 5, "%s", nome_topico);
            set_bloqueio_topico(nome_topico, true, &resp, ptra);
        }
        else if (strncmp(comando, "unlock ", 7) == 0)
        {
            char nome_topico[TAM];
            sscanf(comando + 7, "%s", nome_topico);
            set_bloqueio_topico(nome_topico, false, &resp, ptra);
        }
        else if (strcmp(comando, "close") == 0)
        {

            remover_user("", &resp, ptra, 1, 0);
            printf("Encerrando o sistema...\n");
            // unlink(FIFO_SERVER);
            ptra->running = false;
            escrever_ficheiro(ptra);
            int fd;
            fd = open(FIFO_SERVER, O_WRONLY);
            write(fd, "sai", 3);
            close(fd);

            break;
        }

        else
        {
            printf("Comando desconhecido.\n");
        }
    }
    printf("\t\nThread Admin a sair\n");
    pthread_exit(NULL);
    return NULL;
}

void *thread_tempo_func(void *arg)
{
    ADMIN *ptra = (ADMIN *)arg;
    while (ptra->running)
    {
        sleep(1); // Verifica a cada segundo
        pthread_mutex_lock(&ptra->mutex);
        for (int i = 0; i < MAX_TOPICS; i++)
        {
            for (int j = 0; j < MAX_MSG_TOPIC; j++)
            {
                if (strcmp(ptra->topicos[i].msg[j].mensagem, "") != 0)
                {
                    ptra->topicos[i].msg[j].dura--;
                    if (ptra->topicos[i].msg[j].dura <= 0)
                    {
                        strcpy(ptra->topicos[i].msg[j].mensagem, "");
                        strcpy(ptra->topicos[i].msg[j].topico, "");
                        strcpy(ptra->topicos[i].msg[j].username, "");
                        ptra->topicos[i].numMensagem--; // decrementa o num de mensagem
                        if (ptra->topicos[i].numMensagem < 0)
                        {
                            ptra->topicos[i].numMensagem = 0; // se for menor que 0, acerta para 0s
                        }
                    }
                }
            }
        }
        pthread_mutex_unlock(&ptra->mutex);
    }
    printf("\t\nThread Tempo a sair\n");
    pthread_exit(NULL);
    return NULL;
}
int main()
{
    ADMIN admin;
    // PASSAR A ESTRUTURA ADMIN
    if (mkfifo(FIFO_SERVER, 0666) == -1)
    {
        perror("Erro ao criar o FIFO do servidor");
        exit(1);
    }
    admin.fd_server = open(FIFO_SERVER, O_RDWR);
    if (admin.fd_server == -1)
    {
        perror("Erro ao abrir o FIFO do servidor");
        unlink(FIFO_SERVER);
        exit(1);
    }

    admin.running = true;
    RESPOSTA resp;

    admin.num_topics = 0;
    admin.num_mensagens = 0;
    admin.num_subscricoes = 0;

    inicializar_msgs(&admin, &resp);
    admin.num_users = 0;
    pthread_t thread_clientes, thread_admin, thread_tempo;
    pthread_mutex_init(&admin.mutex, NULL);
    if (pthread_create(&thread_clientes, NULL, thread_clientes_func, (void *)&admin) != 0)
    {
        perror("Erro ao criar a thread de clientes");
        close(admin.fd_server);
        unlink(FIFO_SERVER);
        pthread_exit(NULL);
    }
    if (pthread_create(&thread_admin, NULL, thread_admin_func, (void *)&admin) != 0)
    {
        perror("Erro ao criar a thread do administrador");
        admin.running = false;
        pthread_join(thread_clientes, NULL);
        close(admin.fd_server);
        unlink(FIFO_SERVER);
        pthread_exit(NULL);
    }
    if (pthread_create(&thread_tempo, NULL, thread_tempo_func, (void *)&admin) != 0)
    {
        perror("Erro ao criar a thread de tempo");
        admin.running = false;
        pthread_join(thread_clientes, NULL);
        pthread_join(thread_admin, NULL);
        close(admin.fd_server);
        unlink(FIFO_SERVER);
        pthread_exit(NULL);
    }
    pthread_join(thread_clientes, NULL);
    pthread_join(thread_admin, NULL);
    pthread_join(thread_tempo, NULL);

    pthread_mutex_destroy(&admin.mutex);
    close(admin.fd_server);
    unlink(FIFO_SERVER);
    return 0;
}