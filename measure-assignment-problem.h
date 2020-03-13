/*************************************************************************
	> File Name: measure-assignment-problem.h
	> Author: dengqi
	> Mail: 906262038@qq.com 
	> Created Time: 2019年07月10日 星期三 10时48分55秒
 ************************************************************************/
#ifndef MEASURE_ASSIGNMENT_PROBLEM_H
#define MEASURE_ASSIGNMENT_PROBLEM_H

#include<iostream>
#include"problem-base.h"
#include<algorithm>

using namespace std;

#define THREAD_LOG 0//print thread log
#define LAMBDA_LOG 1
#define MU_LOG 1

#define CAP_ERR 0.1//计算节点容量时的平均误差
#define E 2.718 //自然对数的底数

#define RAND_SEED 1//随机种子
#define TCAM 1000

bool cmp(pair<uint32_t,uint32_t>&a,pair<uint32_t,uint32_t>&b);//使得pair按照val大到小排序
class PiecewiseFunc
{
private:
	vector<double>m_piece;//分段区间
	vector<double>m_slop;//斜率
	double m_start;//初值
	vector<double>m_bias;//y=m_slop*x+m_bias
public:
	PiecewiseFunc(vector<double>&piece,vector<double>&slop,double start):
		m_piece(piece),
		m_slop(slop),
		m_start(start),
		m_bias(vector<double>())
	{
		double y=m_start;
		double bia;
		size_t i;
		for(i=0;i<m_piece.size()-1;i++)
		{
			bia=y-m_slop[i]*m_piece[i];
			m_bias.push_back(bia);
			y=m_slop[i]*m_piece[i+1]+m_bias[i];
		}
		bia=y-m_slop[i]*m_piece[i];
		m_bias.push_back(bia);



	}
	double Result(double x)
	{
		if(x<0)
			return 0;
		size_t i;
		for(i=1;i<m_piece.size();i++)
		{
			if(x<=m_piece[i])
			{
				return m_slop[i-1]*x+m_bias[i-1];
			}

		}
		return m_slop[i-1]*x+m_bias[i-1];
	}
	double GetSlop(double x)
	{
		if(x<0)
			return 0;
		size_t i;
		for(i=1;i<m_piece.size();i++)
		{
			if(x<=m_piece[i])
			{
				return m_slop[i-1];
			}

		}
		return m_slop[i-1];

	}

	vector<double> GetA()//获得系数矩阵A
	{
		return m_slop;
	}
	vector<double> GetB()//获得系数矩阵B
	{
		return m_bias;
	}

};

class MeasureAssignmentProblem;

struct thread_arg{
	MeasureAssignmentProblem* ptr;
	uint32_t n;
};

class MeasureAssignmentProblem: public ProblemBase
{
private:
	/***********final result**********/
	vector<vector<uint32_t> > m_x;
	vector<vector<double> >m_x_cplex;//cplex求解出的结果
	vector<double> m_lambda;//diff to dev,dev2
	vector<uint32_t>m_load;
	/********************************/
	vector<uint32_t>m_measureNodes;
	vector<uint32_t>m_nodeCap;//capacity of measureNodes;

	vector<double> m_lambda0;//initial lambda
	vector<double> m_objVal;//recode multi objval value on every iterate
	uint32_t m_objValNum;//the size of m_objVal
	double m_thetaLambda;//step length of lambda

	shared_ptr<PiecewiseFunc>m_costFun;//代价函数
	vector<double>m_A;//系数矩阵A
	vector<double>m_B;//系数矩阵B
	
	vector<vector<double> > m_mu0;//initial mu
	vector<double> m_objValDual;//recode multi m_objValDual value on every iterate
	uint32_t m_objValDualNum;//the number of m_objValDual
	double m_thetaMu;//step length of mu
	double m_thetaMu0;//initial step length of mu

	double m_lambda_err;//stop iterate condition
	double m_mu_err;

	//middle value
	vector<double> m_lambda_tmp;
	vector<vector<double> >m_mu_tmp;
	vector<vector<uint32_t> > *m_x_tmp;
	vector<uint32_t>m_load_tmp;
	uint32_t m_maxLoad;
	//uint32_t** m_x_tmp;
//	thread_arg *m_arg;
	pthread_mutex_t mutex;
	uint32_t thread_cnt;

	double CostFun(double lambdav);//the cost function of lambdav

	double CostFunDer(double lambdav);//代价函数的导数

public:
	MeasureAssignmentProblem();

	MeasureAssignmentProblem(shared_ptr<Network>,shared_ptr<PiecewiseFunc>costFun);

	virtual ~MeasureAssignmentProblem();

	void SetNodeCapacity();//set m_nodeCap=random,m_nodeCap<n,n为节点上的最大负载

	void SetLambda0(vector<double> lambda0);

	void SetObjNum(uint32_t n);//set m_objValNum

	void SetLambdaStepLength(double length);//set m_thetaLambda

	void SetMu0(vector<vector<double> >mu0);

	void SetObjDualNum(uint32_t n);//set m_objValDualNum

	void SetMuStepLength(double length);//set m_thetaMu

	void SetStopIterCon(double lambda_err,double mu_err);///set m_err

	virtual void SetNetwork(const shared_ptr<Network> network);

	virtual shared_ptr<Network> GetNetwork();

	void UpdateLambda(vector<double> &lambda,const vector<vector<double> >&mu_star,const vector<uint32_t>&load);

	void UpdateMu(vector<vector<double> >&Mu,const vector<double> &lambda,const vector<vector<uint32_t> > &x);

	bool IsStopLambda();

	bool IsStopMu();

	double CalObjVal(const vector<double> &lambda);

	double CalObjValDual(const vector<double> &lambda,const vector<vector<double> >&mu,const vector<vector<uint32_t> > &x);

	virtual void run();

	void SolveDualProblem(const vector<double> &lambda,const vector<vector<double> >&mu);//solve dual problem

	void SolveSubProblem(uint32_t n);//multi-thread,n th thread

	static void* Thread(void *arg);


	virtual void Print();

	bool IsFeasible();
	
	uint32_t CalMaxLoad(vector<vector<uint32_t> >&x);

	void OutPut(const string &filename);//output result

	void OutPutCplexDat(const string &fileName);//输出Cplex的dat文件
	
	void ReadCplexResult(const string &fileName);//读取Cplex的输出文件

	//以下三个接口packetFile为总的packet文件，dir为输出分配方案的文件夹
	void OutPutPacketOnMeasureNode(const string &packetFile,const string &dir);//输出分配任务后每个节点测量的数据包到文件中

	void OutPutPacketOnMeasureNode_ori(const string &packetFile,const string &dir);//输出原始分配任务后每个节点测量的数据包到文件中

	void OutPutPacketOnMeasureNode_ran(const string &packetFile,const string &dir);//输出随机分配任务后每个节点测量的数据包到文件中

	void OutPutCplexDat_rout(//输出用于路由规划的cplex数据
			const string&outdir,const string&indir,//outdir为输出的dat文件目录，indir为读取测量结果的目录
			vector<map<uint32_t,uint32_t> >&allflow_day,//每天的测量流量矩阵,累加
			vector<map<uint32_t,uint32_t> >&allflow_day_real,//每天真实流量矩阵，累加
			vector<map<uint32_t,uint32_t> >&tcam_day,//每天TCAM中记录的流
			vector<map<uint32_t,vector<vector<uint32_t> > > >&linkpaths_day,//每天每条流的可用链路路径
			vector<map<uint32_t,uint32_t> >&Pi_day,//每天每条流的备选路径
			vector<vector<vector<vector<uint32_t> > > >&delt_day//每天的delt
			);

	void Greedy_route(//贪心算法
			const string&outdir,//outdir为输出文件目录
			vector<map<uint32_t,uint32_t> >&allflow_day,//每天的测量流量矩阵,累加
			vector<map<uint32_t,uint32_t> >&allflow_day_real,//每天真实流量矩阵，累加
			vector<map<uint32_t,uint32_t> >&tcam_day,//每天TCAM中记录的流
			vector<map<uint32_t,vector<vector<uint32_t> > > >&linkpaths_day,//每天每条流的可用链路路径
			vector<map<uint32_t,uint32_t> >&Pi_day,//每天每条流的备选路径
			vector<vector<vector<vector<uint32_t> > > >&delt_day//每天的delt
			);
	void Greedy_route2(//贪心算法
			const string&outdir,//outdir为输出文件目录
			vector<map<uint32_t,uint32_t> >&allflow_day,//每天的测量流量矩阵,累加
			vector<map<uint32_t,uint32_t> >&allflow_day_real,//每天真实流量矩阵，累加
			vector<map<uint32_t,uint32_t> >&tcam_day,//每天TCAM中记录的流
			vector<map<uint32_t,vector<vector<uint32_t> > > >&linkpaths_day,//每天每条流的可用链路路径
			vector<map<uint32_t,uint32_t> >&Pi_day,//每天每条流的备选路径
			vector<vector<vector<vector<uint32_t> > > >&delt_day//每天的delt
			);
	void Random_route(//随机选路
			const string&outdir,//outdir为输出文件目录
			vector<map<uint32_t,uint32_t> >&allflow_day,//每天的测量流量矩阵,累加
			vector<map<uint32_t,uint32_t> >&allflow_day_real,//每天真实流量矩阵，累加
			vector<map<uint32_t,uint32_t> >&tcam_day,//每天TCAM中记录的流
			vector<map<uint32_t,vector<vector<uint32_t> > > >&linkpaths_day,//每天每条流的可用链路路径
			vector<map<uint32_t,uint32_t> >&Pi_day,//每天每条流的备选路径
			vector<vector<vector<vector<uint32_t> > > >&delt_day//每天的delt
			);
	void Original_route(//原始路由
			const string&outdir,//outdir为输出文件目录
			vector<map<uint32_t,uint32_t> >&allflow_day,//每天的测量流量矩阵,累加
			vector<map<uint32_t,uint32_t> >&allflow_day_real,//每天真实流量矩阵，累加
			vector<map<uint32_t,uint32_t> >&tcam_day,//每天TCAM中记录的流
			vector<map<uint32_t,vector<vector<uint32_t> > > >&linkpaths_day,//每天每条流的可用链路路径
			vector<map<uint32_t,uint32_t> >&Pi_day,//每天每条流的备选路径
			vector<vector<vector<vector<uint32_t> > > >&delt_day//每天的delt
			);

};

#endif /* MEASURE_ASSIGNMENT_PROBLEM_H */

