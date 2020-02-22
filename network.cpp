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

Flow::Flow(uint32_t id,uint32_t src, uint32_t dst, uint32_t weight):
	m_id(id),
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

inline bool 
operator < (const Flow &lhs, const Flow &rhs)
{
	return lhs.m_id<rhs.m_id;
}

Network::Network(const string &topoFileName, const string &flowFileName):
	m_topo(new Topology(topoFileName)),
	m_flows(vector<shared_ptr<Flow> >()),
	m_coarseFlowNum(0),
	m_fineFlowNum(0),
	m_flowOnNode(map<uint32_t,set<uint32_t> >()),
	m_measureNodes(set<uint32_t>()),
	m_measureNodeNum(0),
	m_linkCap(vector<uint64_t>())
{
	ReadFlow(flowFileName);
	CalLinkCapcity();
}

Network::Network(const Network &network):
	m_topo(network.m_topo),
	m_flows(network.m_flows),
	m_coarseFlowNum(network.m_coarseFlowNum),
	m_fineFlowNum(network.m_fineFlowNum),
	m_flowOnNode(network.m_flowOnNode),
	m_measureNodes(network.m_measureNodes),
	m_measureNodeNum(network.m_measureNodeNum),
	m_linkCap(network.m_linkCap)
{

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
	uint32_t flowID=0;
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
		shared_ptr<Flow> flow(new Flow(flowID,src,dst,weight));
		m_flows.push_back(flow);
		m_fineFlowNum+=weight;
		flowID++;
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
	m_topo->ShortestPathGen();
	for(auto flow:m_flows)
	{
		vector<uint32_t>nodePath;
		int length=m_topo->GetPath(flow->m_src,flow->m_dst,nodePath);
		flow->m_nodePaths.push_back(nodePath);

		if(length==0)
		{
			cout<<flow->m_src<<"->"<<flow->m_dst<<" can't find path"<<endl;
			continue;
		}
		//generate linkPaths
		vector<pair<uint32_t,uint32_t> >linkPath;
		for(size_t i=0;i<nodePath.size()-1;i++)
		{
			linkPath.push_back(make_pair(nodePath[i],nodePath[i+1]));	

			//generate m_flowOnNode 
			m_flowOnNode[nodePath[i]].insert(flow->m_id);
		}
		m_flowOnNode[nodePath[nodePath.size()-1]].insert(flow->m_id);
		flow->m_linkPaths.push_back(linkPath);
	}

}

set<uint32_t>
Network::GetFlowOnNode(uint32_t nodeID)
{
	auto it=m_flowOnNode.find(nodeID);
	if(it==m_flowOnNode.end())
	{
		cerr<<"can't find node "<<nodeID<<" in m_flowOnNode"<<endl;
		exit(1);
	}
	return it->second;
}

void 
Network::Print()
{
	//m_topo.Print();
	cout<<"the number of coarse flows: "<<m_coarseFlowNum<<endl;
	cout<<"the number of fin flows: "<<m_fineFlowNum<<endl;

}

void 
Network::CalLinkCapcity()
{
	uint32_t node1,node2,degree1,degree2;
	vector<uint32_t>t;
	for(uint32_t i=0;i<m_topo->m_links.size();i++)
	{
		node1=m_topo->m_links[i].first;
		node2=m_topo->m_links[i].second;
		degree1=m_topo->GetDegree(node1,t);
		degree2=m_topo->GetDegree(node2,t);
		if(degree1>3&&degree2>3)
			m_linkCap.push_back(OC192);
		else if(degree1<=3&&degree2<=3)
			m_linkCap.push_back(OC24);
		else
			m_linkCap.push_back(OC48);

	}
};
