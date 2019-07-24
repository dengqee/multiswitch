/*************************************************************************
	> File Name: network.h
	> Author: dengqi
	> Mail: 906262038@qq.com 
	> Created Time: 2019年07月05日 星期五 15时02分41秒
 ************************************************************************/
#ifndef NETWORK_H
#define NETWORK_H

#include<stdint.h>
#include<iostream>
#include"topology.h"
#include<vector>
#include<set>
#include<string>
#include<memory>

using namespace std;

class Flow
{
public:
	uint32_t m_id;//flow id number
	uint32_t m_src;
	uint32_t m_dst;
	uint32_t m_weight;

	typedef vector<vector<pair<uint32_t,uint32_t> > > LinkPaths;
	typedef vector<vector<uint32_t> > NodePaths;

	LinkPaths m_linkPaths;//include multipath
	NodePaths m_nodePaths;//include multipath

	Flow(uint32_t id, uint32_t src, uint32_t dst, uint32_t weight = 1);

	void SetPaths(const LinkPaths &linkPaths, const NodePaths &nodePaths);



};


inline bool operator < (const Flow &lhs, const Flow &rhs);

class Network
{
public:
	shared_ptr<Topology> m_topo;
	vector<shared_ptr<Flow> > m_flows;
	uint32_t m_coarseFlowNum;
	uint32_t m_fineFlowNum;
	map<uint32_t, set<uint32_t> >m_flowOnNode;//the set of flow ID of every node
	set<uint32_t> m_measureNodes;//nodes for measure
	uint32_t m_measureNodeNum;//the number of measure nodes

	Network(const string &topoFileName, const string &flowFileName);

	Network(const Network &network);//copy constructor

	void SetMeasureNodeNum(uint32_t n);

	uint32_t GetMeasureNodes(set<uint32_t> &measureNodes);//return measureNodesnum
	
	void ReadFlow(const string &flowFileName);

	void GenFlowPath(); 

	set<uint32_t> GetFlowOnNode(uint32_t nodeID);

	void Print();//print infomation of network

	void OutPut(const string &fileName);//output fine flow path to file 

};

#endif /* NETWORK_H */
