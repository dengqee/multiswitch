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

	uint32_t measureNodeNum=13;
	

	//求解放置问题
	PlasementProblem plasementProblem(geant,measureNodeNum);
	plasementProblem.run();
	plasementProblem.Print();

	//设置代价函数
	vector<double>piece={0, 0.33, 0.67, 0.9, 1};
	vector<double>slop={1, 3, 10, 70, 500};
	shared_ptr<PiecewiseFunc>costFun(new PiecewiseFunc(piece,slop,0));

	//求解任务分配问题
	string assignFileName=dir+"assignment_"+topoName+"_"+to_string(measureNodeNum)+"_x.txt";
	MeasureAssignmentProblem assign(geant,costFun);
	assign.SetLambda0(vector<double>(measureNodeNum,1));
	assign.SetObjNum(10);
	assign.SetLambdaStepLength(0.1);
	assign.SetMu0(vector<double>(measureNodeNum,10));
	assign.SetObjDualNum(20);
	assign.SetMuStepLength(0.0001);
	assign.SetStopIterCon(5,5);
	cout<<"***********run*******************"<<endl;
	cout<<"run..."<<endl;
	assign.run();
	cout<<"********result*******************"<<endl;
	assign.Print();
//	assign.OutPut(assignFileName);





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
