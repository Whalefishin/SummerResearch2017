#include <string>
#include <iostream>
#include <math.h>
#include <ctime>


#include"Balanced_Network.h"
#include "stdlib.h"

using namespace std;


Balanced_Network::Balanced_Network(double N_E, double N_I, double K,
  double J_EE, double J_EI, double J_IE, double J_II, double m_0,
double externalRateFactor, double phi, double lambda,double update_times){
//  srand(1);

  time = 0;
  this->update_times = update_times;
  update_count=0;
  inf_size = 500000;

  //network = new AdjacencyListGraph<Neuron*,string,double>;
  minimum_time_queue = new STLPriorityQueue<double,Neuron*>;

  //adds E excitatory and I inhibitory neurons to the network
  this->N_E = N_E;
  this->N_I = N_I;
  this->K = K;
  this->externalRateFactor = externalRateFactor;
  this->lambda = lambda;
  this->phi = phi;
  this->m_0 = m_0;

  for (int i=1;i<=N_E;i++){
    Neuron* n = new Neuron(i,"E",K,m_0,externalRateFactor,phi,lambda,N_E);
    //network->insertVertex(n);
    neuron_Vector.push_back(n);
  }

  for (int i=1;i<=N_I;i++){
    Neuron* n = new Neuron(i,"I",K,m_0,externalRateFactor,phi,lambda,N_E);
    //network->insertVertex(n);
    neuron_Vector.push_back(n);
  }

  //neuron_Vector = network->getVertices();
  //adds all the update times to a priority queue.
  for (int i=0;i<neuron_Vector.size();i++){
    Neuron* toInsert = neuron_Vector[i];
    minimum_time_queue->insert((-1*toInsert->time_to_be_updated),toInsert);
  }

  //initialize the neurons to states 0 or 1
  initializeNeurons();
  initializeJmatrix(J_EE,J_EI,J_IE,J_II);

  double excAtvCount = 0;
  double inhAtvCount = 0;

  for (int i=0;i<neuron_Vector.size();i++){
    Neuron* n = neuron_Vector[i];
    if (n->state ==1){
      if (n->population == "E"){
        excAtvCount = excAtvCount+1;
      }
      else if (n->population == "I"){
        inhAtvCount = inhAtvCount+1;
      }
    }
  }
  //mean activity stuff
  pair<double,double> excData(0,excAtvCount);
  pair<double,double> inhData(0,inhAtvCount);
  excitatoryActivityTimeSeries.push_back(excData);
  inhibitoryActivityTimeSeries.push_back(inhData);
  cout << "Initial active exc: " + to_string(excAtvCount) << endl;
  cout << "Initial active inh: " + to_string(inhAtvCount) << endl;

  //mean Threshold stuff
  pair<double,double> excThresholdData(0,N_E*1);
  pair<double,double> inhThresholdData(0,N_I*0.7);
  meanExcThresholdTimeSeries.push_back(excThresholdData);
  meanInhThresholdTimeSeries.push_back(inhThresholdData);

}

Balanced_Network::~Balanced_Network(){
  deleteNeurons();
  deleteJmatrix();
  delete minimum_time_queue;
  //delete network;
}

double Balanced_Network::getThreshold(Neuron* n){
  if (containsNeuron(n)){
    return n->threshold;
  }
  else {
    throw runtime_error("This neuron is not in the network.");
  }
}

double Balanced_Network::getExternalInput(Neuron* n){
  if (containsNeuron(n)){
    return n->externalInput;
  }
  else {
    throw runtime_error("This neuron is not in the network.");
  }
}

double Balanced_Network::getTotalInput(Neuron* n){
  if (containsNeuron(n)){
    return n->totalInput;
  }
  else {
    throw runtime_error("This neuron is not in the network.");
  }
}

double Balanced_Network::getN_E(){
  return N_E;
}

double Balanced_Network::getN_I(){
  return N_I;
}

double Balanced_Network::getK(){
  return K;
}

double** Balanced_Network::getJmatrix(){
  return Jmatrix;
}

double Balanced_Network::getTime(){
  return time;
}

vector<double> Balanced_Network::getEI_Ratios(){
  return EI_Ratio_Collection;
}

vector<double> Balanced_Network::getInhEI_Ratios(){
  return EI_Ratio_Collection_Inh;
}

vector<double> Balanced_Network::getExcEI_Ratios(){
  return EI_Ratio_Collection_Exc;
}

vector<double> Balanced_Network::getEI_Ratios_inf(){
  return EI_Ratio_Collection_inf;
}

vector<double> Balanced_Network::getExcEI_Ratios_inf(){
  return EI_Ratio_Collection_Exc_inf;
}

vector<double> Balanced_Network::getInhEI_Ratios_inf(){
  return EI_Ratio_Collection_Inh_inf;
}



vector<pair<double,double>> Balanced_Network::getExcMeanAtv(){
  return excitatoryActivityTimeSeries;
}

vector<pair<double,double>> Balanced_Network::getInhMeanAtv(){
  return inhibitoryActivityTimeSeries;
}

pair<double,double> Balanced_Network::getEM_data_exc(){
  //format: external rate vs. Mean atv
  pair<double,double> toReturn;
  //all the neurons in the same network have the same externalInput
  //so we just choose the first one
  toReturn.first = (neuron_Vector[0]->externalRateFactor);
  double neuronSum=0;
  for (int i=0;i<N_E;i++){
    neuronSum += (neuron_Vector[i]->stateSum/neuron_Vector[i]->update_count);
  }
  toReturn.second = neuronSum/(N_E);
  return toReturn;
}

pair<double,double> Balanced_Network::getEM_data_inh(){
  pair<double,double> toReturn;
  toReturn.first = (neuron_Vector[0]->externalRateFactor);
  double neuronSum=0;
  for (int i=N_E;i<neuron_Vector.size();i++){
    neuronSum += (neuron_Vector[i]->stateSum/neuron_Vector[i]->update_count);
  }
  toReturn.second = neuronSum/(N_I);
  return toReturn;
}

pair<double,double> Balanced_Network::getEM_data_exc2(){
  pair<double,double> toReturn;
  toReturn.first = (neuron_Vector[0]->externalRateFactor);
  double size = excitatoryActivityTimeSeries.size();
  for (int i=0;i< excitatoryActivityTimeSeries.size();i++){
    excStateSum +=excitatoryActivityTimeSeries[i].second;
  }
  //cout << excStateSum << endl;
  toReturn.second = excStateSum/N_E/(excitatoryActivityTimeSeries.size());
  return toReturn;
}

pair<double,double> Balanced_Network::getEM_data_inh2(){
  pair<double,double> toReturn;
  toReturn.first = (neuron_Vector[0]->externalRateFactor);
  double size = inhibitoryActivityTimeSeries.size();
  for (int i=0;i< inhibitoryActivityTimeSeries.size();i++){
    inhStateSum += inhibitoryActivityTimeSeries[i].second;
  }
  cout << inhStateSum << endl;
  toReturn.second = inhStateSum/N_I/(inhibitoryActivityTimeSeries.size());
  return toReturn;
}

double Balanced_Network::getEM_data_exc_sd(){
  vector<double> data;
  for (int i=0;i<excitatoryActivityTimeSeries.size();i++){
    data.push_back(excitatoryActivityTimeSeries[i].second/N_E);
  }
  return standardDeviation(data);
}


double Balanced_Network::getEM_data_inh_sd(){
  vector<double> data;
  for (int i=0;i<inhibitoryActivityTimeSeries.size();i++){
    data.push_back(inhibitoryActivityTimeSeries[i].second/N_I);
  }
  return standardDeviation(data);
}


double Balanced_Network::getMeanExcThreshold(){
  vector<double> thresholds;
  for(int i=0;i<meanExcThresholdTimeSeries.size();i++){
    thresholds.push_back(meanExcThresholdTimeSeries[i].second/N_E);
  }
  //cout << thresholds.size() << endl;
  return mean(thresholds);
}

double Balanced_Network::getMeanInhThreshold(){
  vector<double> thresholds;
  for(int i=0;i<meanInhThresholdTimeSeries.size();i++){
    thresholds.push_back(meanInhThresholdTimeSeries[i].second/N_I);
  }
  return mean(thresholds);
}

double Balanced_Network::getMeanThreshold(){
  vector<double> thresholds;
  for (int i=0;i<meanExcThresholdTimeSeries.size();i++){
    thresholds.push_back(meanExcThresholdTimeSeries[i].second/N_E);
  }
  for (int i=0;i<meanInhThresholdTimeSeries.size();i++){
    thresholds.push_back(meanInhThresholdTimeSeries[i].second/N_I);
  }
  return mean(thresholds);
}


double Balanced_Network::getTheta_exc_inf(){
  double sum =0;
  double count =0;
  int size = meanExcThresholdTimeSeries.size();
  //we take an average of the last 40000 data to approximate m_inf
  for (int i=size-40000;i<size;i++){
    sum += (meanExcThresholdTimeSeries[i].second/N_E);
    count++;
  }
  return sum/count;
}

double Balanced_Network::getTheta_inh_inf(){
  double sum =0;
  double count =0;
  int size = meanInhThresholdTimeSeries.size();
  //we take an average of the last 10000 data to approximate m_inf
  for (int i=size-10000;i<size;i++){
    sum += (meanInhThresholdTimeSeries[i].second/N_I);
    count++;
  }
  return sum/count;
}

double Balanced_Network::getTheta_inf(){
  vector<double> thresholds;
  for (int i=meanExcThresholdTimeSeries.size()-40000;i<meanExcThresholdTimeSeries.size();i++){
    thresholds.push_back(meanExcThresholdTimeSeries[i].second/N_E);
  }
  for (int i=meanInhThresholdTimeSeries.size()-10000;i<meanInhThresholdTimeSeries.size();i++){
    thresholds.push_back(meanInhThresholdTimeSeries[i].second/N_I);
  }
  return mean(thresholds);
}

//Below computes the temporal SD
/*
double Balanced_Network::getExcThresholdSD(){
  vector<double> thresholds;
  for (int i=0;i<meanExcThresholdTimeSeries.size();i++){
    thresholds.push_back(meanExcThresholdTimeSeries[i].second/N_E);
  }
  return standardDeviation(thresholds);
}

double Balanced_Network::getInhThresholdSD(){
  vector<double> thresholds;
  for (int i=0;i<meanInhThresholdTimeSeries.size();i++){
    thresholds.push_back(meanInhThresholdTimeSeries[i].second/N_I);
  }
  return standardDeviation(thresholds);
}


double Balanced_Network::getThresholdSD(){
  vector<double> thresholds;
  for (int i=0;i<meanExcThresholdTimeSeries.size();i++){
    thresholds.push_back(meanExcThresholdTimeSeries[i].second/N_E);
  }
  for (int i=0;i<meanInhThresholdTimeSeries.size();i++){
    thresholds.push_back(meanInhThresholdTimeSeries[i].second/N_I);
  }
  return standardDeviation(thresholds);
}


*/

//This is the populational SD
double Balanced_Network::getExcThresholdSD(){
  vector<double> vector;
  for (int i=0;i<N_E+N_I;i++){
    Neuron* neuron = neuron_Vector[i];
    if (neuron->population == "E"){
      vector.push_back(neuron->thresholdSum/neuron->update_count_neuronal);
    }
  }
  return standardDeviation(vector);
}

double Balanced_Network::getInhThresholdSD(){
  vector<double> vector;
  for (int i=0;i<N_E+N_I;i++){
    Neuron* neuron = neuron_Vector[i];
    if (neuron->population == "I"){
      vector.push_back(neuron->thresholdSum/neuron->update_count_neuronal);
    }
  }
  return standardDeviation(vector);
}

double Balanced_Network::getThresholdSD(){
  vector<double> vector;
  for (int i=0;i<N_E+N_I;i++){
    Neuron* neuron = neuron_Vector[i];
    vector.push_back(neuron->thresholdSum/neuron->update_count_neuronal);
  }
  return standardDeviation(vector);
}

vector<pair<double,double>> Balanced_Network::getMeanExcThresholdTimeSeries(){
  return meanExcThresholdTimeSeries;
}

vector<pair<double,double>> Balanced_Network::getMeanInhThresholdTimeSeries(){
  return meanInhThresholdTimeSeries;
}


double Balanced_Network::getM_exc_inf(){
  double sum =0;
  double count =0;
  int size = excitatoryActivityTimeSeries.size();
  //we take an average of the last 500 data to approximate m_inf
  for (int i=size-40000;i<size;i++){
    sum += excitatoryActivityTimeSeries[i].second/N_E;
    count++;
  }
  return sum/count;
}

double Balanced_Network::getM_inh_inf(){
  double sum =0;
  double count =0;
  int size = inhibitoryActivityTimeSeries.size();
  //we take an average of the last 500 data to approximate m_inf
  for (int i=size-10000;i<size;i++){
    sum += inhibitoryActivityTimeSeries[i].second/N_I;
    count++;
  }
  return sum/count;
}

double Balanced_Network::getM_exc_sd_inf(){
  vector<double> vector;
  int size = excitatoryActivityTimeSeries.size();
  //we take an average of the last 500 data to approximate m_inf
  for (int i=size-40000;i<size;i++){
    vector.push_back(excitatoryActivityTimeSeries[i].second/N_E);
  }
  return standardDeviation(vector);
}

double Balanced_Network::getM_inh_sd_inf(){
  vector<double> vector;
  int size = inhibitoryActivityTimeSeries.size();
  //we take an average of the last 500 data to approximate m_inf
  for (int i=size-10000;i<size;i++){
    vector.push_back(inhibitoryActivityTimeSeries[i].second/N_E);
  }
  return standardDeviation(vector);
}


//temporal SD
/*
double Balanced_Network::getTheta_exc_inf_SD(){
  double sum;
  double count;
  vector<double> tail;
  vector<pair<double,double>> vector = meanExcThresholdTimeSeries;
  int size = vector.size();
  for(int i=-10000;i<size;i++){
    tail.push_back(vector[i].second/N_E);
  }
  return standardDeviation(tail);
}

double Balanced_Network::getTheta_inh_inf_SD(){
  double sum;
  double count;
  vector<double> tail;
  vector<pair<double,double>> vector = meanInhThresholdTimeSeries;
  int size = vector.size();
  for(int i=size-10000;i<size;i++){
    tail.push_back(vector[i].second/N_I);
  }
  return standardDeviation(tail);
}
*/

//populational SD
double Balanced_Network::getTheta_exc_inf_SD(){
  vector<double> vector;
  for (int i=0;i<N_E+N_I;i++){
    Neuron* neuron = neuron_Vector[i];
    if (neuron->population == "E"){
      vector.push_back(neuron->thresholdSum_inf/neuron->update_count_neuronal_inf);
    }
  }
  return standardDeviation(vector);
}

double Balanced_Network::getTheta_inh_inf_SD(){
  vector<double> vector;
  for (int i=0;i<N_E+N_I;i++){
    Neuron* neuron = neuron_Vector[i];
    if (neuron->population == "I"){
      vector.push_back(neuron->thresholdSum_inf/neuron->update_count_neuronal_inf);
    }
  }
  return standardDeviation(vector);
}

double Balanced_Network::getTheta_inf_SD(){
  vector<double> vector;
  for (int i=0;i<N_E+N_I;i++){
    Neuron* neuron = neuron_Vector[i];
    vector.push_back(neuron->thresholdSum_inf/neuron->update_count_neuronal_inf);
  }
  return standardDeviation(vector);
}

/*
vector<double> Balanced_Network::getTime_Vector(){
  return time_vector;
}
*/

double Balanced_Network::getTotalInputExcMean(){
  return mean(totalInput_exc);
}

double Balanced_Network::getTotalInputInhMean(){
  return mean(totalInput_inh);
}

//Temporal SD
/*
double Balanced_Network::getTotalInputExcSD(){
  return standardDeviation(totalInput_exc);
}

double Balanced_Network::getTotalInputInhSD(){
  return standardDeviation(totalInput_inh);
}

double Balanced_Network::getTotalInputExcSDInf(){
  double sum;
  double count;
  vector<double> tail;
  vector<double> vector = totalInput_exc;
  int size = vector.size();
  for(int i=size-40000;i<size;i++){
    tail.push_back(vector[i]);
  }
  return standardDeviation(tail);
}

double Balanced_Network::getTotalInputInhSDInf(){
  double sum;
  double count;
  vector<double> tail;
  vector<double> vector = totalInput_inh;
  int size = vector.size();
  for(int i=size-10000;i<size;i++){
    tail.push_back(vector[i]);
  }
  return standardDeviation(tail);
}
*/

//Populational SD
double Balanced_Network::getTotalInputExcSD(){
  vector<double> vector;
  for (int i=0;i<N_E+N_I;i++){
    Neuron* neuron = neuron_Vector[i];
    if (neuron->population == "E"){
      vector.push_back(neuron->totalInput_Sum/neuron->update_count_neuronal);
    }
  }
  return standardDeviation(vector);
}

double Balanced_Network::getTotalInputInhSD(){
  vector<double> vector;
  for (int i=0;i<N_E+N_I;i++){
    Neuron* neuron = neuron_Vector[i];
    if (neuron->population == "I"){
      vector.push_back(neuron->totalInput_Sum/neuron->update_count_neuronal);
    }
  }
  return standardDeviation(vector);
}

double Balanced_Network::getTotalInputExcSDInf(){
  vector<double> vector;
  for (int i=0;i<N_E+N_I;i++){
    Neuron* neuron = neuron_Vector[i];
    if (neuron->population == "E"){
      vector.push_back(neuron->totalInput_Sum_inf/neuron->update_count_neuronal_inf);
    }
  }
  return standardDeviation(vector);
}

double Balanced_Network::getTotalInputInhSDInf(){
  vector<double> vector;
  for (int i=0;i<N_E+N_I;i++){
    Neuron* neuron = neuron_Vector[i];
    if (neuron->population == "I"){
      vector.push_back(neuron->totalInput_Sum_inf/neuron->update_count_neuronal_inf);
    }
  }
  return standardDeviation(vector);
}




double Balanced_Network::getEInputExcMean(){
  vector<double> vector;
  for (int i=0;i<N_E+N_I;i++){
    Neuron* neuron = neuron_Vector[i];
    if (neuron->population == "E"){
      vector.push_back(neuron->E_Input/neuron->update_count_neuronal);
    }
  }
  return mean(vector);
}

double Balanced_Network::getEInputInhMean(){
  vector<double> vector;
  for (int i=0;i<N_E+N_I;i++){
    Neuron* neuron = neuron_Vector[i];
    if (neuron->population == "I"){
      vector.push_back(neuron->E_Input/neuron->update_count_neuronal);
    }
  }
  return mean(vector);
}

double Balanced_Network::getEInputMean(){
  vector<double> vector;
  for (int i=0;i<N_E+N_I;i++){
    Neuron* neuron = neuron_Vector[i];
    vector.push_back(neuron->E_Input/neuron->update_count_neuronal);
  }
  return mean(vector);
}

double Balanced_Network::getEInputExcMeanInf(){
  vector<double> vector;
  for (int i=0;i<N_E+N_I;i++){
    Neuron* neuron = neuron_Vector[i];
    if (neuron->population == "E"){
      vector.push_back(neuron->E_Input_inf/neuron->update_count_neuronal_inf);
    }
  }
  return mean(vector);
}

double Balanced_Network::getEInputInhMeanInf(){
  vector<double> vector;
  for (int i=0;i<N_E+N_I;i++){
    Neuron* neuron = neuron_Vector[i];
    if (neuron->population == "I"){
      vector.push_back(neuron->E_Input_inf/neuron->update_count_neuronal_inf);
    }
  }
  return mean(vector);
}

double Balanced_Network::getEInputMeanInf(){
  vector<double> vector;
  for (int i=0;i<N_E+N_I;i++){
    Neuron* neuron = neuron_Vector[i];
    vector.push_back(neuron->E_Input_inf/neuron->update_count_neuronal_inf);
  }
  return mean(vector);
}

double Balanced_Network::getEInputExcSD(){
  vector<double> vector;
  for (int i=0;i<N_E+N_I;i++){
    Neuron* neuron = neuron_Vector[i];
    if (neuron->population == "E"){
      vector.push_back(neuron->E_Input/neuron->update_count_neuronal);
    }
  }
  return standardDeviation(vector);
}

double Balanced_Network::getEInputInhSD(){
  vector<double> vector;
  for (int i=0;i<N_E+N_I;i++){
    Neuron* neuron = neuron_Vector[i];
    if (neuron->population == "I"){
      vector.push_back(neuron->E_Input/neuron->update_count_neuronal);
    }
  }
  return standardDeviation(vector);
}

double Balanced_Network::getEInputSD(){
  vector<double> vector;
  for (int i=0;i<N_E+N_I;i++){
    Neuron* neuron = neuron_Vector[i];
    vector.push_back(neuron->E_Input/neuron->update_count_neuronal);
  }
  return standardDeviation(vector);
}

double Balanced_Network::getEInputExcSDInf(){
  vector<double> vector;
  for (int i=0;i<N_E+N_I;i++){
    Neuron* neuron = neuron_Vector[i];
    if (neuron->population == "E"){
      vector.push_back(neuron->E_Input_inf/neuron->update_count_neuronal_inf);
    }
  }
  return standardDeviation(vector);
}

double Balanced_Network::getEInputInhSDInf(){
  vector<double> vector;
  for (int i=0;i<N_E+N_I;i++){
    Neuron* neuron = neuron_Vector[i];
    if (neuron->population == "I"){
      vector.push_back(neuron->E_Input_inf/neuron->update_count_neuronal_inf);
    }
  }
  return standardDeviation(vector);
}

double Balanced_Network::getEInputSDInf(){
  vector<double> vector;
  for (int i=0;i<N_E+N_I;i++){
    Neuron* neuron = neuron_Vector[i];
    vector.push_back(neuron->E_Input_inf/neuron->update_count_neuronal_inf);
  }
  return standardDeviation(vector);
}

double Balanced_Network::getIInputExcMean(){
  vector<double> vector;
  for (int i=0;i<N_E+N_I;i++){
    Neuron* neuron = neuron_Vector[i];
    if (neuron->population == "E"){
      vector.push_back(neuron->I_Input/neuron->update_count_neuronal);
    }
  }
  return mean(vector);
}

double Balanced_Network::getIInputInhMean(){
  vector<double> vector;
  for (int i=0;i<N_E+N_I;i++){
    Neuron* neuron = neuron_Vector[i];
    if (neuron->population == "I"){
      vector.push_back(neuron->I_Input/neuron->update_count_neuronal);
    }
  }
  return mean(vector);
}

double Balanced_Network::getIInputMean(){
  vector<double> vector;
  for (int i=0;i<N_E+N_I;i++){
    Neuron* neuron = neuron_Vector[i];
    vector.push_back(neuron->I_Input/neuron->update_count_neuronal);
  }
  return mean(vector);
}

double Balanced_Network::getIInputExcMeanInf(){
  vector<double> vector;
  for (int i=0;i<N_E+N_I;i++){
    Neuron* neuron = neuron_Vector[i];
    if (neuron->population == "E"){
      vector.push_back(neuron->I_Input_inf/neuron->update_count_neuronal_inf);
    }
  }
  return mean(vector);
}

double Balanced_Network::getIInputInhMeanInf(){
  vector<double> vector;
  for (int i=0;i<N_E+N_I;i++){
    Neuron* neuron = neuron_Vector[i];
    if (neuron->population == "I"){
      vector.push_back(neuron->I_Input_inf/neuron->update_count_neuronal_inf);
    }
  }
  return mean(vector);
}

double Balanced_Network::getIInputMeanInf(){
  vector<double> vector;
  for (int i=0;i<N_E+N_I;i++){
    Neuron* neuron = neuron_Vector[i];
    vector.push_back(neuron->I_Input_inf/neuron->update_count_neuronal_inf);
  }
  return mean(vector);
}

double Balanced_Network::getIInputExcSD(){
  vector<double> vector;
  for (int i=0;i<N_E+N_I;i++){
    Neuron* neuron = neuron_Vector[i];
    if (neuron->population == "E"){
      vector.push_back(neuron->I_Input/neuron->update_count_neuronal);
    }
  }
  return standardDeviation(vector);
}

double Balanced_Network::getIInputInhSD(){
  vector<double> vector;
  for (int i=0;i<N_E+N_I;i++){
    Neuron* neuron = neuron_Vector[i];
    if (neuron->population == "I"){
      vector.push_back(neuron->I_Input/neuron->update_count_neuronal);
    }
  }
  return standardDeviation(vector);
}

double Balanced_Network::getIInputSD(){
  vector<double> vector;
  for (int i=0;i<N_E+N_I;i++){
    Neuron* neuron = neuron_Vector[i];
    vector.push_back(neuron->I_Input/neuron->update_count_neuronal);
  }
  return standardDeviation(vector);
}

double Balanced_Network::getIInputExcSDInf(){
  vector<double> vector;
  for (int i=0;i<N_E+N_I;i++){
    Neuron* neuron = neuron_Vector[i];
    if (neuron->population == "E"){
      vector.push_back(neuron->I_Input_inf/neuron->update_count_neuronal_inf);
    }
  }
  return standardDeviation(vector);
}

double Balanced_Network::getIInputInhSDInf(){
  vector<double> vector;
  for (int i=0;i<N_E+N_I;i++){
    Neuron* neuron = neuron_Vector[i];
    if (neuron->population == "I"){
      vector.push_back(neuron->I_Input_inf/neuron->update_count_neuronal_inf);
    }
  }
  return standardDeviation(vector);
}

double Balanced_Network::getIInputSDInf(){
  vector<double> vector;
  for (int i=0;i<N_E+N_I;i++){
    Neuron* neuron = neuron_Vector[i];
    vector.push_back(neuron->I_Input_inf/neuron->update_count_neuronal_inf);
  }
  return standardDeviation(vector);
}


STLPriorityQueue<double,Neuron*>* Balanced_Network::getPQ(){
  return minimum_time_queue;
}

//addNeurons()
/*
void Balanced_Network::addNeurons(int N_E, int N_I){
  this->N_E = this->N_E + N_E;
  this->N_I = this->N_I + N_I;

  for (int i=1;i<=N_E;i++){
    Neuron* n = new Neuron(i,"E",K,externalRateFactor,phi,lambda);
    network->insertVertex(n);
  }

  for (int i=1;i<=N_I;i++){
    Neuron* n = new Neuron(i,"I",K,externalRateFactor,phi,lambda);
    network->insertVertex(n);
  }

  neuron_Vector = network->getVertices();
  //adds all the update times to a priority queue.
  for (int i=0;i<neuron_Vector.size();i++){
    Neuron* toInsert = neuron_Vector[i];
    minimum_time_queue->insert((-1*toInsert->time_to_be_updated),toInsert);
  }
}
*/

bool Balanced_Network::containsNeuron(Neuron* n){
  if (n->vectorNumber >= neuron_Vector.size()){
    return false;
  }
  else {
    return true;
  }
}


Neuron* Balanced_Network::chooseRandomNeuron(){
  if (neuron_Vector.size()==0){
    throw runtime_error("There are no neurons in the network!");
  }
  float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
  int index = r* neuron_Vector.size();
  return neuron_Vector[index];
}

/*
vector<Neuron*> Balanced_Network::getNeurons(){
  return network->getVertices();
}
*/


void Balanced_Network::insertConnection(Neuron* source, Neuron* target){
  if (isConnected(source,target)){
    throw runtime_error("The two neurons are already connected.");
  }

  string sourcePop = source->population;
  string targetPop = target->population;
  if (sourcePop == "E" && targetPop == "E"){
    Jmatrix[source->vectorNumber][target->vectorNumber] = J_EE;
  }
  else if (sourcePop == "E" && targetPop == "I"){
    Jmatrix[source->vectorNumber][target->vectorNumber] = J_IE;
  }
  else if (sourcePop == "I" && targetPop == "E"){
    Jmatrix[source->vectorNumber][target->vectorNumber] = J_EI;
  }
  else{
    Jmatrix[source->vectorNumber][target->vectorNumber] = J_II;
  }
}

void Balanced_Network::removeConnection(Neuron* source, Neuron* target){
  if (!isConnected(source,target)){
    throw runtime_error("The two neurons are not connected in the first place!");
  }
  Jmatrix[source->vectorNumber][target->vectorNumber] = 0;
}

bool Balanced_Network::isConnected(Neuron* source, Neuron* target){
  if (Jmatrix[source->vectorNumber][target->vectorNumber] != 0 ){
    return true;
  }
  else {
    return false;
  }
}

/*
vector<Neuron*> Balanced_Network::getIncomingNeurons(Neuron* n){
  return adjacencyList[n->vectorNumber];
}
*/

void Balanced_Network::initializeJmatrix(const double J_EE, const double J_EI, const double J_IE, const double J_II){
  this->J_EE = J_EE;
  this->J_EI = J_EI;
  this->J_IE = J_IE;
  this->J_II = J_II;

  //initialize a N_E+N_I square matrix
  //input paramater K is the connectivity index
  Jmatrix = new double*[int (N_E+N_I)];
  for (int i=0;i<N_E+N_I;i++){
    Jmatrix[i] = new double[int(N_E+N_I)];
  }

  for (int i=0;i<N_E+N_I;i++){
    for (int j=0;j<N_E+N_I;j++){
      Jmatrix[i][j] = 0;

      if (i!=j){
      //r is a random float in [0,1]
      float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
      //cout << "Jrand is: " + to_string(r) << endl;

      if (j<N_E && i < N_E && r <= K/N_E){
        Jmatrix[i][j] = J_EE/sqrt(K);
      }
      else if (j<N_E && i >= N_E && r <= K/N_E){
        Jmatrix[i][j] = J_IE/sqrt(K);
      }
      else if (j>=N_E && i < N_E && r <= K/N_I){
        Jmatrix[i][j] = J_EI/sqrt(K);
      }
      else if (j>=N_E && i >= N_E && r <= K/N_I){
        Jmatrix[i][j] = J_II/sqrt(K);
      }
    }
    }
  }
}

void Balanced_Network::initializeNeurons(){
  //  srand(1);

  for (int i=0;i<neuron_Vector.size();i++){
    double r=((double)rand()/(double)RAND_MAX);
    //cout << "initializeNeurons r: " + to_string(r) << endl;
    if (r <=.5){
      neuron_Vector[i]->fire();
    }
    else{
      neuron_Vector[i]->rest();
    }
  }
}

/*
void Balanced_Network::establishConnections(){
  if (neuron_Vector.size()==0){
    throw runtime_error("There are currently no neurons in the network.");
  }

  for (int i=0;i<N_E+N_I;i++){
    for (int j=0;j<N_E+N_I;j++){
      if (Jmatrix[i][j]!=0){
        network->insertEdge(neuron_Vector[j],neuron_Vector[i],"default",Jmatrix[i][j]);
      }
    }
  }
}
*/

double Balanced_Network::Heaviside(double value){
  if (value >0){
    return 1;
  }
  else {
    return 0;
  }
}

void Balanced_Network::checkActiveNeurons(double startTime, double endTime){
  if (time > startTime && time < endTime){
    //scan all the neurons and see which ones are active
    vector<Neuron*> nVector;
    for (int i=0;i<neuron_Vector.size();i++){
      if (neuron_Vector[i]->state == 1){
        nVector.push_back(neuron_Vector[i]);
      }
    }
    pair<double, vector<Neuron*>> toAdd(time,nVector);
    activeNeurons.push_back(toAdd);
  }
}



void Balanced_Network::update(Neuron* neuron_to_record){
  if (neuron_Vector.size()==0){
    throw runtime_error("There are currently no neurons in the network.");
  }
  //choose the minimum one to update(Max of the negatives)
  update_count++;

  Neuron* neuron_to_update = minimum_time_queue->removeMax();

  //update the network time
  time = neuron_to_update->time_to_be_updated;

  //update the neuron time
  neuron_to_update->update_time_to_be_updated();

  //insert the new update time into the priority queue
  //The queue always has the number of elements equal to
  //the number of neurons.
  minimum_time_queue->insert((-1*neuron_to_update->time_to_be_updated),neuron_to_update);


  //for mean Threshold
  if (neuron_to_update->population == "E"){
    pair<double,double> dataToPush(time,
      meanExcThresholdTimeSeries.back().second - neuron_to_update->threshold);
      meanExcThresholdTimeSeries.push_back(dataToPush);
  }
  else {
    pair<double,double> dataToPush(time,
      meanInhThresholdTimeSeries.back().second - neuron_to_update->threshold);
      meanInhThresholdTimeSeries.push_back(dataToPush);
  }

  //SFA stuff - decay
  double timeElapsed = time - neuron_to_update->last_update_time;

  neuron_to_update->threshold = neuron_to_update->original_threshold +
  (neuron_to_update->threshold - neuron_to_update->original_threshold)*
  exp(-neuron_to_update->decay_constant*timeElapsed);

  neuron_to_update->last_update_time = time;


  //reset the total input to the neuron being updated to zero.
  neuron_to_update->totalInput = 0;
  neuron_to_update->totalExcitatoryInput =0;
  neuron_to_update->totalInhibitoryInput =0;


  //vector<Edge<Neuron*,string,double> > incomingConnections = network->getIncomingEdges(neuron_to_update);
    //the for-loop sums over all the connected neurons
  for (int j=0;j<N_E+N_I;j++){
    double strength = Jmatrix[neuron_to_update->vectorNumber][j];
    double state = neuron_Vector[j]->state;
    //neuron_to_update->totalInput += strength * state;
    if (strength > 0){
      neuron_to_update->totalExcitatoryInput += (strength*state);
    }
    else if (strength <0){
      neuron_to_update->totalInhibitoryInput += (strength*state);
    }
  }

    //below takes care of externalInput and threshold
    neuron_to_update->totalExcitatoryInput += neuron_to_update->externalInput;
    neuron_to_update->totalInput = neuron_to_update->totalExcitatoryInput +
    neuron_to_update->totalInhibitoryInput;

    //the neuron fires if the totalInput is above zero, rests otherwise.
    neuron_to_update->previous_state = neuron_to_update->state;

    neuron_to_update->state =
    Heaviside(neuron_to_update->totalInput- neuron_to_update->threshold);

    //SFA stuff - jump
    neuron_to_update->threshold += neuron_to_update->state*
    neuron_to_update->adaptation_jump;
    neuron_to_update->thresholdVector.push_back(neuron_to_update->threshold);

    //for mean Threshold
    if (neuron_to_update->population == "E"){
      meanExcThresholdTimeSeries.back().second += neuron_to_update->threshold;
    }
    else {
      meanInhThresholdTimeSeries.back().second += neuron_to_update->threshold;
    }
    //cout << meanExcThresholdTimeSeries.back().second << endl;

    //EM plot stuff(sixth plot)
    //neuron_to_update->stateSum += neuron_to_update->state;

    //EI ratio stuff(second plot)
    if (neuron_to_update->totalInhibitoryInput!=0){
      neuron_to_update->EI_Ratio += (neuron_to_update->totalExcitatoryInput/
      neuron_to_update->totalInhibitoryInput);
      neuron_to_update->update_count++;
    }
    /*
    neuron_to_update->EI_Ratio_Exc += neuron_to_update->totalExcitatoryInput;
    neuron_to_update->EI_Ratio_Inh += neuron_to_update->totalInhibitoryInput;
    */

    //ISI stuff
    if (neuron_to_update->previous_state ==0 &&
    neuron_to_update->state ==1){
      if (neuron_to_update->last_spike_time ==0){
        neuron_to_update->last_spike_time = time;
        //record this as a time in which a spike happened
        neuron_to_update->spike_times.push_back(time);
      }
      else {
        neuron_to_update->current_spike_time = time;
        double ISI_time = neuron_to_update->current_spike_time -
        neuron_to_update->last_spike_time;
        neuron_to_update->ISI_data.push_back(ISI_time);
        neuron_to_update->last_spike_time = time;
        //record this as a time in which a spike happened
        neuron_to_update->spike_times.push_back(time);
      }
    }

    //SFA stuff
    //neuron_to_update->updateThresholdDiscrete(true, timeElapsed);
    //neuron_to_update->last_update_time = time;




    //studying the plateau region, not really necessary now.
    //checkActiveNeurons(99,100);

    //cout << neuron_Vector[94]->state << endl;


    //Mean Activity stuff
    //this says "if the updated neuron went from rest to active"
    if (neuron_to_update->previous_state ==0 &&
    neuron_to_update->state ==1){
      if (neuron_to_update->population == "E"){
        pair<double,double> data(time,
          excitatoryActivityTimeSeries.back().second+1);
        excitatoryActivityTimeSeries.push_back(data);
        excUpdateCount++;
      }
      else if (neuron_to_update->population == "I"){
        pair<double,double> data(time,
          inhibitoryActivityTimeSeries.back().second+1);
        inhibitoryActivityTimeSeries.push_back(data);
        inhUpdateCount++;
      }
    }
    //this says "else if the updated neuron went from active to rest"
    else if (neuron_to_update->previous_state ==1 &&
    neuron_to_update->state ==0){
      if (neuron_to_update->population == "E"){
        pair<double,double> data(time,
          excitatoryActivityTimeSeries.back().second-1);
        excitatoryActivityTimeSeries.push_back(data);
        excUpdateCount++;
      }
      else if (neuron_to_update->population == "I"){
        pair<double,double> data(time,
          inhibitoryActivityTimeSeries.back().second-1);
        inhibitoryActivityTimeSeries.push_back(data);
        inhUpdateCount++;
      }
    }
    else{
      if (neuron_to_update->population == "E"){
        pair<double,double> data(time,
          excitatoryActivityTimeSeries.back().second);
        excitatoryActivityTimeSeries.push_back(data);
        excUpdateCount++;
      }
      else if (neuron_to_update->population == "I"){
        pair<double,double> data(time,
          inhibitoryActivityTimeSeries.back().second);
        inhibitoryActivityTimeSeries.push_back(data);
        inhUpdateCount++;
      }
    }


    if (neuron_to_record == neuron_to_update){
      //record the totalInput for the target neuron only if it's
      //the one being updated
      //data has the format <time, total input>
      pair<double,double> data_total(time,neuron_to_record->totalInput);
      pair<double,double> data_exc(time,neuron_to_record->totalExcitatoryInput);
      pair<double,double> data_inh(time,neuron_to_record->totalInhibitoryInput);

      record(data_total,data_exc,data_inh,neuron_to_record);
    }
}


void Balanced_Network::update2(){
  if (neuron_Vector.size()==0){
    throw runtime_error("There are currently no neurons in the network.");
  }
  update_count++;
  //choose the minimum one to update(Max of the negatives)
  Neuron* neuron_to_update = minimum_time_queue->removeMax();
  neuron_to_update->update_count_neuronal++;
  //cout << "Neuron Number: " + to_string(neuron_to_update->vectorNumber) << endl;
  //cout << "Update Count: " + to_string(neuron_to_update->update_count_neuronal) <<endl;
  //cout << "Update Count inf: " + to_string(neuron_to_update->update_count_neuronal_inf) <<endl;


  //update the network time
  time = neuron_to_update->time_to_be_updated;

  //update the neuron time
  neuron_to_update->update_time_to_be_updated();

  //insert the new update time into the priority queue
  //The queue always has the number of elements equal to
  //the number of neurons.
  minimum_time_queue->insert((-1*neuron_to_update->time_to_be_updated),neuron_to_update);


  //for mean Threshold
  if (neuron_to_update->population == "E"){
    pair<double,double> dataToPush(time,
      meanExcThresholdTimeSeries.back().second - neuron_to_update->threshold);
      meanExcThresholdTimeSeries.push_back(dataToPush);
  }
  else {
    pair<double,double> dataToPush(time,
      meanInhThresholdTimeSeries.back().second - neuron_to_update->threshold);
      meanInhThresholdTimeSeries.push_back(dataToPush);
  }

  //The other way of averaging - time average for a single neuron first.
  neuron_to_update->thresholdSum+=neuron_to_update->threshold;
  if (update_count > update_times - inf_size){
    neuron_to_update->thresholdSum_inf+=neuron_to_update->threshold;
  }


  //SFA stuff - decay
  double timeElapsed = time - neuron_to_update->last_update_time;

  neuron_to_update->threshold = neuron_to_update->original_threshold +
  (neuron_to_update->threshold - neuron_to_update->original_threshold)*
  exp(-neuron_to_update->decay_constant*timeElapsed);

  neuron_to_update->last_update_time = time;


  //reset the total input to the neuron being updated to zero.
  neuron_to_update->totalInput = 0;
  neuron_to_update->totalExcitatoryInput =0;
  neuron_to_update->totalInhibitoryInput =0;


  //vector<Edge<Neuron*,string,double> > incomingConnections = network->getIncomingEdges(neuron_to_update);
    //the for-loop sums over all the connected neurons
  for (int j=0;j<N_E+N_I;j++){
    double strength = Jmatrix[neuron_to_update->vectorNumber][j];
    double state = neuron_Vector[j]->state;
    //neuron_to_update->totalInput += strength * state;
    if (strength > 0){
      neuron_to_update->totalExcitatoryInput += (strength*state);
    }
    else if (strength <0){
      neuron_to_update->totalInhibitoryInput += (strength*state);
    }
  }

    //below takes care of externalInput and threshold
    neuron_to_update->totalExcitatoryInput += neuron_to_update->externalInput;
    neuron_to_update->totalInput = neuron_to_update->totalExcitatoryInput +
    neuron_to_update->totalInhibitoryInput;

    //the neuron fires if the totalInput is above zero, rests otherwise.
    neuron_to_update->previous_state = neuron_to_update->state;

    neuron_to_update->state =
    Heaviside(neuron_to_update->totalInput- neuron_to_update->threshold);

    //SFA stuff - jump
    neuron_to_update->threshold += neuron_to_update->state*
    neuron_to_update->adaptation_jump;
    neuron_to_update->thresholdVector.push_back(neuron_to_update->threshold);

    //for mean Threshold
    if (neuron_to_update->population == "E"){
      meanExcThresholdTimeSeries.back().second += neuron_to_update->threshold;
    }
    else {
      meanInhThresholdTimeSeries.back().second += neuron_to_update->threshold;
    }
    //cout << meanExcThresholdTimeSeries.back().second << endl;

    //EM plot stuff(sixth plot)
    //neuron_to_update->stateSum += neuron_to_update->state;

    //EI ratio stuff(second plot)
    if (neuron_to_update->totalInhibitoryInput!=0){
      neuron_to_update->EI_Ratio += (neuron_to_update->totalExcitatoryInput/
      neuron_to_update->totalInhibitoryInput);
      neuron_to_update->update_count++;
      if (update_count > update_times - inf_size){
        neuron_to_update->EI_Ratio_inf+= (neuron_to_update->totalExcitatoryInput/
        neuron_to_update->totalInhibitoryInput);
        neuron_to_update->update_count_inf++;
        //cout << "Ratio: " + to_string(neuron_to_update->EI_Ratio_inf) << endl;
        //cout << "Count: " + to_string(neuron_to_update->update_count_inf) << endl;
      }
    }
    /*
    neuron_to_update->EI_Ratio_Exc += neuron_to_update->totalExcitatoryInput;
    neuron_to_update->EI_Ratio_Inh += neuron_to_update->totalInhibitoryInput;
    */

    //ISI stuff
    if (neuron_to_update->previous_state ==0 &&
    neuron_to_update->state ==1){
      if (neuron_to_update->last_spike_time ==0){
        neuron_to_update->last_spike_time = time;
        //record this as a time in which a spike happened
        neuron_to_update->spike_times.push_back(time);
      }
      else {
        neuron_to_update->current_spike_time = time;
        double ISI_time = neuron_to_update->current_spike_time -
        neuron_to_update->last_spike_time;
        neuron_to_update->ISI_data.push_back(ISI_time);
        neuron_to_update->last_spike_time = time;
        //record this as a time in which a spike happened
        neuron_to_update->spike_times.push_back(time);
      }
    }

    //SFA stuff
    //neuron_to_update->updateThresholdDiscrete(true, timeElapsed);
    //neuron_to_update->last_update_time = time;

    //studying the plateau region, not really necessary now.
    //checkActiveNeurons(99,100);

    //cout << neuron_Vector[94]->state << endl;

    //Mean Activity stuff
    //this says "if the updated neuron went from rest to active"
    if (neuron_to_update->previous_state ==0 &&
    neuron_to_update->state ==1){
      if (neuron_to_update->population == "E"){
        pair<double,double> data(time,
          excitatoryActivityTimeSeries.back().second+1);
        excitatoryActivityTimeSeries.push_back(data);
        excUpdateCount++;
      }
      else if (neuron_to_update->population == "I"){
        pair<double,double> data(time,
          inhibitoryActivityTimeSeries.back().second+1);
        inhibitoryActivityTimeSeries.push_back(data);
        inhUpdateCount++;
      }
    }
    //this says "else if the updated neuron went from active to rest"
    else if (neuron_to_update->previous_state ==1 &&
    neuron_to_update->state ==0){
      if (neuron_to_update->population == "E"){
        pair<double,double> data(time,
          excitatoryActivityTimeSeries.back().second-1);
        excitatoryActivityTimeSeries.push_back(data);
        excUpdateCount++;
      }
      else if (neuron_to_update->population == "I"){
        pair<double,double> data(time,
          inhibitoryActivityTimeSeries.back().second-1);
        inhibitoryActivityTimeSeries.push_back(data);
        inhUpdateCount++;
      }
    }
    else{
      if (neuron_to_update->population == "E"){
        pair<double,double> data(time,
          excitatoryActivityTimeSeries.back().second);
        excitatoryActivityTimeSeries.push_back(data);
        excUpdateCount++;
      }
      else if (neuron_to_update->population == "I"){
        pair<double,double> data(time,
          inhibitoryActivityTimeSeries.back().second);
        inhibitoryActivityTimeSeries.push_back(data);
        inhUpdateCount++;
      }
    }

    //Record total input
    pair<double,double> data_total(time,neuron_to_update->totalInput);
    pair<double,double> data_exc(time,neuron_to_update->totalExcitatoryInput);
    pair<double,double> data_inh(time,neuron_to_update->totalInhibitoryInput);


    //Populational SD
    neuron_to_update->totalInput_Sum+=neuron_to_update->totalInput-neuron_to_update->threshold;
    neuron_to_update->E_Input+=neuron_to_update->totalExcitatoryInput;
    neuron_to_update->I_Input+=neuron_to_update->totalInhibitoryInput;

    if (update_count > update_times - inf_size){
      neuron_to_update->totalInput_Sum_inf+=neuron_to_update->totalInput-neuron_to_update->threshold;
      neuron_to_update->E_Input_inf+=neuron_to_update->totalExcitatoryInput;
      neuron_to_update->I_Input_inf+=neuron_to_update->totalInhibitoryInput;
      neuron_to_update->update_count_neuronal_inf++;
      if(neuron_to_update->population == "I"){
        //cout << "E_Input_inf: " + to_string(neuron_to_update->E_Input_inf) <<endl;
      }
    }

    if (neuron_to_update->population == "E"){
      totalInput_exc.push_back(neuron_to_update->totalInput-neuron_to_update->threshold);
      totalInput_exc_timeSeries.push_back(data_total);
    }
    else if (neuron_to_update->population == "I"){
      totalInput_inh.push_back(neuron_to_update->totalInput-neuron_to_update->threshold);
      totalInput_inh_timeSeries.push_back(data_total);
    }

    //cout << "update_count_neuronal: " + to_string(neuron_to_update->update_count_neuronal);

}



void Balanced_Network::addEI_Ratios(){
  for (int i=0;i<neuron_Vector.size();i++){
    EI_Ratio_Collection.push_back(neuron_Vector[i]->EI_Ratio /
      neuron_Vector[i]->update_count);
    EI_Ratio_Collection_inf.push_back(neuron_Vector[i]->EI_Ratio_inf
      / neuron_Vector[i]->update_count_inf);
  }

  for (int i=0;i<N_E;i++){
    EI_Ratio_Collection_Exc.push_back(neuron_Vector[i]->EI_Ratio
      / neuron_Vector[i]->update_count);
    EI_Ratio_Collection_Exc_inf.push_back(neuron_Vector[i]->EI_Ratio_inf
      / neuron_Vector[i]->update_count_inf);
  }

  for (int i=N_E;i<neuron_Vector.size();i++){
    EI_Ratio_Collection_Inh.push_back(neuron_Vector[i]->EI_Ratio
    / neuron_Vector[i]->update_count);
    EI_Ratio_Collection_Inh_inf.push_back(neuron_Vector[i]->EI_Ratio_inf
    / neuron_Vector[i]->update_count_inf);
  }
}


//private methods.

/*
void Balanced_Network::insertNeuron(Neuron* n){
  network->insertVertex(n);
}


void Balanced_Network::removeNeuron(Neuron* n){
  network->removeVertex(n);
}
*/

void Balanced_Network::record(pair<double,double> totalInput_data, pair<double,double> excitatoryInput_data,
  pair<double,double> inhibitoryInput_data, Neuron* neuron_to_record){
  neuron_to_record->addInputData(totalInput_data, excitatoryInput_data,inhibitoryInput_data);
}

void Balanced_Network::deleteNeurons(){
  for (int i=0;i<neuron_Vector.size();i++){
    delete neuron_Vector[i];
  }
}

void Balanced_Network::deleteJmatrix(){
  for (int i=0;i<N_I+N_E;i++){
    delete[] Jmatrix[i];
  }
  delete[] Jmatrix;
}
