/*************************************************************************
	> File Name: network.h
	> Author: dengqi
	> Mail: 906262038@qq.com 
	> Created Time: 2019年07月05日 星期五 15时02分41秒
 ************************************************************************/
#ifndef NETWORK_H
#define NETWORK_H

#include<iostream>
#include"topology.h"
#include<vector>
#include<set>
#include<string>

using namespace std;

class Flow
{
public:
	
	uint32_t m_src;
	uint32_t m_dst;
	uint32_t m_weight;

	typedef vector<vector<pair<uint32_t,uint32_t> > > LinkPaths;
	typedef vector<vector<uint32_t> > NodePaths;

	LinkPaths m_linkPaths;//include multipath
	NodePaths m_nodePaths;//include multipath

	Flow(uint32_t src, uint32_t dst, uint32_t weight = 1);

	void SetPaths(const LinkPaths &linkPaths, const NodePaths &nodePaths);


};

class Network
{
private:
	Topology m_topo;
	vector<Flow> m_flows;
	uint32_t m_coarseFlowNum;
	uint32_t m_fineFlowNum;
	set<uint32_t> m_measureNodes;//nodes for measure

public:
	Network(const string &topoFileName, const string &flowFileName);
	
	void ReadFlow(const string &flowFileName);

	


};

#endif /* NETWORK_H */
