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

class MeasureAssignmentProblem: public ProblemBase
{
private:
	vector<vector<uint32_t> > m_x;//solve

	double m_lambda0;//initial lambda
	vector<double> m_lambdas;//recode multi lambda value on every iterate
	uint32_t m_sizeLambda;//the size of m_lambdas
	double m_thetaLambda;//step length of lambda
	
	vector<double> m_mu0;//initial mu
	vector<vector<double> > m_mus;//recode multi mu value on every iterate
	uint32_t m_sizeMu;//the size of m_mus
	double m_thetaMu;//step length of mu

	double m_err;//stop iterate condition


public:
	MeasureAssignmentProblem();

	MeasureAssignmentProblem(shared_ptr<Network>);

	virtual ~MeasureAssignmentProblem();
	
	void SetLambda0(double lambda0);

	void SetLambdaNum(uint32_t n);//set m_sizeLambda

	void SetLambdaStepLength(double length);//set m_thetaLambda

	void SetMu0(vector<double> mu0);

	void SetMuNum(uint32_t n);//set m_sizeMu

	void SetMuStepLength(double length);//set m_thetaMu

	void SetStopIterCon(double err);///set m_err

	virtual void SetNetwork(const shared_ptr<Network> network);

	virtual shared_ptr<Network> GetNetwork();

	virtual void run();

	void SolveDualProblem(const double &lambda,const vector<double> &mu);//solve dual problem

	void SolveSubProblem(const double &lambda,const vector<double>&mu);//multi-thread


	virtual void Print();

	



};

#endif /* MEASURE_ASSIGNMENT_PROBLEM_H */

