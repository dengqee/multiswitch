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
#include<sstream>
#include<cmath>
#include<functional>
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
	m_A(costFun->GetA()),
	m_B(costFun->GetB()),
	m_x_tmp(new vector<vector<uint32_t> > ())

{
	//set to vector
	for(auto it=network->m_measureNodes.begin();it!=network->m_measureNodes.end();it++)
		m_measureNodes.push_back(*it);
	m_load_tmp=vector<uint32_t>(m_measureNodes.size(),0);
	SetNodeCapacity();//设置容量

}

MeasureAssignmentProblem::~MeasureAssignmentProblem()
{
//	delete m_arg;
	delete m_x_tmp;

}

void 
MeasureAssignmentProblem::SetNodeCapacity()
{
	m_nodeCap.clear();
	srand(RAND_SEED);
	int mmax=0,mmin=120000;
	vector<uint32_t>all_cap;//为23个节点都产生一个容量
	int nodetotal=m_network->m_topo->GetNodeNum();//拓扑的总节点数
	for(int i=0;i<nodetotal;i++)
	{
		//int w_max=4000;//cmsketch的w
		//int w=rand()%w_max;
		set<uint32_t>flowIDOnNode=m_network->GetFlowOnNode(i);
		int maxLoad=0;
		for(auto it=flowIDOnNode.begin();it!=flowIDOnNode.end();it++)
		{
			maxLoad+=m_network->m_flows[*it]->m_weight;
		}
		all_cap.push_back(rand()%maxLoad);
		if(mmax<maxLoad)
			mmax=maxLoad;
		if(mmin>maxLoad)
			mmin=maxLoad;
	}
	for(int node:m_measureNodes)
	{
		m_nodeCap.push_back(all_cap[node]);
		//m_nodeCap.push_back(3300);//所有节点设置为相同的容量
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
MeasureAssignmentProblem::SetMu0(vector<vector<double> >mu0)
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
	m_thetaMu0=length;
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
MeasureAssignmentProblem::UpdateLambda(vector<double> &lambda,const vector<vector<double> >&mu_star,const vector<uint32_t>&load)
{
#if LAMBDA_LOG
	double sum=0;
	cout<<"mu="<<endl;
	for(size_t i=0;i<mu_star.size();i++ )
	{
		for(auto it=mu_star[i].begin();it!=mu_star[i].end();it++)
		{
			sum+=*it;
			cout<<" "<<*it;
		}
		cout<<endl;

	}
#endif
	for(size_t v=0;v<lambda.size();v++)
	{
		double sum=0;
		for(size_t m=0;m<mu_star.size();m++)
		{
			sum+=mu_star[m][v];

		}
		lambda[v]=lambda[v]-m_thetaLambda*(1-sum);
		lambda[v]=lambda[v]>0?lambda[v]:0;
	}
}

void 
MeasureAssignmentProblem::UpdateMu(vector<vector<double> > &mu,const vector<double> &lambda,const vector<vector<uint32_t> >&x_star)
{
	for(size_t m=0;m<mu.size();m++)
		for(size_t v=0;v<mu[m].size();v++)
		{
			double sum=0;
			for(size_t i=0;i<m_network->m_coarseFlowNum;i++)
			{
				sum+=m_network->m_flows[i]->m_weight*x_star[i][v]*m_A[m];
			}
			//sum-=*max_element(m_load_tmp.begin(),m_load_tmp.end());
			mu[m][v]+=m_thetaMu*(sum+m_B[m]*m_nodeCap[v]-lambda[v]);//这里是+B[m]
			if(mu[m][v]<0)
				mu[m][v]=0;

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
		ret+=lambda[i];
	}
	return ret;
}

double 
MeasureAssignmentProblem::CalObjValDual(const vector<double> &lambda,const vector<vector<double> > &mu,const vector<vector<uint32_t> > &x)
{
	double ret;
	double sum=0;
	for(size_t m=0;m<mu.size();m++)
	{
		for(size_t v=0;v<mu[m].size();v++)
		{
			uint32_t tmpsum=0;
			for(size_t i=0;i<m_network->m_coarseFlowNum;i++)
			{
				tmpsum+=m_network->m_flows[i]->m_weight*x[i][v];
			}
			m_load_tmp[v]=tmpsum;
			
			sum+=mu[m][v]*(m_A[m]*tmpsum+m_nodeCap[v]*m_B[m]-lambda[v]);
		}

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
			//if(l>2000)
				m_thetaMu=m_thetaMu0/sqrt(l);
		}


		if(m_objVal.size()>=m_objValNum)
			m_objVal.erase(m_objVal.begin());
		m_objVal.push_back(objVal);

		if(IsStopLambda())
			break;
		k++;
		//m_thetaLambda=0.1/sqrt(k);
	}
}


void 
MeasureAssignmentProblem::SolveDualProblem(const vector<double> &lambda,const vector<vector<double> > &mu)
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
				double objtmp=0;
				for(size_t m=0;m<m_mu_tmp.size();m++)
					objtmp+=m_mu_tmp[m][v]*m_A[m];
				if(obj>=objtmp)
				{
					obj=objtmp;	
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

void 
MeasureAssignmentProblem::OutPutCplexDat(const string &fileName)
{
	ofstream ofs(fileName.c_str());
	ofs<<"F="<<m_network->m_coarseFlowNum<<';'<<endl;//粗流总数
	ofs<<"S="<<m_measureNodes.size()<<';'<<endl;//测量节点数
	ofs<<"M="<<m_A.size()<<';'<<endl;//分段函数段数
/**************************************/
	ofs<<"A=[";
	for(size_t i=0;i<m_A.size();i++)
	{
		ofs<<m_A[i];
		if(i<m_A.size()-1)
			ofs<<',';
	}
	ofs<<"];"<<endl;

	ofs<<"B=[";
	for(size_t i=0;i<m_B.size();i++)
	{
		ofs<<-m_B[i];
		if(i<m_B.size()-1)
			ofs<<',';
	}
	ofs<<"];"<<endl;
/**************************************/

	ofs<<"Si=[";
	for(size_t i=0;i<m_network->m_coarseFlowNum;i++)
	{
		if(i!=0)
			ofs<<"    ";
		ofs<<'{';
		shared_ptr<Flow>flow=m_network->m_flows[i];
		vector<uint32_t>nodePath=flow->m_nodePaths[0];
		vector<uint32_t>Si;
		for(uint32_t node:nodePath)
		{
			for(uint32_t v=0;v<m_measureNodes.size();v++)
			{
				if(node==m_measureNodes[v])
				{
					Si.push_back(v);
				}
			}

		}
		//sort(Si.begin(),Si.end());
		for(size_t v=0;v<Si.size();v++)
		{
			ofs<<Si[v];
			if(v<Si.size()-1)
				ofs<<',';
		}
		ofs<<'}';
		if(i<m_network->m_coarseFlowNum-1)
			ofs<<','<<endl;
	}
	ofs<<"];"<<endl;
/**************************************/
	ofs<<"weight=[";
	for(size_t i=0;i<m_network->m_coarseFlowNum;i++)
	{
		ofs<<m_network->m_flows[i]->m_weight;
		if(i<m_network->m_coarseFlowNum-1)
			ofs<<',';
	}
	ofs<<"];"<<endl;
/**************************************/
	ofs<<"N=[";
	for(size_t v=0;v<m_nodeCap.size();v++)
	{
		ofs<<m_nodeCap[v];
		if(v<m_nodeCap.size()-1)
			ofs<<',';
	}
	ofs<<"];"<<endl;

	ofs.close();


}

void 
MeasureAssignmentProblem::ReadCplexResult(const string &fileName)
{
	ifstream ifs(fileName.c_str());
	istringstream lineBuffer;
	string line;
	getline(ifs,line);//Nmax,测量节点容量
	getline(ifs,line);//phy，测量节点代价
	getline(ifs,line);//load，测量节点负载
	lineBuffer.str(line);
	uint32_t load;
	m_load.clear();
	while(lineBuffer>>load)
	{
		m_load.push_back(load);
	}
	vector<vector<double> >x;
	for(int i=0;i<m_network->m_coarseFlowNum;i++)
	{
		vector<double>xi;
		for(int v=0;v<m_network->m_measureNodeNum;v++)
		{
			double tmp;
			ifs>>tmp;
			xi.push_back(tmp);

		}
		x.push_back(xi);
	}
	m_x_cplex=x;
	ifs.close();
}

void 
MeasureAssignmentProblem::OutPutPacketOnMeasureNode(const string &packetFile,const string &dir)
{
	string outcap=dir+"capacity.txt";
	ofstream ofs(outcap.c_str());
	for(auto i:m_nodeCap)
	{
		ofs<<i<<" ";
	}
	ofs.close();
	ifstream ifs(packetFile.c_str());
	vector<ofstream*>ofss;
	for(int v=0;v<m_network->m_measureNodeNum;v++)
	{
		string outfile=dir+to_string(m_measureNodes[v])+".txt";
		ofstream *ofs=new ofstream(outfile.c_str());
		if(!ofs->good())
		{
			cerr<<"outfile err"<<endl;
			exit(1);
		}
		ofss.push_back(ofs);
	}
	vector<vector<pair<int,int>>>numarea(m_network->m_coarseFlowNum,
			vector<pair<int,int>>(m_network->m_measureNodeNum,make_pair(-1,-1)));
												//记录每条粗流在各个节点上测量的num区间,
											    //(-1,-1)表示不测量
	for(int i=0;i<m_network->m_coarseFlowNum;i++)
	{
		int weight=m_network->m_flows[i]->m_weight;
		int start=0;
		for(int v=0;v<m_network->m_measureNodeNum;v++)
		{
			if(m_x_cplex[i][v]<1e-8)
				continue;
			int first=start;
			int second=first+round(weight*m_x_cplex[i][v])-1;
			start=second+1;
			numarea[i][v]=make_pair(first,second);

		}
	}
	istringstream lineBuffer;
	string line;
	set<uint32_t>fs1,fs2;
	int sum1=0,sum2=0;
	while(getline(ifs,line))
	{
		lineBuffer.clear();
		lineBuffer.str(line);
		uint32_t s,t,num,flowID;
		lineBuffer>>s>>t>>num;
//		fs1.insert(s*100000+t*1000+num);
		if(t<s)
			flowID=s*(m_network->m_topo->GetNodeNum()-1)+t;
		else
			flowID=s*(m_network->m_topo->GetNodeNum()-1)+t-1;
		shared_ptr<Flow>flow=m_network->m_flows[flowID];
		bool flag=false;
		for(int v=0;v<m_network->m_measureNodeNum;v++)
		{
			if(numarea[flowID][v].first==-1&&numarea[flowID][v].second==-1)
				continue;
			if(num>=numarea[flowID][v].first&&num<=numarea[flowID][v].second)
			{
				*ofss[v]<<line<<endl;
//				fs2.insert(s*100000+t*1000+num);
				flag=true;
				sum1++;
				break;
			}
		}
		if(!flag)
		{
		//	fs2.insert(s*100000+t*1000+num);
			sum2++;

		}

	}
	cout<<sum1<<" "<<sum2<<endl;//输出的是未写入的流
	for(int v=0;v<m_network->m_measureNodeNum;v++)
	{
		ofss[v]->close();
		delete ofss[v];
	}


}
void 
MeasureAssignmentProblem::OutPutPacketOnMeasureNode_ori(const string &packetFile,const string &dir)
{
	ifstream ifs(packetFile.c_str());
	vector<ofstream*>ofss;
	for(int v=0;v<m_network->m_measureNodeNum;v++)
	{
		string outfile=dir+to_string(m_measureNodes[v])+".txt";
		ofstream *ofs=new ofstream(outfile.c_str());
		if(!ofs->good())
		{
			cerr<<"outfile err"<<endl;
			exit(1);
		}
		ofss.push_back(ofs);
	}
	istringstream lineBuffer;
	string line;
	set<uint32_t>fs1,fs2;
	int sum=0;
	while(getline(ifs,line))
	{
		lineBuffer.str(line);
		uint32_t s,t,num,flowID;
		lineBuffer>>s>>t>>num;
		//fs1.insert(s*100000+t*1000+num);
		if(t<s)
			flowID=s*(m_network->m_topo->GetNodeNum()-1)+t;
		else
			flowID=s*(m_network->m_topo->GetNodeNum()-1)+t-1;
		shared_ptr<Flow>flow=m_network->m_flows[flowID];
		bool flag=false;
		for(int v=0;v<m_network->m_measureNodeNum;v++)
		{
			set<uint32_t>pathnode(flow->m_nodePaths[0].begin(),flow->m_nodePaths[0].end());//第0条路径是最短路
			if(find(pathnode.begin(),pathnode.end(),m_measureNodes[v])!=pathnode.end())//if在pathnode中找到了第v个节点
			{
				*ofss[v]<<line<<endl;
			//	fs2.insert(s*100000+t*1000+num);
				flag=true;
			}
		}
		if(!flag)
		{
			fs2.insert(s*100000+t*1000+num);
			sum++;

		}

	}
	cout<<fs2.size()<<" "<<sum<<endl;
	for(int v=0;v<m_network->m_measureNodeNum;v++)
	{
		ofss[v]->close();
		delete ofss[v];
	}


}
void 
MeasureAssignmentProblem::OutPutPacketOnMeasureNode_ran(const string &packetFile,const string &dir)
{
	ifstream ifs(packetFile.c_str());
	vector<ofstream*>ofss;
	for(int v=0;v<m_network->m_measureNodeNum;v++)
	{
		string outfile=dir+to_string(m_measureNodes[v])+".txt";
		ofstream *ofs=new ofstream(outfile.c_str());
		if(!ofs->good())
		{
			cerr<<"outfile err"<<endl;
			exit(1);
		}
		ofss.push_back(ofs);
	}
	istringstream lineBuffer;
	string line;
	set<uint32_t>fs1,fs2;
	int sum1=0,sum2=0;
	while(getline(ifs,line))
	{
		lineBuffer.str(line);
		uint32_t s,t,num,flowID;
		lineBuffer>>s>>t>>num;
		uint32_t flowkey=s*100000+t*1000+num;
		hash<uint32_t>hash_uint32_t;
		

		//fs1.insert(s*100000+t*1000+num);
		if(t<s)
			flowID=s*(m_network->m_topo->GetNodeNum()-1)+t;
		else
			flowID=s*(m_network->m_topo->GetNodeNum()-1)+t-1;
		shared_ptr<Flow>flow=m_network->m_flows[flowID];
		set<uint32_t>pathnode(flow->m_nodePaths[0].begin(),flow->m_nodePaths[0].end());//第0条路径是最短路
		set<uint32_t>measureNodesSet=m_network->m_measureNodes;
		vector<uint32_t>ins;
		set_intersection(pathnode.begin(),pathnode.end(),
				measureNodesSet.begin(),measureNodesSet.end(),
				insert_iterator<vector<uint32_t>>(ins,ins.begin()));//求测量节点和路径节点之间的交集，保存在ins
		
		uint32_t hash_val=hash_uint32_t(flowkey)%ins.size();//计算哈希值
		uint32_t measureNode=ins[hash_val];//测量节点号


		bool flag=false;
		for(int v=0;v<m_network->m_measureNodeNum;v++)
		{
			if(m_measureNodes[v]==measureNode)//if在pathnode中找到了第v个节点
			{
				*ofss[v]<<line<<endl;
			//	fs2.insert(s*100000+t*1000+num);
				flag=true;
				sum1++;
				break;
			}
		}
		if(!flag)
		{
			fs2.insert(s*100000+t*1000+num);
			sum2++;

		}

	}
	cout<<sum1<<" "<<sum2<<endl;
	for(int v=0;v<m_network->m_measureNodeNum;v++)
	{
		ofss[v]->close();
		delete ofss[v];
	}


}
void
MeasureAssignmentProblem::OutPutCplexDat_rout(const string&outdir,const string&indir,
		vector<map<uint32_t,uint32_t> >&allflow_day,//每天的测量流量矩阵,累加
		vector<map<uint32_t,uint32_t> >&allflow_day_real,//每天真实流量矩阵，累加
		vector<map<uint32_t,uint32_t> >&tcam_day,//每天TCAM中记录的流
		vector<map<uint32_t,vector<vector<uint32_t> > > >&linkpaths_day,//每天每条流的可用链路路径
		vector<map<uint32_t,uint32_t> >&Pi_day,//每天每条流的备选路径
		vector<vector<vector<vector<uint32_t> > > >&delt_day//每天的delt
		)
{

	vector<map<uint32_t,uint32_t> >flowsval_day,flowsval_day_real;//保存每天测量值
	for(int day=1;day<=10;day++)
	{
		//统计测量值流量矩阵
		string filename=indir+"day"+to_string(day)+"/"+to_string(TCAM)+"_1_est.txt";
		ifstream ifs(filename.c_str());
		map<uint32_t,uint32_t>flowsval;
		string line;
		stringstream lineBuffer;
		while(getline(ifs,line))
		{
			lineBuffer.clear();
			lineBuffer.str(line);
			uint32_t key,val;
			lineBuffer>>key>>val;
			flowsval[key]=val;
		}
		ifs.close();
		flowsval_day.push_back(flowsval);
		//统计真实流量矩阵
		filename=indir+"day"+to_string(day)+"/real.txt";
		ifs.open(filename.c_str());
		map<uint32_t,uint32_t>flowsval_real;
		while(getline(ifs,line))
		{
			lineBuffer.clear();
			lineBuffer.str(line);
			uint32_t key,val;
			lineBuffer>>key>>val;
			flowsval_real[key]=val;
		}
		ifs.close();
		flowsval_day_real.push_back(flowsval_real);

		filename=indir+"day"+to_string(day)+"/"+to_string(TCAM)+"_TCAM.txt";
		ifs.open(filename.c_str());
		map<uint32_t,uint32_t>tcam;
		while(getline(ifs,line))
		{
			lineBuffer.clear();
			lineBuffer.str(line);
			uint32_t key,node;
			lineBuffer>>key>>node;
			tcam[key]=node;
		}
		tcam_day.push_back(tcam);
		ifs.close();

	}
	//vector<map<uint32_t,uint32_t> >allflow_day,allflow_day_real;//每天的流量矩阵,累加
	map<uint32_t,uint32_t>tmp;//记录所有流的key
	for(auto it=flowsval_day[9].begin();it!=flowsval_day[9].end();it++)
	{
		tmp[it->first]=0;
	}
	for(uint32_t day=1;day<=10;day++)
	{
		allflow_day.push_back(tmp);
		for(auto it=flowsval_day[day-1].begin();it!=flowsval_day[day-1].end();it++)
		{
			allflow_day[day-1][it->first]=it->second;

		}
		//真实流量
		allflow_day_real.push_back(tmp);
		for(auto it=flowsval_day_real[day-1].begin();it!=flowsval_day_real[day-1].end();it++)
		{
			allflow_day_real[day-1][it->first]=it->second;

		}
	}
	for(int day=1;day<10;day++)
	{
		string fileName=outdir+"problem4_day"+to_string(day)+".dat";
		ofstream ofs(fileName.c_str());
		ofs<<"E="<<m_network->m_topo->m_linkNum<<";"<<endl;//链路数量
		ofs<<"F="<<m_network->m_fineFlowNum<<';'<<endl;//细流总数
	/*****************************************************************************************/	
		map<uint32_t,uint32_t>tcam=tcam_day[day-1];//tcam中的流
		uint32_t P=0;//最大可用路径数
		map<uint32_t,uint32_t>Pi;//流i有几条可用路径
		map<uint32_t,vector<vector<uint32_t> > >paths;//每条流的可用路径
		for(auto it=tmp.begin();it!=tmp.end();it++)
		{
			uint32_t key=it->first;
			uint32_t s=key/100000;
			uint32_t d=key%100000/1000;
			if(tcam.find(key)!=tcam.end())//如果是大流
			{
				if(tcam[key]==s)//如果测量节点就是源节点
				{
					vector<uint32_t>path,neighbor;
					neighbor=m_network->m_topo->m_nodes[s];//邻居
					paths[key].push_back(m_network->m_topo->m_paths[pair<uint32_t,uint32_t>(s,d)]);//默认路由放第一个
					for(uint32_t node:neighbor)
					{
						if(node==paths[key][0][1])
							continue;
						path.clear();
						if(node==d)
						{
							path.push_back(d);
						}
						else
						{
							pair<uint32_t,uint32_t>nodepair(node,d);
							path=m_network->m_topo->m_paths[nodepair];
						}
						if(path.size()>1&&path[1]==s)
							continue;
						path.insert(path.begin(),s);
						paths[key].push_back(path);
					}
				}
				else if(tcam[key]==d)//测量节点是目的节点
				{
					paths[key].push_back(m_network->m_topo->m_paths[pair<uint32_t,uint32_t>(s,d)]);
				}
				else  //测量节点不是源节点,且不是目的节点
				{
					uint32_t node=tcam[key];//测量节点
					vector<uint32_t>path,neighbor;
					paths[key].push_back(m_network->m_topo->m_paths[pair<uint32_t,uint32_t>(s,d)]);//默认路由放第一个
					uint32_t nextnode;//默认路由中下一个节点
					for(int i=0;i<paths[key][0].size();i++)
					{
						if(paths[key][0][i]==node)
						{
							nextnode=paths[key][0][i+1];
							break;
						}
					}
					neighbor=m_network->m_topo->m_nodes[node];//邻居
					path=m_network->m_topo->m_paths[pair<uint32_t,uint32_t>(s,node)];
					for(uint32_t n:neighbor)
					{
						vector<uint32_t>respath;
						if(n==nextnode||n==path[path.size()-2])
							continue;
						if(n==d)
						{
							respath.push_back(d);
						}
						else
						{
							respath=m_network->m_topo->m_paths[pair<uint32_t,uint32_t>(n,d)];
							if(respath[1]==node)//loop path
							{
								continue;
							}
						}
						path.insert(path.end(),respath.begin(),respath.end());
						paths[key].push_back(path);
					}
				}
			}
			else//如果是小流，只有一条备选路径
			{
				paths[key].push_back(m_network->m_topo->m_paths[pair<uint32_t,uint32_t>(s,d)]);
			}
			Pi[key]=paths[key].size();
			if(P<Pi[key])
				P=Pi[key];
		
		}
		//linkpaths_day.push_back(paths);
		Pi_day.push_back(Pi);//输出
		/****************************************************************************/
		ofs<<"P="<<P<<';'<<endl;//测量节点数
		ofs<<"M="<<m_A.size()<<';'<<endl;//分段函数段数
		/**************************************/
		ofs<<"A=[";
		for(size_t i=0;i<m_A.size();i++)
		{
			ofs<<m_A[i];
			if(i<m_A.size()-1)
				ofs<<',';
		}
		ofs<<"];"<<endl;

		ofs<<"B=[";
		for(size_t i=0;i<m_B.size();i++)
		{
			ofs<<-m_B[i];
			if(i<m_B.size()-1)
				ofs<<',';
		}
		ofs<<"];"<<endl;
		/**************************************/
/*
		ofs<<"Pi=[";
		uint32_t i=0;
		
		for(auto it=Pi.begin();it!=Pi.end();i++,it++)
		{
			ofs<<'{';
			for(int p=0;p<it->second;p++)
			{
				ofs<<p;
				if(p<it->second - 1)
					ofs<<',';
			}
			ofs<<'}';
			if(i<Pi.size()-2)
				ofs<<',';
		}
		
		ofs<<"];"<<endl;
*/		
		/**************************************/
		uint64_t sum=0;
		ofs<<"weight=[";
		if(day==1)
		{
			int i=0;
			for(auto it=allflow_day[day-1].begin();it!=allflow_day[day-1].end();i++,it++)
			{
				ofs<<it->second;//day为时，换算成每秒的速率
				sum+=it->second;
				if(i<allflow_day[day-1].size()-1)
					ofs<<',';
			}
		}
		else
		{
			int i=0;
			for(auto it=allflow_day[day-1].begin();it!=allflow_day[day-1].end();i++,it++)
			{
				ofs<<(it->second-allflow_day[day-2][it->first]);//day为时，换算成每秒的速率
				sum+=it->second-allflow_day[day-2][it->first];
				if(i<allflow_day[day-1].size()-1)
					ofs<<',';
			}

		}
		ofs<<"];"<<endl;
		cout<<sum<<endl;
		/**************************************/
		
		ofs<<"weight_real=[";
		uint32_t i=0;
		sum=0;
		for(auto it=allflow_day_real[day].begin();it!=allflow_day_real[day].end();i++,it++)
		{
			ofs<<(it->second-allflow_day_real[day-1][it->first]);//day为时，换算成每秒的速率
			sum+=it->second-allflow_day_real[day-1][it->first];
			if(i<allflow_day[day].size()-1)
				ofs<<',';
		}
		ofs<<"];"<<endl;
		cout<<sum<<endl;
	


		/***************************************/
		ofs<<"C=[";
		for(size_t e=0;e<m_network->m_linkCap.size();e++)
		{
			ofs<<m_network->m_linkCap[e];
			if(e<m_network->m_linkCap.size()-1)
				ofs<<',';
		}
		ofs<<"];"<<endl;
		/****************************************************/
		vector<vector<vector<uint32_t> > >delt;
		for(int i=0;i<m_network->m_fineFlowNum;i++)
		{
			vector<vector<uint32_t> >t(P,vector<uint32_t>(m_network->m_topo->m_linkNum,0));
			delt.push_back(t);

		}
		i=0;
		vector<pair<uint32_t,uint32_t> >links=m_network->m_topo->m_links;
		map<uint32_t,vector<vector<uint32_t> > >linkpaths;//链路路径，路径保存的是链路的编号
		for(auto it=paths.begin();it!=paths.end();i++,it++)
		{
			delt[i].clear();
			delt[i]=vector<vector<uint32_t> >(it->second.size(),vector<uint32_t>(m_network->m_topo->m_linkNum,0));
			for(uint32_t p=0;p<it->second.size();p++)
			{
				vector<uint32_t>path=it->second[p];
				vector<uint32_t>linkpath;

				for(uint32_t n=0;n<path.size()-1;n++)
				{
					for(uint32_t e=0;e<links.size();e++)
					{
						if(path[n]==links[e].first&&path[n+1]==links[e].second)
						{
							delt[i][p][e]=1;
							linkpath.push_back(e);
							break;
						}
					}
				}
				linkpaths[it->first].push_back(linkpath);
			}
		}
		linkpaths_day.push_back(linkpaths);
		delt_day.push_back(delt);
		
		ofs<<"d=[";
		for(uint32_t i=0;i<delt.size();i++)
		{
			if(i!=0)
				ofs<<"   ";
			ofs<<'{';
			for(uint32_t p=0;p<delt[i].size();p++)
			{
				ofs<<'<'<<p<<",";
				ofs<<'[';
				for(uint32_t e=0;e<delt[i][p].size();e++)
				{
					ofs<<delt[i][p][e];
					if(e<delt[i][p].size()-1)
						ofs<<',';
				}
				ofs<<"]>";
				if(p<delt[i].size()-1)
					ofs<<',';
			}
			ofs<<'}';
			if(i<delt.size()-1)
				ofs<<','<<endl;
		}
		ofs<<"];"<<endl;

			ofs.close();
			
		}



}
bool cmp(pair<uint32_t,uint32_t>&a,pair<uint32_t,uint32_t>&b)//使得流按照val大到小排序
{
	return a.second>b.second;
}

void 
MeasureAssignmentProblem::Greedy_route(//贪心算法
		const string&outdir,//outdir为输出文件目录
		vector<map<uint32_t,uint32_t> >&allflow_day,//每天的测量流量矩阵,累加
		vector<map<uint32_t,uint32_t> >&allflow_day_real,//每天真实流量矩阵，累加
		vector<map<uint32_t,uint32_t> >&tcam_day,//每天TCAM中记录的流
		vector<map<uint32_t,vector<vector<uint32_t> > > >&linkpaths_day,//每天每条流的可用路径
		vector<map<uint32_t,uint32_t> >&Pi_day,//每天每条流的备选路径
		vector<vector<vector<vector<uint32_t> > > >&delt_day//每天的delt
		)
{
	string file=outdir+"greedy_real_load.txt";
	ofstream ofs(file.c_str());
	for(uint32_t day=1;day<10;day++)
	{
		//取出每天的数据
		map<uint32_t,uint32_t>allflow=allflow_day[day-1];
		if(day>1)
		{
			for(auto it=allflow.begin();it!=allflow.end();it++)
			{
				it->second=it->second-allflow_day[day-2][it->first];
			}
		}
		map<uint32_t,uint32_t>allflow_real=allflow_day_real[day];//后一天的流，用于计算真实链路负载
		for(auto it=allflow_real.begin();it!=allflow_real.end();it++)
		{
			it->second=it->second-allflow_day_real[day-1][it->first];
		}
		map<uint32_t,uint32_t>tcam=tcam_day[day-1];//但前天tcam中的流
		map<uint32_t,vector<vector<uint32_t> > >linkpaths=linkpaths_day[day-1];//但前天所有流的备选链路路径
		map<uint32_t,uint32_t>Pi=Pi_day[day-1];//每条流备选路径数
		vector<vector<vector<uint32_t> > >delt=delt_day[day-1];


		//开始贪心算法
		vector<pair<uint32_t,uint32_t> >heavy;//保存大流
		map<uint32_t,vector<vector<uint32_t> > >postpaths;//大流在测量节点后续的路径
		vector<uint32_t>load(m_network->m_topo->m_linkNum,0);//每条链路的负载
		vector<uint32_t>load_real(load);//后一天的真实负载
		uint64_t heavysum=0;//统计heavy中的总流量
		for(auto it=Pi.begin();it!=Pi.end();it++)
		{
			if(it->second==1)//只有一条路径，更新链路负载和真实链路负载
			{
				vector<uint32_t>path=linkpaths[it->first][0];
				for(uint32_t l:path)
				{
					load[l]+=allflow[it->first];
					load_real[l]+=allflow_real[it->first];
				}
			}
			else if(it->second>1)
			{
				uint32_t measurenode=tcam[it->first];
				uint32_t src=it->first/100000;
				heavy.push_back(make_pair(it->first,allflow[it->first]));
				heavysum+=allflow[it->first];
				if(src==measurenode)
				{
					postpaths[it->first]=linkpaths[it->first];
				}
				else//更新测量节点前的链路负载，将测量节点后的链路路径保存
				{
					vector<uint32_t>path=linkpaths[it->first][0];
					uint32_t i;//记录第几条链路开始是测量节点
					for(i=0;i<path.size();i++)
					{
						uint32_t link=path[i];
						if((m_network->m_topo->m_links[link]).first!=measurenode)
						{
							load[link]+=allflow[it->first];
							load_real[link]+=allflow_real[it->first];
						}
						else
						{
							break;
						}
					}
					for(uint32_t p=0;p<it->second;p++)
					{
						vector<uint32_t>path=linkpaths[it->first][p];
						postpaths[it->first].push_back(vector<uint32_t>(path.begin()+i,path.end()));
					}
				}
			}
			else
			{
				cout<<it->first<<"没有可用路径"<<endl;
				exit(1);
			}
		}
		sort(heavy.begin(),heavy.end(),cmp);
		for(auto flow:heavy)
		{
			uint32_t key=flow.first;
			uint32_t minlen=INF;//记录最短长度
			vector<uint32_t>minpath;//记录最短路径的路径编号
			vector<vector<int> >path_link_rest;//每条路径的链路空闲,即容量-负载
			for(uint32_t p=0;p<postpaths[key].size();p++)
			{
				vector<int>link_rest;//链路空闲,即容量-负载
				for(uint32_t l:postpaths[key][p])
				{
					link_rest.push_back(m_network->m_linkCap[l]-load[l]);
				}
				sort(link_rest.begin(),link_rest.end());
				path_link_rest.push_back(link_rest);
				uint32_t len=postpaths[key][p].size();
				minlen=minlen>len?len:minlen;
			}
			vector<uint32_t>choose,can_choose;//选择的路由集合，备选路由集合
			for(uint32_t p=0;p<path_link_rest.size();p++)
			{
				can_choose.push_back(p);
			}
			for(uint32_t l=0;l<minlen;l++)
			{
				int maxrest=-5000000;
				for(uint32_t p:can_choose)
				{
					if(maxrest<path_link_rest[p][l])
					{
						maxrest=path_link_rest[p][l];
						choose.clear();
						choose.push_back(p);
					}
					else if(maxrest==path_link_rest[p][l])
					{
						choose.push_back(p);
					}
				}
				if(choose.size()==1)
				{
					break;
				}
				minlen=path_link_rest[choose[0]].size();
				for(uint32_t p:choose)//update minlen
				{
					if(minlen>path_link_rest[p].size())
					{
						minlen=path_link_rest[p].size();
					}
					
				}
				can_choose=choose;
			}
			uint32_t xip;
			if(choose.size()>1)
			{
				for(uint32_t p:choose)//选择最短的
				{
					if(path_link_rest[p].size()==minlen)
					{
						xip=p;
						break;
					}
				}
			}
			else
			{
				xip=choose[0];
			}
			//选择完路由后更新链路负载
			vector<uint32_t>path=linkpaths[key][xip];
			for(uint32_t l:path)
			{
				load[l]+=flow.second;
				load_real[l]+=allflow_real[key];
			}
		}
		for(uint32_t l:load_real)
		{
			ofs<<l<<" ";
			//cout<<l<<" ";
		}
		ofs<<endl;
		//cout<<endl;
	}
}
void 
MeasureAssignmentProblem::Random_route(//随机算法
		const string&outdir,//outdir为输出文件目录
		vector<map<uint32_t,uint32_t> >&allflow_day,//每天的测量流量矩阵,累加
		vector<map<uint32_t,uint32_t> >&allflow_day_real,//每天真实流量矩阵，累加
		vector<map<uint32_t,uint32_t> >&tcam_day,//每天TCAM中记录的流
		vector<map<uint32_t,vector<vector<uint32_t> > > >&linkpaths_day,//每天每条流的可用路径
		vector<map<uint32_t,uint32_t> >&Pi_day,//每天每条流的备选路径
		vector<vector<vector<vector<uint32_t> > > >&delt_day//每天的delt
		)
{
	string file=outdir+"random_real_load.txt";
	ofstream ofs(file.c_str());
	for(uint32_t day=1;day<10;day++)
	{
		//取出每天的数据
		map<uint32_t,uint32_t>allflow=allflow_day[day-1];
		if(day>1)
		{
			for(auto it=allflow.begin();it!=allflow.end();it++)
			{
				it->second=it->second-allflow_day[day-2][it->first];
			}
		}
		map<uint32_t,uint32_t>allflow_real=allflow_day_real[day];//后一天的流，用于计算真实链路负载
		for(auto it=allflow_real.begin();it!=allflow_real.end();it++)
		{
			it->second=it->second-allflow_day_real[day-1][it->first];
		}
		map<uint32_t,uint32_t>tcam=tcam_day[day-1];//但前天tcam中的流
		map<uint32_t,vector<vector<uint32_t> > >linkpaths=linkpaths_day[day-1];//但前天所有流的备选链路路径
		map<uint32_t,uint32_t>Pi=Pi_day[day-1];//每条流备选路径数
		vector<vector<vector<uint32_t> > >delt=delt_day[day-1];


		//随机算法
		vector<uint32_t>load(m_network->m_topo->m_linkNum,0);//每条链路的负载
		vector<uint32_t>load_real(load);//后一天的真实负载
		for(auto it=Pi.begin();it!=Pi.end();it++)
		{
			uint32_t xip=rand()%it->second;
			vector<uint32_t>path=linkpaths[it->first][xip];
			for(uint32_t l:path)
			{
				load[l]+=allflow[it->first];
				load_real[l]+=allflow_real[it->first];
			}
		}
		for(uint32_t l:load_real)
		{
			ofs<<l<<" ";
//			cout<<l<<" ";
		}
		ofs<<endl;
//		cout<<endl;
	}
}
void
MeasureAssignmentProblem::Original_route(//原始算法
		const string&outdir,//outdir为输出文件目录
		vector<map<uint32_t,uint32_t> >&allflow_day,//每天的测量流量矩阵,累加
		vector<map<uint32_t,uint32_t> >&allflow_day_real,//每天真实流量矩阵，累加
		vector<map<uint32_t,uint32_t> >&tcam_day,//每天TCAM中记录的流
		vector<map<uint32_t,vector<vector<uint32_t> > > >&linkpaths_day,//每天每条流的可用路径
		vector<map<uint32_t,uint32_t> >&Pi_day,//每天每条流的备选路径
		vector<vector<vector<vector<uint32_t> > > >&delt_day//每天的delt
		)
{
	string file=outdir+"original_real_load.txt";
	ofstream ofs(file.c_str());
	for(uint32_t day=1;day<10;day++)
	{
		//取出每天的数据
		map<uint32_t,uint32_t>allflow_real=allflow_day_real[day];//后一天的流，用于计算真实链路负载
		for(auto it=allflow_real.begin();it!=allflow_real.end();it++)
		{
			it->second=it->second-allflow_day_real[day-1][it->first];
		}
		map<uint32_t,vector<vector<uint32_t> > >linkpaths=linkpaths_day[day-1];//但前天所有流的备选链路路径
		map<uint32_t,uint32_t>Pi=Pi_day[day-1];//每条流备选路径数


		vector<uint32_t>load(m_network->m_topo->m_linkNum,0);//每条链路的负载
		vector<uint32_t>load_real(load);//后一天的真实负载
		for(auto it=Pi.begin();it!=Pi.end();it++)
		{
			vector<uint32_t>shortestpath=linkpaths[it->first][0];
			for(uint32_t l:shortestpath)
			{
				load_real[l]+=allflow_real[it->first];
			}
		}
		for(uint32_t l:load_real)
		{
			ofs<<l<<" ";
//			cout<<l<<" ";
		}
		ofs<<endl;
//		cout<<endl;
	}
}
