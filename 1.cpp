#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <pthread.h>

#define max_threads 10

pthread_mutex_t lock;// aqui inicializei o mutex que será essencial para a execução correta do algoritmo

static std::string text;// text será a senha encontrada, inicializada estaticamente
static int overflow;// overflow é uma variável auxiliar para caso a senha tenha mais de 10 caracteres

struct Data{// estrutura da thread, no caso base cada thread é responsável por um caractere
    int curr_char;
};

void* find(void* arg) { //função de busca por força bruta, onde a cada caractere encontrado a string text é atualizada
    pthread_mutex_lock(&lock);//exclusão mútua

    Data* curr_thread = (Data*)arg;

    for (int i = 0; i < 128; ++i) {
        if (i == curr_thread->curr_char) {
            char c = (char)curr_thread->curr_char;
            text.append(1, c);
        }
    }
    pthread_mutex_unlock(&lock);
    return NULL;
}

int main() {
    std::string password;
    
    std::cin >> password;

    int size = password.size();
    overflow = 1;// por padrão overflow é inicializado com 1, isto é, por padrão cada thread irá trabalhar em 1 caractere
    pthread_t thread[max_threads];

    if (size > max_threads) {// essa condição será verdadeira apenas quando a senha tiver mais de 10 caracteres
        int ceiling = 1 + ((size - 1) / max_threads);//aqui é calculado o teto da divisão do tamanho da senha por 10
        if (ceiling * max_threads > size) {//se o teto vezes 10 for maior que o tamanho da senha atual significa que teremos que modificar a senha de forma que cada thread trabalhe com caracteres igualmente distribuídos 
            int aux = (ceiling * max_threads) - size;// aux significa quantos caracteres a mais terei que adicionar à senha
            password.append(aux, 128);// adicionando caracteres "dummy"
            overflow = ceiling;// overflow agora indica quantos caracteres cada thread terá que trabalhar
        }
    }

    std::cout << password << std::endl;

    Data data[max_threads];
    
    for (int i = 0; i < max_threads; ++i) { //cada thread é responsével por n char = overflow
        for (int j = 0; j < overflow; ++j) {// aqui estou iterando a senha overflow vezes para depois iterar a thread
            data[i].curr_char = (int)password[j];
            pthread_create(&thread[i], NULL, find, &data[i]);
            pthread_join(thread[i], NULL);
        }
        password.erase(0, overflow);// os caracteres que já trabalhei são deletados da senha
    }
    std::cout << text << std::endl;// senha final encontrada 
    return 0;
}
