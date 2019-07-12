/*************************************************************************
	> File Name: measure-assignment-problem.cpp
	> Author: dengqi
	> Mail: 906262038@qq.com 
	> Created Time: 2019年07月10日 星期三 10时48分37秒
 ************************************************************************/

#include<iostream>
#include"measure-assignment-problem.h"
#include<pthread.h>
#include<algorithm>
#include<float.h>
using namespace std;
/*{{{*/
MeasureAssignmentProblem::MeasureAssignmentProblem()
{
}

MeasureAssignmentProblem::MeasureAssignmentProblem(shared_ptr<Network> network):
	ProblemBase(network)
{
	//set to vector
	for(auto it=network->m_measureNodes.begin();it!=network->m_measureNodes.end();it++)
		m_measureNodes.push_back(*it);

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
MeasureAssignmentProblem::SetObjNum(uint32_t n)
{
	m_objValNum=n;
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
MeasureAssignmentProblem::SetObjDualNum(uint32_t n)
{
	m_objValDualNum=n;
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
MeasureAssignmentProblem::UpdateLambda(double &lambda,const vector<double>&mu_star)
{
	double sum=0;
	for(auto it=mu_star.begin();it!=mu_star.end();it++)
		sum+=*it;
	lambda=lambda-m_thetaLambda+m_thetaLambda*sum;
}

void 
MeasureAssignmentProblem::UpdateMu(vector<double> &mu,const double &lambda,const vector<vector<uint32_t> >&x_star)
{
	for(size_t v=0;v<mu.size();v++)
	{
		uint32_t sum=0;
		for(size_t i=0;i<m_network->m_coarseFlowNum;i++)
		{
			sum+=m_network->m_flows[i]->m_weight*x_star[i][v];
		}
		sum-=lambda;
		mu[v]+=m_thetaMu*sum;

	}
	
}

bool 
MeasureAssignmentProblem::IsStopLambda()
{
	if(m_objVal.size()<m_objValNum)//iterate m_objValNum times at least
		return false;

	double sum=0;
	for(auto it=m_objVal.begin();it!=m_objVal.end();it++)
	{
		sum+=*it;
	}
	double ave=sum/m_objVal.size();
	double var=0;
	for(auto it=m_objVal.begin();it!=m_objVal.end();it++)
		var+=(*it-ave)*(*it-ave);
	var/=m_objVal.size();
	return var<m_err;


}

bool 
MeasureAssignmentProblem::IsStopMu()
{
	if(m_objValDual.size()<m_objValDualNum)//iterate m_objValNum times at least
		return false;

	double sum=0;
	for(auto it=m_objValDual.begin();it!=m_objValDual.end();it++)
	{
		sum+=*it;
	}
	double ave=sum/m_objValDual.size();
	double var=0;
	for(auto it=m_objValDual.begin();it!=m_objValDual.end();it++)
		var+=(*it-ave)*(*it-ave);
	var/=m_objValDual.size();
	return var<m_err;

}

double 
MeasureAssignmentProblem::CalObjVal(const double &lambda)
{
	return lambda;
}

double 
MeasureAssignmentProblem::CalObjValDual(const double &lambda,const vector<double> &mu,const vector<vector<uint32_t> > &x)
{
	double ret;
	double sum=0;
	for(size_t v=0;v<mu.size();v++)
	{
		double tmpsum=0;
		for(size_t i=0;i<m_network->m_coarseFlowNum;i++)
		{
			tmpsum+=m_network->m_flows[i]->m_weight*x[i][v];
		}
		sum+=mu[v]*(tmpsum-lambda);
	}
	ret=lambda+sum;
	return ret;
}

void 
MeasureAssignmentProblem::run()
{
	m_objVal.clear();
	m_objValDual.clear();

	m_lambda_tmp=m_lambda0;
	m_objVal.push_back(m_lambda_tmp);
	while(1)
	{
		m_mu_tmp=m_mu0;
		while(1)
		{
			SolveDualProblem(m_lambda_tmp,m_mu_tmp);
			double objDualVal=CalObjValDual(m_lambda_tmp,m_mu_tmp,m_x_tmp);
			if(m_objValDual.size()>=m_objValDualNum)
				m_objValDual.erase(m_objValDual.begin());
			m_objValDual.push_back(objDualVal);

			if(IsStopMu())
			{
				UpdateLambda(m_lambda_tmp,m_mu_tmp);
				break;
			}
			UpdateMu(m_mu_tmp,m_lambda_tmp,m_x_tmp);

		}
		double objVal=CalObjVal(m_lambda_tmp);
		if(m_objVal.size()>=m_objValNum)
			m_objVal.erase(m_objVal.begin());
		m_objVal.push_back(objVal);

		if(IsStopLambda())
			break;
	}
}

void 
MeasureAssignmentProblem::SolveDualProblem(const double &lambda,const vector<double> &mu)
{
	m_x_tmp.clear();
	m_x_tmp.resize(m_network->m_coarseFlowNum,vector<uint32_t>());

	uint32_t thread_num=m_network->m_coarseFlowNum;
	pthread_t* threads=new pthread_t[thread_num];
	int res;
	
	pthread_mutex_init(&mutex,NULL);//initialize mutex

	//create thread
	for(uint32_t n=0;n<thread_num;n++)
	{
		thread_arg* arg;
		arg->ptr=this;
		arg->n=n;
		res=pthread_create(threads+n,NULL,Thread,(void*)arg);
		if(res!=0)
		{
			cout<<"Created thread "<<n<<" failed!"<<endl;
			exit(res);
		}
	}
	//join thread
	for(int n=0;n<thread_num;n++)
	{
		res=pthread_join(threads[n],NULL); 
		if(!res)
			cout<<"Thread "<<n<<" joined"<<endl;
		else 
			cout<<"Thread "<<n<<" join failed"<<endl;
	}

	pthread_mutex_destroy(&mutex);
	delete[] threads;

}

void*
MeasureAssignmentProblem::Thread(void *arg)
{
	thread_arg* arg_ptr=(thread_arg*)arg;
	arg_ptr->ptr->SolveSubProblem(arg_ptr->n);
}

void
MeasureAssignmentProblem::SolveSubProblem(uint32_t n)
{
	uint32_t i=n;//flow id
	vector<uint32_t>xi(vector<uint32_t>(m_measureNodes.size(),0));
	shared_ptr<Flow>flow=m_network->m_flows[i];
	vector<uint32_t>nodePath=flow->m_nodePaths[0];

	double obj=DBL_MAX;

	for(uint32_t node:nodePath)
	{
		for(uint32_t v=0;v<m_measureNodes.size();v++)
		{
			if(node==m_measureNodes[v])
			{
				if(obj>m_mu_tmp[v]*flow->m_weight)
				{
					xi=vector<uint32_t>(m_measureNodes.size(),0);
					xi[v]=1;
					
				}
				
			}
		}

	}

	pthread_mutex_lock(&mutex);//lock

	m_x_tmp[i]=xi;

	pthread_mutex_unlock(&mutex);//unlock
	
	pthread_exit(NULL);

}

void 
MeasureAssignmentProblem::Print()
{

}
