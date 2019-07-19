/*************************************************************************
  > File Name: main.cpp
  > Author: dengqi
  > Mail: 906262038@qq.com 
  > Created Time: 2019年07月05日 星期五 15时56分50秒
 ************************************************************************/

#include<iostream>
#include"topology.h"
#include"network.h"
#include<string>
#include"plasement-problem.h"
#include"measure-assignment-problem.h"

using namespace std;
int main()
{
	string dir="/home/dengqi/project5/Thesis/multiswitch/data/";
	string topoName="Geant";
	string topoFileName=dir+"Topology_"+topoName+".txt";

	string flowFileName=dir+topoName+"flow.txt"; 

	string pathFileName=dir+topoName+"Path.txt";

	shared_ptr<Network> geant(new Network(topoFileName,flowFileName));
	geant->GenFlowPath();

	PlasementProblem plasementProblem(geant,13);
	plasementProblem.run();
	plasementProblem.Print();

	MeasureAssignmentProblem assign(geant);
	assign.SetLambda0(2550);
	assign.SetObjNum(20);
	assign.SetLambdaStepLength(100);
	assign.SetMu0(vector<double>(13,1));
	assign.SetObjDualNum(20);
	assign.SetMuStepLength(0.000001);
	assign.SetStopIterCon(10);
	cout<<"***********run*******************"<<endl;
	assign.run();




	/*
	   Topology geant(topoFileName);
	   geant.ShortestPathGen();
	//geant.WritePath(pathFileName);
	geant.ReadPath(pathFileName);

	vector<uint32_t> path;
	if(geant.GetPath(231,10,path))
	for(auto i:path)
	cout<<i<<" ";
	else
	cout<<"can't find path"<<endl;
	*/


	return 0;
}
