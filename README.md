# Projeto-1-Salao-de-Barbeiros

Projeto tinha como objetivo sincronizando as ações dos barbeiros e clientes conforme descrito fazendo uso adequado de semáforos e mutexes.


Foram implementados semaforos de numero de cadeiras que tinham como objetivo controlar para apenas 5 cliente serem preenchidos nas cadeiras por vez.

Foram implementados semaforos de barbeiros para ter a informação de quando um barbeiro foi acordado/solicitado.

Foi implementado uma struct Fila com objetivo de simular a fila de clientes que ficavam esperando .

Foram implementadas tbm funções para uso dessa fila como inicializa fila ,retira cliente da fila ,adiciona cliente na fila , e check se a fila estar vazia .

Foi implementado função da thread de chegada de clientes onde ja informava a chegada do cliente e em seguida se possivel colocado na fila e adiciona no semaforo de barbeiro uma solicitação de ser atendido caso não for possivel cliente ia embora.

Foi implementado um função de thread de barbeiro onde o barbeiro ficava esperando o semaforo ter uma solicitação para ser atendido e assim começava o atendimento e quando terminava se não tem fila o barbeiro dorme.

Na main foi inicializado os semaforos e as threads dos barbeiros logo em seguida é inicializado as threads das chegadas dos cliente e depois é esperado todos serem atendidos.

gcc -o projeto projeto.c -lpthread
