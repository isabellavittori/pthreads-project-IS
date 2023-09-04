#include <iostream>
#include <vector>
#include <algorithm>
#include <pthread.h>

pthread_mutex_t lock;
int number_of_islands = 0;

class DisjointUnionSets {//aqui é a classe dos conjuntos disjuntos, onde irei criar o mapa e procurar pelas ilhas
	
	std::vector<int> rank, parent;
	int n;
	
	public:
	DisjointUnionSets(int n) {
		rank.resize(n);
		parent.resize(n);
		this->n = n;
		makeSet();
	}

	void makeSet() {
	    for (int i = 0; i < n; i++) {
            parent[i] = i;
        }
	}

	int find(int x) {
    	if (parent[x] != x) {
			parent[x] = find(parent[x]);
		}
		return parent[x];
	}

	void Union(int x, int y) {//o union é responsável por unir os pedaços de terra por meio do link correto entre pedaço de terra(filho) e ilha(pai)
		int xRoot = find(x);
		int yRoot = find(y);

		if (xRoot == yRoot) return;

		if (rank[xRoot] < rank[yRoot]) {
            parent[xRoot] = yRoot;
        } 

		else if (rank[yRoot] < rank[xRoot]) {
            parent[yRoot] = xRoot;
        } 
		else {
			parent[yRoot] = xRoot;
			rank[xRoot] = rank[xRoot] + 1;
		}
	}
};

struct Args {//argumentos das threads, aqui pode-se modificar a matriz conforme quiser
    std::vector<std::vector<int>>a = {{1, 1, 0, 0, 0},
							          {0, 1, 0, 0, 1},
							          {1, 0, 0, 1, 1},
							          {0, 0, 0, 0, 0},
							          {1, 0, 1, 0, 1}};

    DisjointUnionSets* dus;
};

void* makeUnion(void *arg) {// essa função é responsável por procurar os pedaçoes de terra e os unir em ilhas
	pthread_mutex_lock (&lock);

    Args* curr_thread = (Args*) arg;

    int n = curr_thread->a.size(); //linha
	int m = curr_thread->a[0].size(); //coluna

    curr_thread->dus = new DisjointUnionSets(n * m);

	for (int j = 0; j < n; j++) {//thread buscando pelas terras
		for (int k = 0; k < m; k++) {
			if (curr_thread->a[j][k] == 0) {continue;}

			if (j + 1 < n && curr_thread->a[j + 1][k] == 1)
				curr_thread->dus->Union(j * (m) + k, (j + 1) * (m) + k);

			if (j - 1 >= 0 && curr_thread->a[j - 1][k] == 1)
				curr_thread->dus->Union(j * (m) + k, (j - 1) * (m) + k);

     		if (k + 1 < m && curr_thread->a[j][k + 1] == 1)
				curr_thread->dus->Union(j * (m) + k, (j) * (m) + k + 1);

			if (k - 1 >= 0 && curr_thread->a[j][k - 1] == 1)
				curr_thread->dus->Union(j * (m) + k, (j) * (m) + k - 1);

			if (j + 1 < n && k + 1 < m && curr_thread->a[j + 1][k + 1] == 1)
				curr_thread->dus->Union(j * (m) + k, (j + 1) * (m) + k + 1);

			if (j + 1 < n && k - 1 >= 0 && curr_thread->a[j + 1][k - 1] == 1)
				curr_thread->dus->Union(j * m + k, (j + 1) * (m) + k - 1);

			if (j - 1 >= 0 && k + 1 < m && curr_thread->a[j - 1][k + 1] == 1)
				curr_thread->dus->Union(j * m + k, (j - 1) * m + k + 1);

			if (j - 1 >= 0 && k - 1 >= 0 && curr_thread->a[j - 1][k - 1] == 1)
				curr_thread->dus->Union(j * m + k, (j - 1) * m + k - 1);
		}
	}
	
    pthread_mutex_unlock (&lock);
	return NULL;
}

void* countIslands(void* arg) {//essa função irá contar quantas ilhas existem de acordo com a ligação entre os filhos e pais
    pthread_mutex_lock (&lock);

   

    
        Args* curr_thread = (Args*) arg;

        int n = curr_thread->a.size(); //linha
	    int m = curr_thread->a[0].size(); //coluna

        int *c = new int[n * m];
    
        for (int j = 0; j < n; ++j) {
            for (int k = 0; k < m; ++k) {
                if (curr_thread->a[j][k] == 1) {
                    int x = curr_thread->dus->find(j * m + k);
                    if (c[x] == 0) {
                        number_of_islands++;
                        c[x]++;
                    }
                    c[x]++;
                }
            }
        }
      
   
    pthread_mutex_unlock(&lock);
    return NULL;
}

int main() {  
    pthread_t thread[2];
    Args args;
 
    pthread_create(&thread[0], NULL, makeUnion, &args);
    pthread_join(thread[0], NULL);

    pthread_create(&thread[1], NULL, countIslands, &args);
    pthread_join(thread[1], NULL);
    
  
    std::cout << number_of_islands << std::endl;
        
    return 0;
}
