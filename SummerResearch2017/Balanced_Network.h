#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <utility>

//#include "GraphLab/adjacencyListGraph.h"
#include "Neuron.h"
#include "GraphLab/stl/stlPriorityQueue.h"
#include "Statistics.h"

using namespace std;

class Balanced_Network{

public:
  Balanced_Network(double N_E, double N_I, double K,
    double J_EE, double J_EI, double J_IE, double J_II, double m_0,
     double externalRateFactor, double phi, double lambda,double update_times);

  ~Balanced_Network();

  //The accessor methods

  //gets threshold for a single neuron
  double getThreshold(Neuron* n);

  //gets externalInput for a single neuron
  double getExternalInput(Neuron* n);

  //gets total input for a single neuron
  double getTotalInput(Neuron* n);

  //gets N_E and N_I
  double getN_E();
  double getN_I();
  double getK();

  //gets the Jmatrix
  double** getJmatrix();

  //gets the current time for the network.
  double getTime();

  //EI Ratio
  vector<double> getEI_Ratios();
  vector<double> getExcEI_Ratios();
  vector<double> getInhEI_Ratios();
  vector<double> getEI_Ratios_inf();
  vector<double> getExcEI_Ratios_inf();
  vector<double> getInhEI_Ratios_inf();

  //Mean Activity
  vector<pair<double,double>> getExcMeanAtv();
  vector<pair<double,double>> getInhMeanAtv();
  vector<pair<double,double>> getMeanExcThresholdTimeSeries();
  vector<pair<double,double>> getMeanInhThresholdTimeSeries();
  //Not used in the simulation
  pair<double,double> getEM_data_exc();
  pair<double,double> getEM_data_inh();
  //Used in the simulation
  pair<double,double> getEM_data_exc2();
  pair<double,double> getEM_data_inh2();
  double getEM_data_exc_sd();
  double getEM_data_inh_sd();
  double getM_exc_inf();
  double getM_inh_inf();
  double getM_exc_sd_inf();
  double getM_inh_sd_inf();

  //Threshold
  double getMeanExcThreshold();
  double getMeanInhThreshold();
  double getMeanThreshold();
  double getExcThresholdSD();
  double getInhThresholdSD();
  double getThresholdSD();
  double getTheta_exc_inf();
  double getTheta_inh_inf();
  double getTheta_inf();
  double getTheta_exc_inf_SD();
  double getTheta_inh_inf_SD();
  double getTheta_inf_SD();

  //Total Input
  double getTotalInputExcMean();
  double getTotalInputInhMean();
  double getTotalInputExcSD();
  double getTotalInputInhSD();
  double getTotalInputExcSDInf();
  double getTotalInputInhSDInf();

  //E Input
  double getEInputExcMean();
  double getEInputInhMean();
  double getEInputMean();
  double getEInputExcMeanInf();
  double getEInputInhMeanInf();
  double getEInputMeanInf();
  double getEInputExcSD();
  double getEInputInhSD();
  double getEInputSD();
  double getEInputExcSDInf();
  double getEInputInhSDInf();
  double getEInputSDInf();

  //I Input
  double getIInputExcMean();
  double getIInputInhMean();
  double getIInputMean();
  double getIInputExcMeanInf();
  double getIInputInhMeanInf();
  double getIInputMeanInf();
  double getIInputExcSD();
  double getIInputInhSD();
  double getIInputSD();
  double getIInputExcSDInf();
  double getIInputInhSDInf();
  double getIInputSDInf();


  //gets the PQ for the update method.
  //For debugging purpose mostly, can saftely ignore.
  STLPriorityQueue<double,Neuron*>* getPQ();


  //The Neuron methods

  //adds N_E excitatory neurons and N_I inhibitory neurons
  //into the network.
  //void addNeurons(int N_E, int N_I);

  //check if a neuron is in the network, returns true iff it is.
  bool containsNeuron(Neuron* n);

  //choose a random neuron in the network.
  Neuron* chooseRandomNeuron();

  //get all the neurons in the network and put them into a
  //vector
  //vector<Neuron*> getNeurons();



  //The Connection methods

  //insert a connection between two neurons.
  //paramaters: the source and target neuron, the label (can just be
  //empty), and the connection strength.
  void insertConnection(Neuron* source, Neuron* target);

  //remove a connection between two neurons
  void removeConnection(Neuron* source, Neuron* target);

  //check if two neurons are connected.
  //returns true iff there is a connection established
  //from the source neuron onto the target neuron.
  bool isConnected(Neuron* source, Neuron* target);

  //get all the incoming connections to a neuron (all the
  //connections with that neuron as the target)
  //vector<Edge<Neuron*,string,double> > getIncomingConnections(Neuron* n);


  //Initialization methods

  //randomly generate the J matrix with connectivity index K
  void initializeJmatrix(double J_EE, double J_EI, double J_IE, double J_II);

  //randomly set the states of all the neurons in the network
  //to either 1 or 0
  void initializeNeurons();

  //establish connections between neurons based on the J matrix.
  //on average, each neuron should have incoming connections
  //from K excitatory and K inhibitory neurons.
  //void establishConnections();

  //The Heaviside function, returns 1 if the input value >0,
  //0 otherwise
  double Heaviside(double value);


  void checkActiveNeurons(double startTime, double endTime);

  //Update the network once
  //We can change the neuron_to_record to a vector of neurons
  void update(Neuron* neuron_to_record);

  void update2();

  //The same update function, but only records the data
  //for the EM plot.
  //Outdated now.
  //void update2();
  //void update3();

  void addEI_Ratios();

  double update_times;
  double update_count;

  vector<Neuron*> neuron_Vector;

  double lambda;
  double phi;

  double m_inf;
  double theta_inf;

  //at time pair.first, we have the active neurons
  //to be the pair.second
  vector<pair<double,vector<Neuron*>>> activeNeurons;
  vector<double> totalInput_exc;
  vector<double> totalInput_inh;
  vector<pair<double,double>> totalInput_exc_timeSeries;
  vector<pair<double,double>> totalInput_inh_timeSeries;


private:
  //helper methods
  //void insertNeuron(Neuron* n);
  //void removeNeuron(Neuron* n);
  void record(pair<double,double> totalInput_data, pair<double,double> excitatoryInput_data,
    pair<double,double> inhibitoryInput_data, Neuron* neuron_to_record);

  //deletor methods
  void deleteNeurons();
  void deleteJmatrix();

  //Graph<Neuron*,string,double>* network;
  double** Jmatrix;
  //vector<vector<Neuron*>> adjacencyList;

  double inf_size;

  //The number of excitatory and inhibitory neurons
  double N_E;
  double N_I;

  double K;
  double m_0;
  double J_EE;
  double J_EI;
  double J_IE;
  double J_II;

  double externalRateFactor;

  double time;
  //vector<double> time_vector;
  STLPriorityQueue<double,Neuron*>* minimum_time_queue;



  //EI ratio and ISI recording
  vector<double> EI_Ratio_Collection;
  vector<double> EI_Ratio_Collection_Exc;
  vector<double> EI_Ratio_Collection_Inh;
  vector<double> EI_Ratio_Collection_inf;
  vector<double> EI_Ratio_Collection_Exc_inf;
  vector<double> EI_Ratio_Collection_Inh_inf;

  vector<double> network_ISI;
  vector<pair<double,double>> excitatoryActivityTimeSeries;
  vector<pair<double,double>> inhibitoryActivityTimeSeries;

  double excStateSum=0;
  double inhStateSum=0;
  double excUpdateCount =0;
  double inhUpdateCount =0;
  int num_update_done;

  //Threshold Adaptation recording
  vector<pair<double,double>> meanExcThresholdTimeSeries;
  vector<pair<double,double>> meanInhThresholdTimeSeries;
};
