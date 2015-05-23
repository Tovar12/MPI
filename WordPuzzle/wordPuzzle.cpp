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
/** Variables of class **/
string *wordsToSearch;

/** Convert to string **/
string toStr(int x){
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
/**
  * Find a word in a sentence
  * @param *lineToSearch the line where the word will be searched
  * @param sizeWords the size of the pointer *words
  * @param *words a pointer with the words that need to be searched
**/
void findWord(char *lineToSearch, int sizeWords, string *words){
  string line (lineToSearch);
  string output;
  int iterator = 0;
  for(int i = 0; i < sizeWords; i++){
    int found = line.find(words[i]);
    if(found != string::npos){
      //freopen("out.txt", "w", stdout);
      string posX = toStr(found);
      output = words[i] + " ( " + posX + " , )";
      //printf("Word %s found in pos (%d,)\n", words[i].c_str(), found);
      cout << output << endl;
    }
  }
}

/**
  * Send Data to workers
  * @param *soup pointer with the soup
  * @param *words pointer with the words to search
  * @sizeSopa size of the pointer *soup
  * @sizeWords size of the pointer *words
  *
**/
void sendDataToWorkers(string *soup, string *words, int sizeSoup,
  int sizeWords){
  nextWorker();
  //Send the size of words to all workers
  for(int i = 0; i < numWorkers; i++){
    MPI_Isend(&sizeWords, 1, MPI_INT, currentWorker, FROM_MASTER, MPI_COMM_WORLD,
      &request);
    nextWorker();
  }
  currentWorker = 0;
  nextWorker();
  //Send all words to all workers
  for(int i = 0; i < numWorkers; i++){
    for(int j = 0; j < sizeWords; j++){
      int size = words[j].size();
      /** send the size of each word **/
      MPI_Isend(&size, 1, MPI_INT, currentWorker, FROM_MASTER, MPI_COMM_WORLD,
        &request);
      char *writable = new char[words[j].size() + 1];
      /** Put the final char in the pointer **/
      writable[words[j].size()] = '\0';
      /** Copy the word in the new pointer **/
      std::copy(words[j].begin(), words[j].end(), writable);
      /** Send the word **/
      MPI_Isend(writable, words[j].size(), MPI_CHAR, currentWorker, FROM_MASTER,
        MPI_COMM_WORLD, &request);
    }
    nextWorker();
  }
  currentWorker = 0;
  nextWorker();
  int index;
  /** Send a line of *soup to each worker **/
  for(int i = 0; i < sizeSoup; i++){
    /** Send an index just to know that there are still data to send **/
    index = i;
    MPI_Isend(&index, 1, MPI_INT, currentWorker, FROM_MASTER, MPI_COMM_WORLD,
      &request);
    int size = soup[i].size();
    /** send the size of each line of soup **/
    MPI_Isend(&size, 1, MPI_INT, currentWorker, FROM_MASTER, MPI_COMM_WORLD,
      &request);
    char *value = new char[soup[i].size() + 1];
    /** Put the final char in the pointer **/
    value[soup[i].size()] = '\0';
    /** Copy the word in the new pointer **/
    std::copy(soup[i].begin(), soup[i].end(), value);
    /** Send the line of soup **/
    MPI_Isend(value, soup[i].size(), MPI_CHAR, currentWorker, FROM_MASTER,
    MPI_COMM_WORLD, &request);
    nextWorker();
  }

  /** Send -1 to each worker when there aren't any more data to send **/
  for(int i = 0; i < sizeSoup; i++){
    int value = -1;
    MPI_Isend(&value, 1, MPI_INT, currentWorker, FROM_MASTER, MPI_COMM_WORLD,
      &request);
    nextWorker();
  }
}
/**
  * Process the data and return an answer to Master
**/
void receiveDataFromMaster(){
  string *words;
  int sizeWords;
  int valid;
  /** Receive the size of words **/
  MPI_Recv(&sizeWords, 1, MPI_INT, MASTER, FROM_MASTER, MPI_COMM_WORLD,
    &status);
  words = new string[sizeWords];
  /** Receive the words **/
  for(int i = 0; i < sizeWords; i++){
    int tempSize;
    /** Receive the size of the word **/
    MPI_Recv(&tempSize, 1, MPI_INT, MASTER, FROM_MASTER, MPI_COMM_WORLD,
      &status);
    char *word = new char[tempSize + 1];
    word[tempSize] = '\0';
    MPI_Recv(word, tempSize, MPI_CHAR, MASTER, FROM_MASTER, MPI_COMM_WORLD,
      &status);
    words[i] = word;
  }
  MPI_Recv(&valid, 1, MPI_INT, MASTER, FROM_MASTER, MPI_COMM_WORLD, &status);
  while(valid != -1){
    int tempSize;
    /** Receive the size of each line of soup **/
    MPI_Recv(&tempSize, 1, MPI_INT, MASTER, FROM_MASTER, MPI_COMM_WORLD,
      &status);
    char *lineSoup = new char[tempSize + 1];
    lineSoup[tempSize] = '\0';
    MPI_Recv(lineSoup, tempSize, MPI_CHAR, MASTER, FROM_MASTER,MPI_COMM_WORLD,
      &status);
    findWord(lineSoup, sizeWords, &words[0]);
    /** Update the value of valid **/
    MPI_Recv(&valid, 1, MPI_INT, MASTER, FROM_MASTER, MPI_COMM_WORLD, &status);
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
    int n, m;
    string *soup, *words;
    freopen("in.txt", "r", stdin);
    cin >> n >> m;
    string verticalWord;
    soup = new string[n+n];
    words = new string[m];
    for(int i = 0; i < n; ++i){
      cin >> soup[i];
    }
    for(int i = 0; i < m; ++i){
      cin >> words[i];
    }
    for(int i = 0; i < n; ++i) {
      for(int j = 0; j < n; ++j) {
        verticalWord += soup[j][i];
      }
      soup[n + i] = verticalWord;
      verticalWord = "";
    }
    sendDataToWorkers(&soup[0], &words[0], n+n, m);
    double endTime = MPI_Wtime();
    printf("Total execution time %f\n", endTime - startTime);
  } else {
    receiveDataFromMaster();
  }
  MPI_Finalize();
  return 0;
}
