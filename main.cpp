/*************************************************************************
  > File Name: main.cpp
  > Author: dengqi
  > Mail: 906262038@qq.com 
  > Created Time: 2019年07月05日 星期五 15时56分50秒
 ************************************************************************/

#include<iostream>
#include<cstdlib>
#include"topology.h"
#include"network.h"
#include<string>
#include"plasement-problem.h"
#include"measure-assignment-problem.h"

using namespace std;
int main(int argc,char*argv[])
{
	string dir="/home/dengqi/project5/Thesis/multiswitch/data/";
	string topoName="Geant";
	string topoFileName=dir+"Topology_"+topoName+".txt";

	string flowFileName=dir+topoName+"flow.txt"; 

	string pathFileName=dir+topoName+"Path.txt";


	shared_ptr<Network> geant(new Network(topoFileName,flowFileName));
	geant->GenFlowPath();

	uint32_t measureNodeNum;
	
	if(argc>1)
		measureNodeNum=atoi(argv[1]);
	else 
		measureNodeNum=16;

	PlasementProblem plasementProblem(geant,measureNodeNum);
	plasementProblem.run();
	plasementProblem.Print();

	string assignFileName=dir+"assignment_"+topoName+"_"+to_string(measureNodeNum)+"_x.txt";
	MeasureAssignmentProblem assign(geant);
	assign.SetLambda0(3000);
	assign.SetObjNum(10);
	assign.SetLambdaStepLength(1);
	assign.SetMu0(vector<double>(measureNodeNum,10));
	assign.SetObjDualNum(20);
	assign.SetMuStepLength(0.000001);
	assign.SetStopIterCon(5,10);
	cout<<"***********run*******************"<<endl;
	cout<<"rum..."<<endl;
	assign.run();
	cout<<"********result*******************"<<endl;
	assign.Print();
	assign.OutPut(assignFileName);





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
