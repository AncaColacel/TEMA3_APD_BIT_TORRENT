#include <mpi.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define TRACKER_RANK 0
#define MAX_FILES 10
#define MAX_FILENAME 15
#define HASH_SIZE 32
#define MAX_CHUNKS 100
#define MAX_FILENAME_LENGTH 100
#define MAX_SEGMENTS 100


/*
structura aferenta unui fisier cu:
1) numele fisierului
2) numarul de segmente
3) matrice de segmente (hashurile)
*/ 
typedef struct {
    char filename[MAX_FILENAME_LENGTH];
    int num_segments;
    char segments[MAX_SEGMENTS][MAX_FILENAME_LENGTH];
} FileInfo;

/*
structura aferenta unui client
1) fisiere detinute
2) vector de fisiere
3) nr fisiere dorite
4) nume fisiere dorite
*/
typedef struct {
    char name_peer[20];
    int num_owned_files;
    FileInfo owned_files[MAX_SEGMENTS];
    int num_desired_files;
    char desired_files[MAX_SEGMENTS][MAX_FILENAME_LENGTH];
} ClientInfo;


typedef struct  {
    FileInfo fisier;
    ClientInfo client_peer;
} MyMap;



void *download_thread_func(void *arg)
{
    int rank = *(int*) arg;

    return NULL;
}

void *upload_thread_func(void *arg)
{
    int rank = *(int*) arg;

    return NULL;
}


void readClientInfoFromFile(FILE *file, ClientInfo *client) {
    // Citirea numărului de fișiere deținute
    fscanf(file, "%d", &(client->num_owned_files));

    // Citirea informațiilor despre fiecare fișier deținut
    for (int i = 0; i < client->num_owned_files; ++i) {
        // Citirea numelui fișierului
        fscanf(file, "%s", client->owned_files[i].filename);

        // Citirea numărului de segmente ale fișierului
        fscanf(file, "%d", &(client->owned_files[i].num_segments));

        // Citirea segmentelor
        for (int j = 0; j < client->owned_files[i].num_segments; ++j) {
            fscanf(file, "%s", client->owned_files[i].segments[j]);
        }
    }

    // Citirea numărului de fișiere dorite
    fscanf(file, "%d", &(client->num_desired_files));

    // Citirea numelor fișierelor dorite
    for (int i = 0; i < client->num_desired_files; ++i) {
        fscanf(file, "%s", client->desired_files[i]);
    }

    
    
}



void tracker(int numtasks, int rank, MyMap map[], ClientInfo clients[]) {
    //creare swarm de fisiere
    //test 1 are 3 fisiere => fiecare are cate un SWARM (hash cu fisier -> client)
    /*
    Map[0] - File: file1, Peer: client_1
    Map[1] - File: file2, Peer: client_1
    Map[2] - File: file3, Peer: client_2
    */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < clients[i].num_owned_files; j++) {
            if (strcmp(clients[i].owned_files[j].filename, "file1") == 0) {
                map[0].fisier = clients[i].owned_files[j];
                map[0].client_peer = clients[i];
                
            }
            else if (strcmp(clients[i].owned_files[j].filename, "file2") == 0) {
                map[1].fisier = clients[i].owned_files[j];
                map[1].client_peer = clients[i];
            }
            else if (strcmp(clients[i].owned_files[j].filename, "file3") == 0) {
                map[2].fisier = clients[i].owned_files[j];
                map[2].client_peer = clients[i];
                
            }
                
        }
    }

    

}

void peer(int numtasks, int rank) {
    
    // parte de schelet
    pthread_t download_thread;
    pthread_t upload_thread;
    void *status;
    int r;

    r = pthread_create(&download_thread, NULL, download_thread_func, (void *) &rank);
    if (r) {
        printf("Eroare la crearea thread-ului de download\n");
        exit(-1);
    }

    r = pthread_create(&upload_thread, NULL, upload_thread_func, (void *) &rank);
    if (r) {
        printf("Eroare la crearea thread-ului de upload\n");
        exit(-1);
    }

    r = pthread_join(download_thread, &status);
    if (r) {
        printf("Eroare la asteptarea thread-ului de download\n");
        exit(-1);
    }

    r = pthread_join(upload_thread, &status);
    if (r) {
        printf("Eroare la asteptarea thread-ului de upload\n");
        exit(-1);
    }
}
 
int main (int argc, char *argv[]) {

    // Citirea informațiilor pentru fiecare client
    // fiecare client este preluat de catre peerul avand rankul corespunzator
    for (int i = 0; i < 3; ++i) {
        char filename[20];
        sprintf(filename,"test1/in%d.txt", i + 1);
        sprintf(clients[i].name_peer, "client_%d", i + 1);
        
        FILE *file = fopen(filename, "r");
        if (file == NULL) {
            perror("Eroare la deschiderea fisierului de intrare");
            exit(-1);
        }
        
        readClientInfoFromFile(file, &clients[i]);
        fclose(file);
    }

    // Inițializarea vectorului de structuri ClientInfo pt test 1
    ClientInfo clients[3];
    MyMap map[3];
    
    // partea de schelet
    int numtasks, rank;
 
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    if (provided < MPI_THREAD_MULTIPLE) {
        fprintf(stderr, "MPI nu are suport pentru multi-threading\n");
        exit(-1);
    }
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == TRACKER_RANK) {
        tracker(numtasks, rank, map, clients);
    } else {
        peer(numtasks, rank);
    }

    MPI_Finalize();
}
