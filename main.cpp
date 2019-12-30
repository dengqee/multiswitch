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
#include<stdlib.h>

using namespace std;
int main()
{
	string dir="/home/dengqi/project5/Thesis/multiswitch/data/";
	string topoName="BA50";
	string topoFileName=dir+"Topology_"+topoName+".txt";

	string flowFileName=dir+topoName+"flow.txt"; 

	string pathFileName=dir+topoName+"Path.txt";


	shared_ptr<Network> net(new Network(topoFileName,flowFileName));
	net->GenFlowPath();
	net->m_topo->WritePath("data/BA50Path.txt");

	uint32_t measureNodeNum=40;
	uint32_t rand_seed=1000;//1-10为随机种子，1000-10000为节点容量
	

	//求解放置问题
	PlasementProblem plasementProblem(net,measureNodeNum);
	plasementProblem.run();
	plasementProblem.Print();
	string measureNodeFile="/home/dengqi/eclipse-workspace/ElasticSketchCode/data/multiswitch/packet/"+to_string(measureNodeNum)+"/measureNode.txt";
	plasementProblem.OutPut(measureNodeFile);//输出测量节点
/*************没有用****************/
	//设置代价函数
	vector<double>piece={0, 0.33, 0.67, 0.9, 1};
	vector<double>slop={1, 3, 10, 70, 500};
	shared_ptr<PiecewiseFunc>costFun(new PiecewiseFunc(piece,slop,0));

	//求解任务分配问题
	string assignFileName=dir+"assignment_"+topoName+"_"+to_string(measureNodeNum)+"_x.txt";
	MeasureAssignmentProblem assign(net,costFun);
	assign.SetLambda0(vector<double>(measureNodeNum,5000));
	assign.SetObjNum(10);
	assign.SetLambdaStepLength(0.1);
	assign.SetMu0(vector<vector<double> >(slop.size(),vector<double>(measureNodeNum,100)));
	assign.SetObjDualNum(10);
	assign.SetMuStepLength(0.0001);
	assign.SetStopIterCon(50,50);
/*********************************/

	assign.SetNodeCapacity(rand_seed);//设置节点容量
	string datFileName="./cplex/problem3/problem3.dat";
	assign.OutPutCplexDat(datFileName);


	//运行Cplex求解problem3,根据求解得到的x，分配Geant_packets到每个测量节点上
	char input;
	while(true)
	{
		cout<<"运行Cplex求解problem3,之后输入c,继续运行:";
		cin>>input;
		if(input=='c')
			break;
	}
	string resCplex="./cplex/problem3/Nmax_phy_load_x.txt";
	assign.ReadCplexResult(resCplex);
	string packetFile="./data/BA50_packets.txt";
/*
	dir="/home/dengqi/eclipse-workspace/ElasticSketchCode/data/multiswitch/packet/"+to_string(measureNodeNum)+"/packets_original/";//original

	string md="mkdir -p "+dir;
	system(md.c_str());

	assign.OutPutPacketOnMeasureNode_ori(packetFile,dir);
	dir="/home/dengqi/eclipse-workspace/ElasticSketchCode/data/multiswitch/packet/"+to_string(measureNodeNum)+"/packets_random/";//random

	md="mkdir -p "+dir;
	system(md.c_str());

	assign.OutPutPacketOnMeasureNode_ran(packetFile,dir);
	
*/	
	dir="/home/dengqi/eclipse-workspace/ElasticSketchCode/data/multiswitch/packet/"+to_string(measureNodeNum)+"/packets_balanced/";//无容量约束的完全负载均衡
	//dir="/home/dengqi/eclipse-workspace/ElasticSketchCode/data/multiswitch/packet/"+to_string(measureNodeNum)+"/packets_subbalanced/"+to_string(rand_seed)+"/";//有容量约束的负载均衡
	string md="mkdir -p "+dir;
	system(md.c_str());
	assign.OutPutPacketOnMeasureNode(packetFile,dir);//输出经过负载均衡的测量分配方案



	/*
	cout<<"***********run*******************"<<endl;
	cout<<"run..."<<endl;
	assign.run();
	cout<<"********result*******************"<<endl;
	assign.Print();
	*/
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
