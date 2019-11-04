/*************************************************************************
	> File Name: plasement-problem.cpp
	> Author: dengqi
	> Mail: 906262038@qq.com 
	> Created Time: 2019年07月08日 星期一 10时36分31秒
 ************************************************************************/

#include<iostream>
#include"plasement-problem.h"
#include<algorithm>
#include<iterator>
#include<fstream>
using namespace std;

PlasementProblem::PlasementProblem()
{

}

PlasementProblem::PlasementProblem(shared_ptr<Network> network,uint32_t k):
	ProblemBase(network),
	m_measureNodeNum(k),
	m_measureNodes(set<uint32_t>()),
	m_solved(false)
{

}

PlasementProblem::~PlasementProblem()
{

}

void
PlasementProblem::SetNetwork(const shared_ptr<Network> network)
{
	m_network=network;
}

void 
PlasementProblem::SetMeasureNodeNum(uint32_t k)
{
	m_measureNodeNum=k;
}

uint32_t 
PlasementProblem::GetMeasureNodeNum()
{
	return m_measureNodeNum;
}

set<uint32_t>
PlasementProblem::GetMeasureNodes()
{
	return m_measureNodes;
}

shared_ptr<Network>
PlasementProblem::GetNetwork()
{
	return m_network;
}

void 
PlasementProblem::run()
{
	if(m_measureNodeNum==0)	
		return;
	m_measureNodes.clear();
	bool succeed=false;

	set<uint32_t>coveredFlows;//coveredFlows id

	set<uint32_t>unusedNodes;//set of unused node;
	for(uint32_t i=0;i<m_network->m_topo->GetNodeNum();i++)
		unusedNodes.insert(i);
	

	for(uint32_t i=0;i<m_measureNodeNum;++i)
	{
		if(unusedNodes.size()==0)
			break;
		uint32_t maxNode=*(unusedNodes.begin());
		set<uint32_t> flowIDOnNode=m_network->GetFlowOnNode(maxNode);
		
		set<uint32_t> diff;
		set<uint32_t> diff_max;
		set_difference(flowIDOnNode.begin(),flowIDOnNode.end(),coveredFlows.begin(),coveredFlows.end(),inserter(diff,diff.begin()));
		diff_max=diff;
		uint32_t maxFlowInc=diff.size();

		//find maximizing the ^{si|S}
		for(auto it=unusedNodes.begin();it!=unusedNodes.end();it++)
		{
			flowIDOnNode.clear();
			diff.clear();

			flowIDOnNode=m_network->GetFlowOnNode(*it);

			set_difference(flowIDOnNode.begin(),flowIDOnNode.end(),coveredFlows.begin(),coveredFlows.end(),inserter(diff,diff.begin()));
			if(maxFlowInc<diff.size())
			{
				maxFlowInc=diff.size();
				maxNode=*it;
				diff_max=diff;

			}

		}
		m_measureNodes.insert(maxNode);
		unusedNodes.erase(maxNode);
		coveredFlows.insert(diff_max.begin(),diff_max.end());
		if(coveredFlows.size()==m_network->m_coarseFlowNum)
		{
			coveredFlows.clear();
			succeed=true;
		}
		
	}
	if(succeed)
	{
		m_solved=true;
	}
	/*
	for(uint32_t i=0;i<22*23;i++)
	{
		if(coveredFlows.find(i)==coveredFlows.end())
			cout<<i<<endl;
	}
	*/
	m_network->m_measureNodes=m_measureNodes;
	m_network->m_measureNodeNum=m_measureNodeNum;
}

void 
PlasementProblem::Print()
{
	cout<<"m_solved="<<m_solved<<endl;
	cout<<"m_measureNodeNum="<<m_measureNodeNum<<endl;
	for(auto it=m_measureNodes.begin();it!=m_measureNodes.end();it++)
		cout<<*it<<" ";
	cout<<endl;
}

void 
PlasementProblem::OutPut(const string &outfile)
{
	ofstream ofs(outfile.c_str());
	for(auto it=m_measureNodes.begin();it!=m_measureNodes.end();it++)
		ofs<<*it<<" ";
	ofs.close();
}
