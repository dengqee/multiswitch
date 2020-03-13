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
	string topoName="BA50";
	string topoFileName=dir+"Topology_"+topoName+".txt";

	string flowFileName=dir+topoName+"flow.txt"; 

	string pathFileName=dir+topoName+"Path.txt";


	shared_ptr<Network> geant(new Network(topoFileName,flowFileName));
	geant->GenFlowPath();

	uint32_t measureNodeNum=40;
	

	//求解放置问题
	PlasementProblem plasementProblem(geant,measureNodeNum);
	plasementProblem.run();
	plasementProblem.Print();
	string measureNodeFile="/home/dengqi/eclipse-workspace/ElasticSketchCode/data/multiswitch/packet/"+to_string(measureNodeNum)+"/measureNode.txt";
	plasementProblem.OutPut(measureNodeFile);//输出测量节点

	//设置代价函数
	vector<double>piece={0, 0.33, 0.67, 0.9, 1};
	vector<double>slop={1, 3, 10, 70, 500};
	shared_ptr<PiecewiseFunc>costFun(new PiecewiseFunc(piece,slop,0));

	//求解任务分配问题
	string assignFileName=dir+"assignment_"+topoName+"_"+to_string(measureNodeNum)+"_x.txt";
	MeasureAssignmentProblem assign(geant,costFun);
	assign.SetLambda0(vector<double>(measureNodeNum,5000));
	assign.SetObjNum(10);
	assign.SetLambdaStepLength(0.1);
	assign.SetMu0(vector<vector<double> >(slop.size(),vector<double>(measureNodeNum,100)));
	assign.SetObjDualNum(10);
	assign.SetMuStepLength(0.0001);
	assign.SetStopIterCon(50,50);
	string datFileName="./cplex/problem3/problem3.dat";
	assign.OutPutCplexDat(datFileName);


	//输出路由规划需要的dat文件
	string outdir="./cplex/problem3/problem4_dat/";
	string indir="./data/BA50_days/"+to_string(measureNodeNum)+"/est/"; 
	vector<map<uint32_t,uint32_t> >allflow_day;//每天的测量流量矩阵,累加
	vector<map<uint32_t,uint32_t> >allflow_day_real;//每天真实流量矩阵，累加
	vector<map<uint32_t,uint32_t> >tcam_day;//每天TCAM中记录的流
	vector<map<uint32_t,vector<vector<uint32_t> > > >linkpaths_day;//每天每条流的可用链路路径
	vector<map<uint32_t,uint32_t> >Pi_day;//每天每条流的备选路径
	vector<vector<vector<vector<uint32_t> > > >delt_day;//每天的delt

	assign.OutPutCplexDat_rout(outdir,indir,allflow_day,allflow_day_real,tcam_day,linkpaths_day,Pi_day,delt_day);
	assign.Greedy_route(outdir,allflow_day,allflow_day_real,tcam_day,linkpaths_day,Pi_day,delt_day);
	assign.Random_route(outdir,allflow_day,allflow_day_real,tcam_day,linkpaths_day,Pi_day,delt_day);
	assign.Original_route(outdir,allflow_day,allflow_day_real,tcam_day,linkpaths_day,Pi_day,delt_day);
	

	return 0;
}
