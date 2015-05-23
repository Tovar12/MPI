#include <bits/stdc++.h>
#include "mpi.h"

using namespace std;

/** Variables de clase **/
#define MASTER 0
#define FROM_MASTER 1 //A message from MASTER
#define FROM_WORKER 2 //A message from a WORKER
const int MAXVAL = 500;
int sizeOfPointers;
int *results;

/**Variables de MPI**/
int taskId, numTasks, numWorkers, currentWorker = 0;
MPI_Status status;
MPI_Request request;

/** Iterate between workers **/
void nextWorker(){
  currentWorker++;
  if(currentWorker > numWorkers){
    currentWorker = 1;
  }
}
/** Add to values
  * @param numberArray1 position in arreglo1
  * @param numberArray2 position in arreglo1
  * @return the result of add numberArray1 with numberArray2
**/
int addNumbers(int numberArray1, int numberArray2){
  return numberArray1 + numberArray2;
  //printf("%d + %d = %d\n", numberArray1, numberArray2, arrayResult[pos]);
}
/**
  * Send to each worker a value of each pointer with data to later add
  * @param *dataArray1 all the data of the first pointer
  * @param *dataArray2 all the data of the second pointer
  * @return the length of both pointers
**/
void sendDataToWorkers(int *dataArray1, int *dataArray2, int dataLength){
  nextWorker();
  for(int i = 0; i < dataLength; i++){
    int value = i;
    MPI_Isend(&value, 1, MPI_INT, currentWorker, FROM_MASTER, MPI_COMM_WORLD,
      &request);
    value = dataArray1[i];
    MPI_Isend(&value, 1, MPI_INT, currentWorker, FROM_MASTER, MPI_COMM_WORLD,
      &request);
    value = dataArray2[i];
    MPI_Isend(&value, 1, MPI_INT, currentWorker, FROM_MASTER, MPI_COMM_WORLD,
      &request);
    nextWorker();
  }
  for(int i = 0; i < dataLength; i++){
    int value = -1;
    MPI_Isend(&value, 1, MPI_INT, currentWorker, FROM_MASTER, MPI_COMM_WORLD,
      &request);
    nextWorker();
  }
}
/**
  * Receive the data necessary to process. Also send the result when is ready
**/
void receiveDataFromWorkers(){
  int index;
  int dataArray1;
  int dataArray2;
  MPI_Recv(&index, 1, MPI_INT, MASTER, FROM_MASTER, MPI_COMM_WORLD, &status);
  while(index != -1){
    /** Recibe el dato del primer puntero **/
    MPI_Recv(&dataArray1, 1, MPI_INT, MASTER, FROM_MASTER, MPI_COMM_WORLD,
      &status);
    /** Recibe el dato del segundo puntero **/
    MPI_Recv(&dataArray2, 1, MPI_INT, MASTER, FROM_MASTER, MPI_COMM_WORLD,
      &status);
    int result = addNumbers(dataArray1, dataArray2);
    /**
    printf("Soy el worker %d y estoy haciendo: %d + %d = %d\n", taskId,
    dataArray1, dataArray2, result);
    **/
    /** Envía la posición del puntero para saber donde almacenar el resultado**/
    MPI_Isend(&index, 1, MPI_INT, MASTER, FROM_WORKER, MPI_COMM_WORLD,
      &request);
    /** Envía el resultado de la suma **/
    MPI_Isend(&result, 1, MPI_INT, MASTER, FROM_WORKER, MPI_COMM_WORLD,
      &request);
    /** Actualiza el valor del index **/
    MPI_Recv(&index, 1, MPI_INT, MASTER, FROM_MASTER, MPI_COMM_WORLD, &status);
  }
}
/**
  * Receive the result data of the add and the store It in a pointer
  * @param dataLength the size of the pointer with all the answers
**/
void receiveResult(int dataLength){
  results = new int[dataLength];
  currentWorker = 0;
  nextWorker();
  for(int i = 0; i < dataLength; i++){
    int index;
    int actualData;
    MPI_Recv(&index, 1, MPI_INT, currentWorker, FROM_WORKER, MPI_COMM_WORLD,
      &status);
    MPI_Recv(&actualData, 1, MPI_INT, currentWorker, FROM_WORKER,
      MPI_COMM_WORLD, &status);
    results[index] = actualData;
    nextWorker();
  }
}
/** A method necessary to work with MPI **/
void initMPI(int argc, char *argv[]) {
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &taskId);
  //numTasks corresponde al parámetro que acompaña -np X
  MPI_Comm_size(MPI_COMM_WORLD, &numTasks);
  numWorkers = numTasks-1;
}

int main(int argc, char *argv[]) {
  initMPI(argc, argv);
  if(taskId == MASTER){
    /** Start timer **/
    double startTime = MPI_Wtime();
    srand (time(NULL));
    cout << "Enter the size of the pointers: " << endl;
    cin >> sizeOfPointers;
    int *array1 = new int [sizeOfPointers];
    int *array2 = new int [sizeOfPointers];
    for(int i = 0; i < sizeOfPointers; i++){
      array1[i] = rand() % MAXVAL;
      array2[i] = rand() % MAXVAL;
    }
    sendDataToWorkers(&array1[0], &array2[0], sizeOfPointers);
    receiveResult(sizeOfPointers);
    /**
    for(int i = 0; i < sizeOfPointers; ++i){
      printf("%d + %d es %d\n", array1[i], array2[i], results[i]);
    }
    **/
    double endTime = MPI_Wtime();
    printf("Total execution time %f\n", endTime - startTime);
  } else {
    printf("Soy el nodo %d\n", taskId);
    receiveDataFromWorkers();
  }
  MPI_Finalize();
  return 0;
}
