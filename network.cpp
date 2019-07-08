/*************************************************************************
	> File Name: network.cpp
	> Author: dengqi
	> Mail: 906262038@qq.com 
	> Created Time: 2019年07月07日 星期日 14时50分10秒
 ************************************************************************/

#include"network.h"
#include<iostream>
#include<fstream>
#include<sstream>

using namespace std;

Flow::Flow(uint32_t src, uint32_t dst, uint32_t weight):
	m_src(src),
	m_dst(dst),
	m_weight(weight),
	m_linkPaths(LinkPaths()),
	m_nodePaths(NodePaths())
{

}

void
Flow::SetPaths(const LinkPaths &linkPaths, const NodePaths &nodePaths)
{
	m_linkPaths = linkPaths;
	m_nodePaths = nodePaths;
}

Network::Network(const string &topoFileName, const string &flowFileName):
	m_topo(topoFileName),
	m_flows(vector<Flow>()),
	m_coarseFlowNum(0),
	m_fineFlowNum(0),
	m_measureNodes(set<uint32_t>()),
	m_measureNodeNum(0)
{
	ReadFlow(flowFileName);
}

void 
Network::SetMeasureNodeNum(uint32_t n)
{
	m_measureNodeNum=n;
}

uint32_t 
Network::GetMeasureNodes(set<uint32_t> &measureNodes)
{
	if(m_measureNodes.size()==0)
		return 0;
	measureNodes=m_measureNodes;
	return m_measureNodeNum;
}

void 
Network::ReadFlow(const string &flowFileName)
{
	ifstream ifs(flowFileName.c_str());
	if(!ifs.good())
	{

		cerr<<"file "<<flowFileName<<" open erro!"<<endl;
		exit(1);
	}

	cout<<"Read flow..."<<endl;

	uint32_t src,dst,weight;
	string line;
	istringstream lineBuffer;
	while(getline(ifs,line))
	{
		//read src,dst,weight
		lineBuffer.clear();
		lineBuffer.str(line);
		lineBuffer>>src>>dst;
		getline(ifs,line);
		lineBuffer.clear();
		lineBuffer.str(line);
		lineBuffer>>weight;
		//create flow
		Flow flow(src,dst,weight);
		m_flows.push_back(flow);
		m_fineFlowNum+=weight;
	}
	m_coarseFlowNum=m_flows.size();
	ifs.close();

	cout<<"coarse flow num: "<<m_coarseFlowNum<<" fine flow num: "<<m_fineFlowNum<<endl;
	cout<<"Read flow complete!"<<endl;
}

void 
Network::GenFlowPath()
{
	if(m_flows.empty())
	{
		cerr<<"flows is empty, please call ReadFlow first"<<endl;
		exit(1);
	}
	m_topo.ShortestPathGen();
	for(auto flow:m_flows)
	{
		vector<uint32_t>nodePath;
		int length=m_topo.GetPath(flow.m_src,flow.m_dst,nodePath);
		flow.m_nodePaths.push_back(nodePath);
		if(length==0)
		{
			cout<<flow.m_src<<"->"<<flow.m_dst<<" can't find path"<<endl;
			continue;
		}
		//generate linkPaths
		vector<pair<uint32_t,uint32_t> >linkPath;
		for(size_t i=0;i<nodePath.size()-1;i++)
		{
			linkPath.push_back(make_pair(nodePath[i],nodePath[i+1]));	
		}
		flow.m_linkPaths.push_back(linkPath);
	}

}

void 
Network::Print()
{
	//m_topo.Print();
	cout<<"the number of coarse flows: "<<m_coarseFlowNum<<endl;
	cout<<"the number of fin flows: "<<m_fineFlowNum<<endl;

}
