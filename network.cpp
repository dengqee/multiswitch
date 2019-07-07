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
	m_measureNodes(set<uint32_t>())
{
	ReadFlow(flowFileName);
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


