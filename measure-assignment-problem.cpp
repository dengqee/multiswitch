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
#include<cstdlib>
#include<unistd.h>
#include<fstream>
using namespace std;
/*{{{*/

double 
MeasureAssignmentProblem::CostFun(double lambdav)
{
	return m_costFun->Result(lambdav);
}

double 
MeasureAssignmentProblem::CostFunDer(double lambdav)
{
	return m_costFun->GetSlop(lambdav);
}


MeasureAssignmentProblem::MeasureAssignmentProblem()
{
}

MeasureAssignmentProblem::MeasureAssignmentProblem(shared_ptr<Network> network,shared_ptr<PiecewiseFunc>costFun):
	ProblemBase(network),
//	m_arg(new thread_arg),
	m_costFun(costFun),
	m_x_tmp(new vector<vector<uint32_t> > ())

{
	//set to vector
	for(auto it=network->m_measureNodes.begin();it!=network->m_measureNodes.end();it++)
		m_measureNodes.push_back(*it);
	m_load_tmp=vector<uint32_t>(m_measureNodes.size(),0);
	SetNodeCapacity(network->m_fineFlowNum);

}

MeasureAssignmentProblem::~MeasureAssignmentProblem()
{
//	delete m_arg;
	delete m_x_tmp;

}

void 
MeasureAssignmentProblem::SetNodeCapacity(uint32_t n)
{
	srand(RAND_SEED);
	for(int i=0;i<m_measureNodes.size();i++)
	{
		m_nodeCap.push_back(rand()%n);
	}
}

void
MeasureAssignmentProblem::SetLambda0(vector<double> lambda0)
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
MeasureAssignmentProblem::SetStopIterCon(double lambda_err,double mu_err)
{
	m_lambda_err=lambda_err;
	m_mu_err=mu_err;
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
MeasureAssignmentProblem::UpdateLambda(vector<double> &lambda,const vector<double>&mu_star,const vector<uint32_t>&load)
{
#if LAMBDA_LOG
	double sum=0;
	cout<<"mu=";
	for(auto it=mu_star.begin();it!=mu_star.end();it++)
	{
		sum+=*it;
		cout<<" "<<*it;
	}
	cout<<endl;
#endif
	for(size_t v=0;v<lambda.size();v++)
	{
		lambda[v]=lambda[v]-m_thetaLambda*CostFunDer(lambda[v])+m_thetaLambda*mu_star[v]*m_nodeCap[v];
		lambda[v]=lambda[v]>0?lambda[v]:0;
	}
}

void 
MeasureAssignmentProblem::UpdateMu(vector<double> &mu,const vector<double> &lambda,const vector<vector<uint32_t> >&x_star)
{
	for(size_t v=0;v<mu.size();v++)
	{
		double sum=0;
		for(size_t i=0;i<m_network->m_coarseFlowNum;i++)
		{
			sum+=m_network->m_flows[i]->m_weight*x_star[i][v];
		}
		sum-=m_nodeCap[v]*lambda[v];
		//sum-=*max_element(m_load_tmp.begin(),m_load_tmp.end());
		mu[v]+=m_thetaMu*sum;
		if(mu[v]<0)
			mu[v]=0;

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
	return var<m_lambda_err;


}

bool 
MeasureAssignmentProblem::IsStopMu()
{
	if(m_objValDual.size()<m_objValDualNum)//iterate m_objValNum times at least
		return false;
	
//	if(m_maxLoad<m_lambda_tmp)
//		return true;

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
	return var<m_mu_err;

}

double 
MeasureAssignmentProblem::CalObjVal(const vector<double> &lambda)
{
	double ret=0;
	for(size_t i=0;i<lambda.size();i++)
	{
		ret+=CostFun(lambda[i]);
	}
	return ret;
}

double 
MeasureAssignmentProblem::CalObjValDual(const vector<double> &lambda,const vector<double> &mu,const vector<vector<uint32_t> > &x)
{
	double ret;
	double sum=0;
	for(size_t v=0;v<mu.size();v++)
	{
		uint32_t tmpsum=0;
		for(size_t i=0;i<m_network->m_coarseFlowNum;i++)
		{
			tmpsum+=m_network->m_flows[i]->m_weight*x[i][v];
		}
		m_load_tmp[v]=tmpsum;
		
		sum+=mu[v]*(tmpsum-m_nodeCap[v]*lambda[v]);
	}
	m_maxLoad=*max_element(m_load_tmp.begin(),m_load_tmp.end());
	ret=CalObjVal(lambda)+sum;
	return ret;
}

void 
MeasureAssignmentProblem::run()
{
	m_objVal.clear();
	m_objValDual.clear();

	m_lambda_tmp=m_lambda0;

	uint32_t k=0,l=0;//k is the number of lambda iterate time, l is mu
	while(1)
	{
		double objVal=CalObjVal(m_lambda_tmp);
#if LAMBDA_LOG 
		cout<<endl<<"lambda iterate time: "<<k<<endl;
		if(k>1)
		{
			IsFeasible();
			cout<<"max load="<<CalMaxLoad(*m_x_tmp)<<endl;
			cout<<"objDual="<<*(m_objValDual.rbegin())<<endl;
		}
		cout<<"Lambda: ";
		for(auto i:m_lambda_tmp)
		cout<<i<<" ";
		cout<<endl;
		cout<<"objVal= "<<objVal<<endl;

		cout<<"load:";
		for(auto it=m_load_tmp.begin();it!=m_load_tmp.end();it++)
			cout<<*it<<" ";
		cout<<endl;
		
#endif
		m_objValDual.clear();
		m_mu_tmp=m_mu0;
		l=0;
		while(1)
		{

			SolveDualProblem(m_lambda_tmp,m_mu_tmp);

			double objDualVal=CalObjValDual(m_lambda_tmp,m_mu_tmp,*m_x_tmp);
#if MU_LOG
			cout<<"    mu iterate time: "<<l<<endl;

			cout<<"    objDual="<<objDualVal<<endl;
#endif

			if(m_objValDual.size()>=m_objValDualNum)
				m_objValDual.erase(m_objValDual.begin());
			m_objValDual.push_back(objDualVal);

			if(IsStopMu())
			{
				/*if(k==0&&m_lambda_tmp<m_maxLoad)
				{
					m_lambda_tmp=m_maxLoad;
					
				}
				*/
				UpdateLambda(m_lambda_tmp,m_mu_tmp,m_load_tmp);
				break;
			}
			UpdateMu(m_mu_tmp,m_lambda_tmp,*m_x_tmp);
			l++;
			m_thetaMu=0.0001/sqrt(l);
		}


		if(m_objVal.size()>=m_objValNum)
			m_objVal.erase(m_objVal.begin());
		m_objVal.push_back(objVal);

		if(IsStopLambda())
			break;
		k++;
		m_thetaLambda=0.1/sqrt(k);
	}
}


void 
MeasureAssignmentProblem::SolveDualProblem(const vector<double> &lambda,const vector<double> &mu)
{
	m_x_tmp->clear();
	m_x_tmp->resize(m_network->m_coarseFlowNum,vector<uint32_t>());

	uint32_t thread_num=m_network->m_coarseFlowNum;
	pthread_t* threads=new pthread_t[thread_num];
	int res;
	thread_cnt=0;	
	pthread_mutex_init(&mutex,NULL);//initialize mutex
	//create thread
	thread_arg* arg=new thread_arg[thread_num];
	for(uint32_t n=0;n<thread_num;n++)
	{
		arg[n].n=n;
		arg[n].ptr=this;
		res=pthread_create(&(threads[n]),NULL,Thread,(void*)&(arg[n]));
		if(res!=0)
		{
			cout<<"\tCreated thread "<<n<<" failed!"<<endl;
			exit(res);
		}
#if THREAD_LOG
		cout<<"\tCreated thread "<<n<<"!"<<endl;
#endif
	}
	//join thread
	for(int n=0;n<thread_num;n++)
	//for(int n=0;n<1;n++)
	{
		res=pthread_join(threads[n],NULL);
#if THREAD_LOG
		if(!res){
			cout<<"\tThread "<<n<<" joined"<<endl;

		}
		else
			cout<<"\tThread "<<n<<" join failed"<<endl;
#endif
	}

	pthread_mutex_destroy(&mutex);
	delete[] arg;
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
#if THREAD_LOG
	cout<<"\tthread "<<n<<" start"<<endl;
#endif
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
				if(obj>=m_mu_tmp[v])
				{
					obj=m_mu_tmp[v];	
					xi=vector<uint32_t>(m_measureNodes.size(),0);
					xi[v]=1;
					
				}
				
			}
		}

	}


	pthread_mutex_lock(&mutex);//lock
	(*m_x_tmp)[i]=xi;
#if THREAD_LOG
	cout<<"\tthread "<<n<<" end"<<endl;
#endif
	pthread_mutex_unlock(&mutex);//unlock
	pthread_exit(NULL);

}

void 
MeasureAssignmentProblem::Print()
{
	cout<<"lambda: "<<endl;
	for(auto i:m_lambda)
		cout<<i<<" ";
	cout<<endl;
	cout<<"load: ";
	for(auto it=m_load.begin();it!=m_load.end();it++)
		cout<<*it<<" ";
	cout<<endl;

}

bool 
MeasureAssignmentProblem::IsFeasible()
{
	for(uint32_t v=0;v<m_measureNodes.size();v++)
	{
		uint32_t sum=0;
		for(uint32_t i=0;i<m_network->m_coarseFlowNum;i++)
		{
			sum+=m_network->m_flows[i]->m_weight*(*m_x_tmp)[i][v];
		}
		if(sum>m_nodeCap[v])
		{
			cout<<"unfeasible!"<<endl;
			return false;
		}
	}
	cout<<"feasible!"<<endl;
	return true;

}

uint32_t 
MeasureAssignmentProblem::CalMaxLoad(vector<vector<uint32_t> >&x)
{
	uint32_t lambda=0;
	for(uint32_t v=0;v<m_measureNodes.size();v++)
	{
		uint32_t sum=0;
		for(uint32_t i=0;i<m_network->m_coarseFlowNum;i++)
		{
			sum+=m_network->m_flows[i]->m_weight*x[i][v];
		}
		if(sum>lambda)
		{
			lambda=sum;
		}
	}
	return lambda;

}

void 
MeasureAssignmentProblem::OutPut(const string&filename)
{
	ofstream ofs(filename.c_str());
	for(size_t i=0;i<m_x.size();i++)
	{

		for(size_t j=0;j<m_x[i].size();j++)
			ofs<<m_x[i][j]<<" ";
		ofs<<endl;
	}
	ofs.close();
}
