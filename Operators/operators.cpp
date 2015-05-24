#include <bits/stdc++.h>
#include "mpi.h"
using namespace std;
/** Variables MPI **/
#define MASTER 0
#define FROM_MASTER 1 //A message from MASTER
#define FROM_WORKER 2 //A message from a WORKER
int taskId, numTasks, numWorkers, currentWorker = 0;
MPI_Status status;
MPI_Request request;

string *results;

int toInt(string s) {
  const char *hola = s.c_str();
  return atoi(hola);
}

/** Convert to string **/
string toStr(float x){
  stringstream s;
  s << x;
  return s.str();
}

/** Iterate between workers **/
void nextWorker(){
  currentWorker++;
  if(currentWorker > numWorkers){
    currentWorker = 1;
  }
}

float evalute(char *line){
  string expr(line);
  string first = "", sec = "";
  float result = 0;
  int numero1 = 0;
  int numero2 = 0;
  //int size = line.size();
  for(int i = 0; i < expr.size(); i++){
    switch (expr[i]) {
      case '+':
        first = expr.substr(0, i);
        sec = expr.substr(i + 1, expr.size());
        numero1 = toInt(first);
        numero2 = toInt(sec);
        result = (float)numero1 + (float)numero2;
        break;
      case '-':
        first = expr.substr(0, i);
        sec = expr.substr(i + 1, expr.size());
        numero1 = toInt(first);
        numero2 = toInt(sec);
        result = (float)numero1 - (float)numero2;
        break;
      case '*':
        first = expr.substr(0, i);
        sec = expr.substr(i + 1, expr.size());
        numero1 = toInt(first);
        numero2 = toInt(sec);
        result = (float)numero1 * (float)numero2;
        break;
      case '/':
        first = expr.substr(0, i);
        sec = expr.substr(i + 1, expr.size());
        numero1 = toInt(first);
        numero2 = toInt(sec);
        result = (float)numero1 / (float)numero2;
        break;
    }
    if (first != "" && sec != "") break;
  }

  return result;
  //cout << "Suma: " << (toInt(first) + toInt(sec)) << endl;
  //cout << line << endl;
}

void sendDataToWorkers(string line, int index){
  nextWorker();
  int iterator;
  /** Send an index just to know that there are still data to send **/
  iterator = 1;
  //MPI_Isend(&iterator, 1, MPI_INT, currentWorker, FROM_MASTER, MPI_COMM_WORLD,
  //  &request);
  int size = line.size();
  MPI_Isend(&size, 1, MPI_INT, currentWorker, FROM_MASTER, MPI_COMM_WORLD,
    &request);
  MPI_Isend(&iterator, 1, MPI_INT, currentWorker, FROM_MASTER, MPI_COMM_WORLD,
    &request);
  char *writable = new char[line.size() + 1];
  writable[line.size()] = '\0';
  std::copy(line.begin(), line.end(), writable);
  MPI_Isend(writable, line.size(), MPI_CHAR, currentWorker, FROM_MASTER,
    MPI_COMM_WORLD, &request);
  MPI_Isend(&index, 1, MPI_INT, currentWorker, FROM_MASTER, MPI_COMM_WORLD,
    &request);
}

void finishSendingData(){
  currentWorker = 0;
  nextWorker();
  for(int i = 0; i < numWorkers; i++){
    int size = 0;
    int index = 0;
    MPI_Isend(&size, 1, MPI_INT, currentWorker, FROM_MASTER, MPI_COMM_WORLD,
      &request);
    int iterator = -1;
    MPI_Isend(&iterator, 1, MPI_INT, currentWorker, FROM_MASTER, MPI_COMM_WORLD,
      &request);
    nextWorker();
  }
}

void receiveDataFromMaster(){
  int iterator = 0;
  int sizeLine;
  int index;
  currentWorker = 0;
  nextWorker();
  while(iterator != -1){
    MPI_Recv(&sizeLine, 1, MPI_INT, MASTER, FROM_MASTER, MPI_COMM_WORLD,
      &status);
    MPI_Recv(&iterator, 1, MPI_INT, MASTER, FROM_MASTER, MPI_COMM_WORLD,
      &status);
    if(sizeLine != 0){
      char *line = new char[sizeLine + 1];
      line[sizeLine] = '\0';
      MPI_Recv(line, sizeLine, MPI_CHAR, MASTER, FROM_MASTER, MPI_COMM_WORLD,
        &status);
      MPI_Recv(&index, 1, MPI_INT, MASTER, FROM_MASTER, MPI_COMM_WORLD,
        &status);
      float result = evalute(line);
      MPI_Isend(&sizeLine, 1, MPI_INT, MASTER, FROM_WORKER, MPI_COMM_WORLD,
        &request);
      MPI_Isend(line, sizeLine, MPI_CHAR, MASTER, FROM_WORKER, MPI_COMM_WORLD,
        &request);
      MPI_Isend(&index, 1, MPI_INT, MASTER, FROM_WORKER, MPI_COMM_WORLD,
        &request);
      MPI_Isend(&result, 1, MPI_FLOAT, MASTER, FROM_WORKER, MPI_COMM_WORLD,
        &request);
      nextWorker();
    }
  }
}

void receiveResult(int sizeLines){
  int sizeLine;
  int index;
  float result;
  results = new string [sizeLines];
  currentWorker = 0;
  nextWorker();
  for(int i = 0; i < sizeLines; i++){
    MPI_Recv(&sizeLine, 1, MPI_INT, currentWorker, FROM_WORKER, MPI_COMM_WORLD,
      &status);
    char *line = new char[sizeLine + 1];
    line[sizeLine] = '\0';
    MPI_Recv(line, sizeLine, MPI_CHAR, currentWorker, FROM_WORKER, MPI_COMM_WORLD,
      &status);
    MPI_Recv(&index, 1, MPI_INT, currentWorker, FROM_WORKER, MPI_COMM_WORLD,
      &status);
    MPI_Recv(&result, 1, MPI_FLOAT, currentWorker, FROM_WORKER, MPI_COMM_WORLD,
      &status);
    string newLine (line);
    string output = newLine + "= " + toStr(result);
    results[index] = output;
    nextWorker();
  }
}

void createTXT(int sizeLines){
  freopen("datos_out.txt", "w", stdout);
  for(int i = 0; i < sizeLines; i++){
    cout << results[i] << endl;
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

int main(int argc, char *argv[]){
  initMPI(argc, argv);
  if(taskId == MASTER){
    /** Start timer **/
    double startTime = MPI_Wtime();
    ifstream file("datos_in.txt");
    string line;
    int index = 0;
    while(getline(file, line)){
      //cout << line << endl;
      sendDataToWorkers(line, index);
      index++;
    }
    finishSendingData();
    receiveResult(index);
    double endTime = MPI_Wtime();
    printf("Total execution time %f\n", endTime - startTime);      
    createTXT(index);
  } else {
    receiveDataFromMaster();
  }
  MPI_Finalize();
  return 0;
}
