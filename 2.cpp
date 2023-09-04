#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <pthread.h>
#include <fstream>

pthread_mutex_t lock;

//esse algoritmo utiliza de um arquivo com exemplos de entrada, o nome do arquivo está como "exemplo_q2.txt"

struct Graph{ //utilizei essa estrutura para a construção de todo o grafo
    int thread_number;
    std::vector<std::pair<std::string, bool>> visited; //aqui é um par nó-booleano que checa se determinado nó foi visitado ou não
    std::vector<std::string> resource; //aqui são os nós recursos
    std::vector<std::string> process; //nós processos
    std::vector<std::pair<std::string, std::string>> connection; //conexões entre recursos e processos
};

void parse(Graph* g, std::string filename) { //essa função é responsável por construir todo o grafo
    std::ifstream file;
    std::string hold;
    int n;
    file.open("/home/isabella/Documentos/Isa Visual Studio/threads/exemplo.txt");
    
    file >> n; //lendo a primeira variável do arquivo, correspondente ao número de recursos

    for (int i = 0; i < n; ++i) {
        file >> hold; //lendo os recursos
        g->resource.push_back(hold); //adicionando os recursos ao grafo
    }

    file >> n; //lendo o número de processos

    for (int i = 0; i < n; ++i) {
        std::string prc_name;
        int size;
        file >> prc_name; //nome do primeiro processo

        g->process.push_back(prc_name); //adicionando o processo ao grafo

        file >> size; //lendo o número de recursos que esse processo alocou 

        for (int j = 0; j < size; ++j) {
            file >> hold; //lendo o recurso alocado
            std::pair<std::string, std::string> p = std::make_pair(hold, prc_name);// aloca recurso
            g->connection.push_back(p); //realizando a conexão recurso processo
        }

        file >> size; //lendo o número de recursos que o processo pede

        for (int j = 0; j < size; ++j) {
            file >> hold; //lendo o recurso requisitado
            std::pair<std::string, std::string> p = std::make_pair(prc_name, hold);// pede recurso
            g->connection.push_back(p); //realizando a conexão processo recurso
        }
    }

    for (auto res : g->resource) {
        std::pair<std::string, bool> p = std::make_pair(res, false); //aqui estou marcando todos os recursos como não visitados
        g->visited.push_back(p);
    }

    for (auto prc : g->process) {
        std::pair<std::string, bool> p = std::make_pair(prc, false); //aqui estou marcando todos os processos como não visitados
        g->visited.push_back(p);
    }

    file.close();
}

void resetVisit(Graph* g) { //essa função é apenas para ter certeza de antes de iniciar a busca, todos os nós não foram visitados
    for (auto a : g->visited) {
        a.second = false;
    }
}

bool isCycle(Graph* g, std::string node, std::vector<std::string> parents) { //aqui começa a busca, eu recebo o grafo, o nó atual, e os pais
    std::vector<std::string> adj;
    bool status = false; //o status mantém o tracking de quando devo parar de buscar em determinado nó
    bool search = false; //aqui é o resultado da busca, ou seja, se o nó que estou visitando atualmente na verdade é o seu próprio pai, i.e. deadlock

    for (auto a : parents) {
        if (a == node) {
            search = true;
            break;
        }
    }

    for (auto n : g->visited) {
        if (n.first == node) {
            if (n.second || search) return true;
        }
    }
    
    for (auto n : g->visited) {
        if (n.first == node) n.second = true;
    }

    parents.push_back(node);

    for (auto n : g->connection) {
        if (n.first == node) {
            adj.push_back(n.second);
        }
    }

    if (adj.size() > 0) {
        for (auto n : adj) {
            status = status || isCycle(g, n, parents);
        }
    }
    return status;
}

void* isCycleExist(void* arg) { //essa é uma função auxiliar onde as threads trabalham, é uma thread por processo
    pthread_mutex_lock(&lock);
    std::vector<std::string> parents;

    Graph* curr_thread = (Graph*)arg;

    resetVisit(curr_thread);

    if (isCycle(curr_thread, curr_thread->process[curr_thread->thread_number], parents)) { //a cada processo visitado a thread irá analisar se há risco de deadlock
        std::cout << "Risco de deadlock" << std::endl;
    }
    else {
        std::cout << "Não há risco de deadlock" << std::endl;
    }

    pthread_mutex_unlock(&lock);
    return NULL;
}

int main() {
    Graph graph;
    std::string filename;

    std::cin >> filename;
    parse(&graph, filename);
    int size = graph.process.size();

    pthread_t thread[size];

    for (int i = 0; i < size; ++i) {
        graph.thread_number = i;
        pthread_create(&thread[i], NULL, isCycleExist, &graph);
        pthread_join(thread[i], NULL);
    }

    return 0;
}
