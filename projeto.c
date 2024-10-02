#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdbool.h>

#define NUM_CADEIRAS 5
#define NUM_BARBEIROS 3
#define NUM_CLIENTES 10

pthread_mutex_t mutex;
sem_t semaforo_cadeiras;
sem_t semaforo_barbeiros;

/* Struct que simula a fila para colocar a ordem de atendimento
 A fila é ordenada pelo fato de sabermos o primeiro e ultimo tempo 
porem a fila para nao ser necessario ficar reordenando assim que chegamos no ultimo espaço voltamos
para o primeiro no caso clientes[NUM_CADEIRAS];
 */
typedef struct {
    int clientes[NUM_CADEIRAS];
    int frente;
    int tras;
} Fila;
// cria a variavel do tipo Fila global
Fila fila;
// função para iniciar a fila sem termos
void inicializa_fila() {
    fila.frente = 0;
    fila.tras = 0;
}
// função para adicionar um cliente na posição de tras  fila.clientes[fila.tras] e mudar o escopo do final da fila
void adiciona_fila(int cliente) {
    fila.clientes[fila.tras] = cliente;
    fila.tras = (fila.tras + 1) % NUM_CADEIRAS;
}
// função para retirar um cliente na posição de frente  fila.clientes[fila.frente] e mudar o escopo da frente da fila
int retira_fila() {
    int cliente = fila.clientes[fila.frente];
    fila.frente = (fila.frente + 1) % NUM_CADEIRAS;
    return cliente;
}
// funçao que retorna 1 se fila estiver vazia no caso frente igual a tras da fila
int fila_vazia() {
    return fila.frente == fila.tras;
}
// função thread para chegada de clientes
void* chegada_cliente(void* arg) {
    // variavel do numero do cliente
    int id_cliente = *(int*)arg;

    printf("Cliente %d chegou.\n", id_cliente);

    // Tenta ocupar uma cadeira
    // no caso de ter vaga sem_trywait retorna 0 e ja decrementa o semaforo de contagem 

    if (sem_trywait(&semaforo_cadeiras) == 0) {
        // mutex para proteguer dados de cadeira de espera

        pthread_mutex_lock(&mutex);
        //acessa o semaforo de cadeiras para atribuir a uma variavel quantas disponiveis existir

        int cadeiras_disponiveis;
        sem_getvalue(&semaforo_cadeiras, &cadeiras_disponiveis);
        //função da fila para adicionar cliente

        adiciona_fila(id_cliente);
        
        printf("Cliente %d sentou na cadeira. Cadeiras disponíveis: %d\n", id_cliente, cadeiras_disponiveis);
        // final do mutex

        pthread_mutex_unlock(&mutex);
        // Acorda um barbeiro com o semaforo

        sem_post(&semaforo_barbeiros);

        // caso de não ter espaço na fila 

    } else {
     
        printf("Cliente %d foi embora pois nao tinha cadeiras disponíveis.\n", id_cliente);
    }

    return NULL;
}

// função thread para funcionamento de cada barbeiro

void* barbeiro(void* arg) {
    int id_barbeiro = *(int*)arg;
    
    bool barbeiro_dormindo = true;
    while (1) {
        // Espera até o semaforo dos barbeiros serem incrementado por um cliente 
        //ou seja  ser acordado por um cliente
        sem_wait(&semaforo_barbeiros);
        //inicia a variavel que vai ter informaçao de qual cliente é atendido
        int cliente_atendido; // 
        //mutex para proteger a fila
        pthread_mutex_lock(&mutex);
        // if para ver se estava ou não durmindo e ver se a fila esta vazia 
        if (!fila_vazia()&& barbeiro_dormindo) {
            // retira da fila o numero do cliente atendido
            cliente_atendido = retira_fila();
            printf("Barbeiro %d acordou e está atendendo o cliente %d.\n", id_barbeiro, cliente_atendido);
            pthread_mutex_unlock(&mutex);
            //simula um tempo de corte de cabelo
            sleep(rand() % 5);
            printf("Barbeiro %d terminou de atender o cliente %d.\n", id_barbeiro, cliente_atendido);
            barbeiro_dormindo = false;
            //caso de não ter cliente esperando o barbeiro dorme
            if (fila_vazia()) {
                printf("Barbeiro %d esta durmindo não tem clientes \n", id_barbeiro);
                barbeiro_dormindo = true;
            }
        }else if (!fila_vazia()&& !barbeiro_dormindo){
            cliente_atendido = retira_fila();
            printf("Barbeiro %d está atendendo o cliente %d.\n", id_barbeiro, cliente_atendido);
            pthread_mutex_unlock(&mutex);
            sleep(rand() % 5);
            printf("Barbeiro %d terminou de atender o cliente %d.\n", id_barbeiro, cliente_atendido);
            barbeiro_dormindo = false;
            if (fila_vazia()) {
                printf("Barbeiro %d esta durmindo não tem clientes \n", id_barbeiro);
                barbeiro_dormindo = true;
            }
        }

        // Libera a cadeira
        sem_post(&semaforo_cadeiras);
    }

    return NULL;
}


int main() {
    pthread_t thread_id_clientes[NUM_CLIENTES];
    pthread_t thread_id_barbeiros[NUM_BARBEIROS];
    int ids_clientes[NUM_CLIENTES];
    int ids_barbeiros[NUM_BARBEIROS];

    // Inicializa os semáforos e mutex
    pthread_mutex_init(&mutex, NULL);
    sem_init(&semaforo_cadeiras, 0, NUM_CADEIRAS); // Número de cadeiras disponíveis
    sem_init(&semaforo_barbeiros, 0, 0);           // Inicialmente, nenhum barbeiro acordado

    // Inicializa a fila
    inicializa_fila();

    // Cria os barbeiros
    for (int i = 0; i < NUM_BARBEIROS; i++) {
        ids_barbeiros[i] = i + 1;
         // Numeração dos barbeiros começa em 1
        pthread_create(&thread_id_barbeiros[i], NULL, barbeiro, &ids_barbeiros[i]);
    }

    // Cria os clientes
    for (int i = 0; i < NUM_CLIENTES; i++) {
        ids_clientes[i] = i + 1; 
        // Numeração dos clientes começa em 1
        pthread_create(&thread_id_clientes[i], NULL, chegada_cliente, &ids_clientes[i]);
        sleep(rand() % 2); // Simula a chegada dos clientes em intervalos aleatórios
    }

    
    // Aguarda todos os clientes terminarem
    for (int i = 0; i < NUM_CLIENTES; i++) {
        pthread_join(thread_id_clientes[i], NULL);
    }

     int cadeiras_disponiveis;
    sem_getvalue(&semaforo_cadeiras, &cadeiras_disponiveis);
    // Aguarda todos os barbeiros pararem de atender
    while(cadeiras_disponiveis!=5){
        sem_getvalue(&semaforo_cadeiras, &cadeiras_disponiveis);
    }
    // termina as threads dos barbeiros
    for (int i = 0; i < NUM_BARBEIROS; i++) {
         pthread_cancel(thread_id_barbeiros[i]);
    
    }

     

    printf("Fim do expediente.\n");

    return 0;
}
