# Fórum de Tópicos em Linux (IPC com FIFOs)

Trabalho prático da disciplina de Sistemas Operativos (2024/2025), desenvolvido em grupo por Tiago Manata e Guilherme Neves.

## Descrição

Sistema de fórum em modelo cliente-servidor que corre em Linux, composto por dois programas:

- **`manager`** — servidor central. Gere utilizadores, tópicos, subscrições e mensagens, e disponibiliza uma consola de administração.
- **`feed`** — cliente. Permite a um utilizador registar-se, criar/subscrever tópicos e publicar mensagens que chegam a todos os subscritores desse tópico.

A comunicação entre processos é feita através de **named pipes (FIFOs)**: um FIFO principal do servidor para receber pedidos, e um FIFO por cliente (identificado pelo PID) para receber respostas. O servidor usa threads para atender clientes, processar comandos de administração e gerir a expiração de mensagens, sincronizadas com mutexes.

## Funcionalidades

- Registo de utilizadores com nome único
- Criação e listagem de tópicos
- Subscrição e cancelamento de subscrição a tópicos
- Publicação de mensagens num tópico, com duração (TTL) em segundos — a mensagem expira automaticamente e deixa de ser visível
- Persistência de mensagens ainda válidas para ficheiro em disco ao encerrar o servidor, e reposição ao arrancar
- Consola de administração (no processo `manager`) com comandos para:
  - listar utilizadores ligados (`users`)
  - remover um utilizador (`remove <username>`)
  - listar tópicos (`topics`)
  - ver as mensagens de um tópico (`show <topico>`)
  - bloquear/desbloquear um tópico (`lock` / `unlock <topico>`)
  - encerrar o servidor de forma limpa (`close`)
- Notificação dos clientes quando um utilizador é removido, sai, ou quando um tópico é bloqueado/desbloqueado

## Tecnologias

- **Linguagem:** C
- **Sistema operativo:** Linux
- **Mecanismos de SO utilizados:**
  - Named pipes / FIFOs (`mkfifo`, `open`, `read`, `write`) para comunicação entre processos
  - Multithreading com `pthread` (threads de input, de receção de mensagens, de administração e de gestão de tempo/expiração)
  - Mutexes (`pthread_mutex_t`) para proteger a estrutura de dados partilhada entre threads
  - Sinais (`SIGUSR1`) para interromper threads bloqueadas em leitura
- **Build:** `make` / `gcc`

## Instalação e como correr

Pré-requisitos: Linux (ou WSL), `gcc` e `make` instalados.

```bash
# 1. Compilar
cd so_2425_tp_GuilhermeNeves_2023111980_TiagoManata_2023147352
make

# 2. Definir o ficheiro usado para persistência de mensagens
export MSG_FICH=msg.txt

# 3. Arrancar o servidor (num terminal)
./manager

# 4. Ligar um ou mais clientes (num ou mais terminais diferentes)
./feed <username>
```

Dentro do cliente (`feed`), usar o comando `comandos` para ver a lista de comandos disponíveis. Alguns exemplos:

```
subscribe geral
msg geral 60 Olá pessoal!
topics
unsubscribe geral
exit
```

No terminal do `manager`, os comandos de administração (`users`, `topics`, `show <topico>`, `lock <topico>`, `unlock <topico>`, `remove <username>`, `close`) podem ser escritos diretamente.

> Nota: o `makefile` inclui um alvo `broker` com o nome de output `manager` — se `make` não gerar o executável `manager`, compilar manualmente com `gcc manager.c -o manager -lpthread` (e o mesmo para `feed.c`).

## O que aprendi

- Comunicação entre processos em Linux usando named pipes, incluindo os cuidados a ter com leitura/escrita bloqueante
- Programação concorrente com `pthread`, e a necessidade de proteger dados partilhados com mutexes para evitar condições de corrida
- Desenho de um protocolo próprio de mensagens entre cliente e servidor (structs partilhadas via `util.h`)
- Gestão do ciclo de vida de processos e recursos do SO (FIFOs, file descriptors) — e a importância de os libertar corretamente
- Uso de sinais para desbloquear threads presas em chamadas de leitura, ao encerrar o cliente

## Melhorias futuras

- Substituir os limites fixos (`MAX_USERS`, `MAX_TOPICS`, `MAX_MSG_TOPIC`) por estruturas dinâmicas
- Reforçar a validação de input do lado do cliente (ex: `feed.c` assume sempre 3 argumentos após o comando `msg`, o que pode causar acesso a memória inválida se o comando vier incompleto)
- Melhorar o tratamento de erros nas operações de I/O (várias chamadas a `open`/`write`/`read` não verificam o valor de retorno)
- Substituir o buffer de leitura por uma verificação mais robusta no `thread_receive_f` (o `read` em loop não trata bem o caso de erro ou de ligação fechada)
- Adicionar autenticação simples de utilizadores, em vez de apenas verificar unicidade do nome
- Escrever testes automatizados para os cenários de concorrência (múltiplos clientes a publicar/subscrever em simultâneo)
- Documentar o protocolo de comandos (`PEDIDO`/`RESPOSTA`) num ficheiro separado, para facilitar a leitura do código
