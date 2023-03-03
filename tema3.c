#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int numtasks, rank;
    int num_worker;
    int topo[4][12]; // matrice cu nr.linii = nr.coordonatori, nr.col = nr.workers
    int topo_recieved[4][12];
    int i, j;

    for (i = 0; i < 4; i++)  {
        for(j = 0; j < 12; j++) {
            topo[i][j] = -1;
        }
    }
                
    for (i = 0; i < 4; i++)  {
        for(j = 0; j < 12; j++) {
            topo_recieved[i][j] = -1;
        }
    }

    char file_name[100]; 
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int gata;
    if (rank == 0) {
        sprintf(file_name, "cluster%d.txt",rank);
        FILE *f = fopen(file_name, "rt");
        if (f == NULL)  MPI_Finalize();
    
        // citesc prima linie din fisier
	    fscanf(f, "%d", &num_worker);

        for (j = 0; j < 12; j++) {  
            topo[rank][j] =-1;
        }

        for (j = 0; j < 12; j++) {  
            topo_recieved[rank][j] =-1;
        }

	    for (int i = 0; i < num_worker; i++){
		    fscanf(f, "%d", &topo[rank][i]);
        }

        fclose(f);

        MPI_Send(topo, 48, MPI_INT, 3, 0, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 3);
        MPI_Status status;
        MPI_Recv(topo_recieved, 48, MPI_INT, 3, 0, MPI_COMM_WORLD, &status);
        for(i = 0; i < 4; i++)  {
            for(j = 0; j < 12; j++)   {
                if (topo[i][j] != topo_recieved[i][j] && topo[i][j] == -1) topo[i][j] = topo_recieved[i][j];
            }
        }

        gata = 0;
        for(i = 0; i < 4; i++)  {
            if(topo[i][0] == -1) {
                gata = 1;
                break;
            }
        }

        if (gata == 0) {
            printf("%d -> ", rank);
            for (i = 0; i < 4; i++) {
                printf("%d:",i);
                for (j = 0; j < 12 && topo[i][j] != -1; j++) {
                    printf("%d", topo[i][j]);
                    if (topo[i][j + 1] != -1) {
                        printf(",");
                    }
                }
                printf(" ");
            }
            printf("\n");
            for (i = 0; i < num_worker; ++i) {
                MPI_Send(topo, 48, MPI_INT, topo[rank][i], 0, MPI_COMM_WORLD);
                printf("M(%d,%d)\n", rank, topo[rank][i]);
            }
        }
        
        // task 2
        int N = atoi(argv[1]); // dim. vectorului
        int V[N]; // declarare vector de N
        for (i = 0; i < N; i++) V[i] = N - i - 1;
        int nr_workers = numtasks - 4;
        int calcs = N / nr_workers + 1;
        int count_dist[nr_workers];
        int used_calcs = 0;
        
        for (i = 0; i < nr_workers && used_calcs + calcs < N; i++) {
            count_dist[i] = calcs;
            used_calcs += calcs;
        }

        count_dist[i] = N - used_calcs;
        i++;
        while (i < nr_workers) {
            count_dist[i] = 0;
            i++;
        }

        for (i = nr_workers - 1; count_dist[i] == 0; i--) {
            count_dist[i]++;
            count_dist[nr_workers - i - 1]--;
        }

        int index_dist[nr_workers];
        index_dist[0] = 0;
        for (i = 1; i < nr_workers; i++) {
            index_dist[i] = index_dist[i - 1] + count_dist[i - 1];
        }

        for (i = 0; topo[0][i] != -1; i++) {
            MPI_Send(&N, 1, MPI_INT, topo[0][i], 0, MPI_COMM_WORLD);
            MPI_Send(V, N, MPI_INT, topo[0][i], 0, MPI_COMM_WORLD);
            MPI_Send(&nr_workers, 1, MPI_INT, topo[0][i], 0, MPI_COMM_WORLD);
            MPI_Send(count_dist, nr_workers, MPI_INT, topo[0][i], 0, MPI_COMM_WORLD);
            MPI_Send(index_dist, nr_workers, MPI_INT, topo[0][i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", 0, topo[0][i]);
        }

        MPI_Send(&N, 1, MPI_INT, 3, 0, MPI_COMM_WORLD);
        MPI_Send(V, N, MPI_INT, 3, 0, MPI_COMM_WORLD);
        MPI_Send(&nr_workers, 1, MPI_INT, 3, 0, MPI_COMM_WORLD);
        MPI_Send(count_dist, nr_workers, MPI_INT, 3, 0, MPI_COMM_WORLD);
        MPI_Send(index_dist, nr_workers, MPI_INT, 3, 0, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", 0, 3);

        int V_from_workers[N];
        for (i = 0; topo[0][i] != -1; i++) {           
            MPI_Recv(V_from_workers, N, MPI_INT, topo[0][i], 0, MPI_COMM_WORLD, &status);
            for (j = 0; j < N; j++) {
                if (V_from_workers[j] == 5 * V[j]) {
                    V[j] = V_from_workers[j];
                }
            }
        }

        int V_from_3[N];
        MPI_Recv(V_from_3, N, MPI_INT, 3, 0, MPI_COMM_WORLD, &status);
        for (j = 0; j < N; j++) {
            if (V_from_3[j] == 5 * V[j]) {
                V[j] = V_from_3[j];
            }
        }

        printf("Rezultat: ");
        for(i = 0; i < N; i++) {
            printf("%d", V[i]);
            if (i < N - 1)
                printf(" ");
        }
        printf("\n");
    }
    else if (rank == 3) { 
        MPI_Status status;    
        MPI_Recv(topo_recieved, 48, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);
        sprintf(file_name, "cluster%d.txt",rank);
        FILE *f = fopen(file_name, "rt");    
        if (f == NULL) MPI_Finalize();
	    fscanf(f, "%d", &num_worker);
        for(j = 0; j < 12; j++)  { 
            topo[rank][j] = -1;    
        }
	    for (int i = 0; i < num_worker; i++){
		    fscanf(f, "%d", &topo[rank][i]);
        }
        fclose(f);

        for(i = 0; i < 4; i++)  {
            for(j = 0; j < 12; j++)   {
                if (topo[i][j] != topo_recieved[i][j] && topo[i][j] == -1) topo[i][j] = topo_recieved[i][j];         
            }
        } 
        gata = 0;
        for(i = 0; i < 4; i++)  {
            if(topo[i][0] == -1) {
                gata = 1;
                break;
            }
        }

        if (gata == 0) {
            printf("%d -> ", rank);
            for (i = 0; i < 4; i++) {
                printf("%d:",i);
                for (j = 0; j < 12 && topo[i][j] != -1; j++) {
                    printf("%d", topo[i][j]);
                    if (topo[i][j + 1] != -1) {
                        printf(",");
                    }
                }
                printf(" ");
            }
            printf("\n");
            for (i = 0; i < num_worker; i++) {
                MPI_Send(topo, 48, MPI_INT, topo[rank][i], 0, MPI_COMM_WORLD);
            }
        }
            
        MPI_Send(topo, 48, MPI_INT, 0, 0, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 0);
        MPI_Recv(topo_recieved, 48, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        for(i = 0; i < 4; i++)  {
            for(j = 0; j < 12; j++)   {
                if (topo[i][j] != topo_recieved[i][j] && topo[i][j] == -1) topo[i][j] = topo_recieved[i][j];
            }
        }
        gata = 0;
        for(i = 0; i < 4; i++)  {
            if(topo[i][0] == -1) {
                gata = 1;
                break;
            }
        } 

        if (gata == 0) {
            printf("%d -> ", rank);
            for (i = 0; i < 4; i++) {
                printf("%d:",i);
                for (j = 0; j < 12 && topo[i][j] != -1; j++) {
                    printf("%d", topo[i][j]);
                    if (topo[i][j + 1] != -1) {
                        printf(",");
                    }
                }
                printf(" ");
            }
            printf("\n");
            for (i = 0; i < num_worker; i++) {
                MPI_Send(topo, 48, MPI_INT, topo[rank][i], 0, MPI_COMM_WORLD);
                printf("M(%d,%d)\n", rank, topo[rank][i]);
            }
        }

        MPI_Send(topo, 48, MPI_INT, 2, 0, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 2);
        
        //task 2
        int N_recieved, nr_workers_recieved;
        MPI_Recv(&N_recieved, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        int V_recieved[N_recieved];
        MPI_Recv(V_recieved, N_recieved, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&nr_workers_recieved, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        int count_dist_recieved[nr_workers_recieved];
        int index_dist_recieved[nr_workers_recieved];
        MPI_Recv(count_dist_recieved, nr_workers_recieved, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(index_dist_recieved, nr_workers_recieved, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

        for (i = 0; topo[3][i] != -1; i++) {
            MPI_Send(&N_recieved, 1, MPI_INT, topo[3][i], 0, MPI_COMM_WORLD);
            MPI_Send(V_recieved, N_recieved, MPI_INT, topo[3][i], 0, MPI_COMM_WORLD);
            MPI_Send(&nr_workers_recieved, 1, MPI_INT, topo[3][i], 0, MPI_COMM_WORLD);
            MPI_Send(count_dist_recieved, nr_workers_recieved, MPI_INT, topo[3][i], 0, MPI_COMM_WORLD);
            MPI_Send(index_dist_recieved, nr_workers_recieved, MPI_INT, topo[3][i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", 3, topo[3][i]);
        }

        MPI_Send(&N_recieved, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
        MPI_Send(V_recieved, N_recieved, MPI_INT, 2, 0, MPI_COMM_WORLD);
        MPI_Send(&nr_workers_recieved, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
        MPI_Send(count_dist_recieved, nr_workers_recieved, MPI_INT, 2, 0, MPI_COMM_WORLD);
        MPI_Send(index_dist_recieved, nr_workers_recieved, MPI_INT, 2, 0, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", 3, 2);

        int V_from_workers[N_recieved];
        for (i = 0; topo[3][i] != -1; i++) {           
            MPI_Recv(V_from_workers, N_recieved, MPI_INT, topo[3][i], 0, MPI_COMM_WORLD, &status);
            for (j = 0; j < N_recieved; j++) {
                if (V_from_workers[j] == 5 * V_recieved[j]) {
                    V_recieved[j] = V_from_workers[j];
                }
            }
        }

        int V_from_2[N_recieved];
        MPI_Recv(V_from_2, N_recieved, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);
        for (j = 0; j < N_recieved; j++) {
            if (V_from_2[j] == 5 * V_recieved[j]) {
                V_recieved[j] = V_from_2[j];
            }
        }

        MPI_Send(V_recieved, N_recieved, MPI_INT, 0, 0, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", 3, 0);

    }
    else if (rank == 1) {
        MPI_Status status;
        sprintf(file_name, "cluster%d.txt",rank);
        FILE *f = fopen(file_name, "rt");
        if (f == NULL)  MPI_Finalize();
 	    fscanf(f, "%d", &num_worker);
        for(j = 0; j < 12; j++) {   
            topo[rank][j] = -1;
        }
	    for (int i = 0; i < num_worker; i++){
		    fscanf(f, "%d", &topo[rank][i]);
        }

        fclose(f);
            
        MPI_Send(topo, 48, MPI_INT, 2, 0, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 2);

        MPI_Recv(topo_recieved, 48, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);
        for(i = 0; i < 4; i++)  {
            for(j = 0; j < 12; j++)   {
                    if (topo[i][j] != topo_recieved[i][j] && topo[i][j] == -1) topo[i][j] = topo_recieved[i][j];
            }
        }
        gata = 0;
        for(i = 0; i < 4; i++)  {
            if(topo[i][0] == -1) {
                gata = 1;
                break;
            }
        }

        if (gata == 0) {
            printf("%d -> ", rank);
            for (i = 0; i < 4; i++) {
                printf("%d:",i);
                for (j = 0; j < 12 && topo[i][j] != -1; j++) {
                    printf("%d", topo[i][j]);
                    if (topo[i][j + 1] != -1) {
                        printf(",");
                    }
                }
                printf(" ");
            }
            printf("\n");
            for (i = 0; i < num_worker; i++) {
                MPI_Send(topo, 48, MPI_INT, topo[rank][i], 0, MPI_COMM_WORLD);
                printf("M(%d,%d)\n", rank, topo[rank][i]);
            }
        }

        //task 2
        int N_recieved, nr_workers_recieved;
        MPI_Recv(&N_recieved, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);
        int V_recieved[N_recieved];
        MPI_Recv(V_recieved, N_recieved, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&nr_workers_recieved, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);
        int count_dist_recieved[nr_workers_recieved];
        int index_dist_recieved[nr_workers_recieved];
        MPI_Recv(count_dist_recieved, nr_workers_recieved, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(index_dist_recieved, nr_workers_recieved, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);

        for (i = 0; topo[1][i] != -1; i++) {
            MPI_Send(&N_recieved, 1, MPI_INT, topo[1][i], 0, MPI_COMM_WORLD);
            MPI_Send(V_recieved, N_recieved, MPI_INT, topo[1][i], 0, MPI_COMM_WORLD);
            MPI_Send(&nr_workers_recieved, 1, MPI_INT, topo[1][i], 0, MPI_COMM_WORLD);
            MPI_Send(count_dist_recieved, nr_workers_recieved, MPI_INT, topo[1][i], 0, MPI_COMM_WORLD);
            MPI_Send(index_dist_recieved, nr_workers_recieved, MPI_INT, topo[1][i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", 1, topo[1][i]);
        }

        int V_from_workers[N_recieved];
        for (i = 0; topo[1][i] != -1; i++) {           
            MPI_Recv(V_from_workers, N_recieved, MPI_INT, topo[1][i], 0, MPI_COMM_WORLD, &status);
            for (j = 0; j < N_recieved; j++) {
                if (V_from_workers[j] == 5 * V_recieved[j]) {
                    V_recieved[j] = V_from_workers[j];
                }
            }
        }

        MPI_Send(V_recieved, N_recieved, MPI_INT, 2, 0, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", 1, 2);
    }

    else if (rank == 2) {
        MPI_Status status;
        MPI_Recv(topo_recieved, 48, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);
        sprintf(file_name, "cluster%d.txt",rank);
        FILE *f = fopen(file_name, "rt");
        if (f == NULL) MPI_Finalize();    
	    fscanf(f, "%d", &num_worker);
        for(j = 0; j < 12; j++) { 
            topo[rank][j] = -1;
        }
	    for (int i = 0; i < num_worker; i++){
		    fscanf(f, "%d", &topo[rank][i]);
                
        }
        fclose(f);
        
        for(i = 0; i < 4; i++)  {
            for(j = 0; j < 12; j++)   {
                if (topo[i][j] != topo_recieved[i][j] && topo[i][j] == -1) topo[i][j] = topo_recieved[i][j];           
            }
        }
            
        gata = 0;
        for(i = 0; i < 4; i++)  {
            if(topo[i][0] == -1) {
                gata = 1;
                break;
            } 
        }

        if (gata == 0) {
            printf("%d -> ", rank);
            for (i = 0; i < 4; i++) {
                printf("%d:",i);
                for (j = 0; j < 12 && topo[i][j] != -1; j++) {
                    printf("%d", topo[i][j]);
                    if (topo[i][j + 1] != -1) {
                        printf(",");
                    }
                }
                printf(" ");
            }
            printf("\n");
            for (i = 0; i < num_worker; i++) {
                MPI_Send(topo, 48, MPI_INT, topo[rank][i], 0, MPI_COMM_WORLD);
                printf("M(%d,%d)\n", rank, topo[rank][i]);
            }
        }
             
        MPI_Send(topo, 48, MPI_INT, 3, 0, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 3);
        MPI_Recv(topo_recieved, 48, MPI_INT, 3, 0, MPI_COMM_WORLD, &status);           
        for(i = 0; i < 4; i++)  {
            for(j = 0; j < 12; j++)   {
                if (topo[i][j] != topo_recieved[i][j] && topo[i][j] == -1) topo[i][j] = topo_recieved[i][j];
            }
        }
        gata = 0;
        for(i = 0; i < 4; i++)  {
            if(topo[i][0] == -1) {
                gata = 1;
                break;
            }
        }

        if (gata == 0) {
            printf("%d -> ", rank);
            for (i = 0; i < 4; i++) {
                printf("%d:",i);
                for (j = 0; j < 12 && topo[i][j] != -1; j++) {
                    printf("%d", topo[i][j]);
                    if (topo[i][j + 1] != -1) {
                        printf(",");
                    }
                }
                printf(" ");
            }
            printf("\n");
            for (i = 0; i < num_worker; i++) {
                MPI_Send(topo, 48, MPI_INT, topo[rank][i], 0, MPI_COMM_WORLD);
                printf("M(%d,%d)\n", rank, topo[rank][i]);
            }
        }
            
        MPI_Send(topo, 48, MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 1);
            
        //task 2
        int N_recieved, nr_workers_recieved;
        MPI_Recv(&N_recieved, 1, MPI_INT, 3, 0, MPI_COMM_WORLD, &status);
        int V_recieved[N_recieved];
        MPI_Recv(V_recieved, N_recieved, MPI_INT, 3, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&nr_workers_recieved, 1, MPI_INT, 3, 0, MPI_COMM_WORLD, &status);
        int count_dist_recieved[nr_workers_recieved];
        int index_dist_recieved[nr_workers_recieved];
        MPI_Recv(count_dist_recieved, nr_workers_recieved, MPI_INT, 3, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(index_dist_recieved, nr_workers_recieved, MPI_INT, 3, 0, MPI_COMM_WORLD, &status);

        for (i = 0; topo[2][i] != -1; i++) {
            MPI_Send(&N_recieved, 1, MPI_INT, topo[2][i], 0, MPI_COMM_WORLD);
            MPI_Send(V_recieved, N_recieved, MPI_INT, topo[2][i], 0, MPI_COMM_WORLD);
            MPI_Send(&nr_workers_recieved, 1, MPI_INT, topo[2][i], 0, MPI_COMM_WORLD);
            MPI_Send(count_dist_recieved, nr_workers_recieved, MPI_INT, topo[2][i], 0, MPI_COMM_WORLD);
            MPI_Send(index_dist_recieved, nr_workers_recieved, MPI_INT, topo[2][i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", 2, topo[2][i]);
        }

        MPI_Send(&N_recieved, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        MPI_Send(V_recieved, N_recieved, MPI_INT, 1, 0, MPI_COMM_WORLD);
        MPI_Send(&nr_workers_recieved, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        MPI_Send(count_dist_recieved, nr_workers_recieved, MPI_INT, 1, 0, MPI_COMM_WORLD);
        MPI_Send(index_dist_recieved, nr_workers_recieved, MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", 2, 1);

        int V_from_workers[N_recieved];
        for (i = 0; topo[2][i] != -1; i++) {           
            MPI_Recv(V_from_workers, N_recieved, MPI_INT, topo[2][i], 0, MPI_COMM_WORLD, &status);
            for (j = 0; j < N_recieved; j++) {
                if (V_from_workers[j] == 5 * V_recieved[j]) {
                    V_recieved[j] = V_from_workers[j];
                }
            }
        }

        int V_from_1[N_recieved];
        MPI_Recv(V_from_1, N_recieved, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);
        for (j = 0; j < N_recieved; j++) {
            if (V_from_1[j] == 5 * V_recieved[j]) {
                V_recieved[j] = V_from_1[j];
            }
        }

        MPI_Send(V_recieved, N_recieved, MPI_INT, 3, 0, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", 2, 3);
    }

    else if (rank > 3) {
        MPI_Status status;
        MPI_Recv(topo_recieved, 48, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);

        // caut in topo_recieved coordonatorul
        int gasit = 0;
        for (i = 0; i < 4; i++) {
            for (j = 0; j < 12; j++)
                if (topo_recieved[i][j] == rank) {
                    gasit = 1;
                    break;
                }
            if (gasit) break;
        }

        int coord = i;

        // afisez topologia primita de la coordonator in formatul cerut 
        printf("%d -> ", rank);
        for (i = 0; i < 4; i++) {
            printf("%d:",i);
            for (j = 0; j < 12 && topo_recieved[i][j] != -1; j++) {
                printf("%d", topo_recieved[i][j]);
                if (topo_recieved[i][j + 1] != -1) {
                    printf(",");
                }
            }
            printf(" ");
        }
        printf("\n");

        //task 2
        int N_recieved, nr_workers_recieved;
        MPI_Recv(&N_recieved, 1, MPI_INT, coord, 0, MPI_COMM_WORLD, &status);
        int V_recieved[N_recieved];
        MPI_Recv(V_recieved, N_recieved, MPI_INT, coord, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&nr_workers_recieved, 1, MPI_INT, coord, 0, MPI_COMM_WORLD, &status);
        int count_dist_recieved[nr_workers_recieved];
        int index_dist_recieved[nr_workers_recieved];
        MPI_Recv(count_dist_recieved, nr_workers_recieved, MPI_INT, coord, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(index_dist_recieved, nr_workers_recieved, MPI_INT, coord, 0, MPI_COMM_WORLD, &status);


        for (i = index_dist_recieved[rank - 4]; 
            i < index_dist_recieved[rank - 4] + count_dist_recieved[rank - 4]; i++) {
                V_recieved[i] *= 5;
            }
        
        MPI_Send(V_recieved, N_recieved, MPI_INT, coord, 0, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, coord);      
    }

    MPI_Finalize();
}