/*************************************************************************
	> File Name: topology.h
	> Author: dengqi
	> Mail: 906262038@qq.com 
	> Created Time: 2019年07月05日 星期五 18时42分07秒
 ************************************************************************/
#ifndef TOPOLOGY_H
#define TOPOLOGY_H

#include<iostream>
#include<cstdlib>
#include<vector>
#include<string>
#include<map>

using namespace std;

#define INF 1<<31
class Topology
{
private:
	uint32_t m_nodeNum;
	uint32_t m_linkNum;
	vector<pair<uint32_t,uint32_t>>m_links;
	map<pair<uint32_t,uint32_t>,vector<uint32_t> > m_paths;//all shortest paths
public:
	Topology();
	Topology(const string &topoName);

	Topology(const Topology &topo);

	virtual ~Topology();

	void ReadTopology(const string &topoName);

	void ShortestPathGen();//calculate all shortest path of every node pairs by Floyd 

	void ReadPath(const string &pathFileName);//read shortest paths from file

	void WritePath(const string &pathFileName);//Write paths to file

	uint32_t GetPath(uint32_t src, uint32_t dst, vector<uint32_t>&path);//return the length of path 


	


};
#endif /* TOPOLOGY_H */
