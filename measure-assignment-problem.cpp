/*************************************************************************
	> File Name: measure-assignment-problem.cpp
	> Author: dengqi
	> Mail: 906262038@qq.com 
	> Created Time: 2019年07月10日 星期三 10时48分37秒
 ************************************************************************/

#include<iostream>
#include"measure-assignment-problem.h"
using namespace std;
/*{{{*/
MeasureAssignmentProblem::MeasureAssignmentProblem()
{
}

MeasureAssignmentProblem::MeasureAssignmentProblem(shared_ptr<Network> network):
	ProblemBase(network)
{

}

MeasureAssignmentProblem::~MeasureAssignmentProblem()
{

}

void
MeasureAssignmentProblem::SetLambda0(double lambda0)
{
	m_lambda0=lambda0;
}

void 
MeasureAssignmentProblem::SetLambdaNum(uint32_t n)
{
	m_sizeLambda=n;
}

void 
MeasureAssignmentProblem::SetLambdaStepLength(double length)
{
	m_thetaLambda=length;
}

void
MeasureAssignmentProblem::SetMu0(vector<double> mu0)
{
	m_mu0=mu0;
}

void
MeasureAssignmentProblem::SetMuNum(uint32_t n)
{
	m_sizeMu=n;
}

void
MeasureAssignmentProblem::SetMuStepLength(double length)
{
	m_thetaMu=length;
}

void
MeasureAssignmentProblem::SetStopIterCon(double err)
{
	m_err=err;
}

void
MeasureAssignmentProblem::SetNetwork(shared_ptr<Network> network)
{
	m_network=network;
}

shared_ptr<Network> 
MeasureAssignmentProblem::GetNetwork()
{
	return m_network;
}
/*}}}*/

void 
MeasureAssignmentProblem::run()
{

}

void 
MeasureAssignmentProblem::SolveDualProblem(const double &lambda,const vector<double> &mu)
{

}

void
MeasureAssignmentProblem::SolveSubProblem(cinst double &lambda,const vector<double> &mu)
{

}

void 
MeasureAssignmentProblem::Print()
{

}
