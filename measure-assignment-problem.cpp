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
	//	m_nodeCap.push_back(rand()%);
	}
	for(int node:m_measureNodes)
	{
		m_nodeCap.push_back(all_cap[node]);
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
	int sum=0;
	while(getline(ifs,line))
	{
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
			}
		}
		//if(!flag)
		//{
		//	fs2.insert(s*100000+t*1000+num);
		//	sum++;

		//}

	}
	cout<<fs2.size()<<" "<<sum<<endl;//输出的是未写入的流
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
	int sum=0;
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
				break;
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
