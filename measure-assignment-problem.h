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

using namespace std;

#define THREAD_LOG 0//print thread log
#define LAMBDA_LOG 1
#define MU_LOG 1

class MeasureAssignmentProblem;
struct thread_arg{
	MeasureAssignmentProblem* ptr;
	uint32_t n;
};
class MeasureAssignmentProblem: public ProblemBase
{
private:
	vector<vector<uint32_t> > m_x;//solve
	vector<uint32_t>m_measureNodes;

	double m_lambda0;//initial lambda
	vector<double> m_objVal;//recode multi objval value on every iterate
	uint32_t m_objValNum;//the size of m_objVal
	double m_thetaLambda;//step length of lambda
	
	vector<double> m_mu0;//initial mu
	vector<double> m_objValDual;//recode multi m_objValDual value on every iterate
	uint32_t m_objValDualNum;//the number of m_objValDual
	double m_thetaMu;//step length of mu

	double m_err;//stop iterate condition

	//middle value
	double m_lambda_tmp;
	vector<double> m_mu_tmp;
	vector<vector<uint32_t> > *m_x_tmp;
	vector<uint32_t>m_load_tmp;
	//uint32_t** m_x_tmp;
//	thread_arg *m_arg;
	pthread_mutex_t mutex;
	uint32_t thread_cnt;

public:
	MeasureAssignmentProblem();

	MeasureAssignmentProblem(shared_ptr<Network>);

	virtual ~MeasureAssignmentProblem();
	
	void SetLambda0(double lambda0);

	void SetObjNum(uint32_t n);//set m_objValNum

	void SetLambdaStepLength(double length);//set m_thetaLambda

	void SetMu0(vector<double> mu0);

	void SetObjDualNum(uint32_t n);//set m_objValDualNum

	void SetMuStepLength(double length);//set m_thetaMu

	void SetStopIterCon(double err);///set m_err

	virtual void SetNetwork(const shared_ptr<Network> network);

	virtual shared_ptr<Network> GetNetwork();

	void UpdateLambda(double &lambda,const vector<double>&mu_star);

	void UpdateMu(vector<double>&Mu,const double &lambda,const vector<vector<uint32_t> > &x);

	bool IsStopLambda();

	bool IsStopMu();

	double CalObjVal(const double &lambda);

	double CalObjValDual(const double &lambda,const vector<double> &mu,const vector<vector<uint32_t> > &x_star);

	virtual void run();

	void SolveDualProblem(const double &lambda,const vector<double> &mu);//solve dual problem

	void SolveSubProblem(uint32_t n);//multi-thread,n th thread

	static void* Thread(void *arg);


	virtual void Print();

	bool IsFeasible();
	
	uint32_t CalMaxLoad(vector<vector<uint32_t> >&x);


};

#endif /* MEASURE_ASSIGNMENT_PROBLEM_H */

