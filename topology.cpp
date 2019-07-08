/*************************************************************************
	> File Name: topology.cpp
	> Author: dengqi
	> Mail: 906262038@qq.com 
	> Created Time: 2019年07月05日 星期五 11时22分36秒
 ************************************************************************/

#include<iostream>
#include"topology.h"
#include<fstream>
#include<sstream>
#include<cstdlib>
using namespace std;

Topology::Topology():
	m_nodeNum(0),
	m_linkNum(0),
	m_links(vector<pair<uint32_t,uint32_t> >()),
	m_paths(map<pair<uint32_t,uint32_t>,vector<uint32_t> >())
{

}

Topology::Topology(const string &topoName)
{
	ReadTopology(topoName);
}

Topology::Topology(const Topology &topo):
	m_nodeNum(topo.m_nodeNum),
	m_linkNum(topo.m_linkNum),
	m_links(topo.m_links)
{
	
}

Topology::~Topology()
{

}

void
Topology::ReadTopology(const string &topoName)
{
	cout<<"Read topology file..."<<endl;
	ifstream ifs(topoName.c_str());
	if(!ifs.good())
	{
		cerr<<"file "<<topoName<<" open erro!"<<endl;
		exit(1);
	}
	uint32_t nodeNum;
	uint32_t linkNum = 0;
	uint32_t src, dst;
	vector<pair<uint32_t,uint32_t> > links;

	istringstream lineBuffer;
	string line;
	getline(ifs,line);
	lineBuffer.str(line);
	lineBuffer>>nodeNum;

	while(getline(ifs,line))
	{
		lineBuffer.clear();
		lineBuffer.str(line);
		lineBuffer>>src;
		lineBuffer>>dst;
		links.push_back(make_pair(src,dst));
		
	}
	m_nodeNum=nodeNum;
	m_linkNum=links.size();
	m_links=links;
	
	ifs.close();
	cout<<"node: "<<m_nodeNum<<" link: "<<m_linkNum<<endl;
	cout<<"Read topology complete!"<<endl;
}

void
Topology::ShortestPathGen()
{
	if(m_links.empty())
		return;
	m_paths.clear();
	cout<<"calculate shortest paths..."<<endl;

	vector<vector<uint32_t> > dist(m_nodeNum, vector<uint32_t>(m_nodeNum,INF));
	vector<vector<uint32_t> > path(m_nodeNum, vector<uint32_t>(m_nodeNum,0));

	for(auto it=m_links.begin();it!=m_links.end();it++)
	{
		dist[it->first][it->second]=1;
		dist[it->second][it->first]=1;
	}
	for(uint32_t i=0;i<m_nodeNum;i++)
	{
		dist[i][i]=0;
		for(uint32_t j=0;j<m_nodeNum;j++)
			path[i][j]=j;
	}

	//Floyd
	uint32_t select=0;
	for(uint32_t temp=0;temp<m_nodeNum;temp++)
		for(uint32_t row=0;row<m_nodeNum;row++)
			for(uint32_t col=0;col<m_nodeNum;col++)
			{
				select=(dist[row][temp]==INF||dist[temp][col]==INF)?INF:(dist[row][temp]+dist[temp][col]);
				if(dist[row][col]>select)
				{
					dist[row][col]=select;
					path[row][col]=path[row][temp];
				}
			}

	//find shortest path
	for(uint32_t row=0;row<m_nodeNum;row++)
		for(uint32_t col=0;col<m_nodeNum;col++)
		{
			if(row==col)
				continue;
			vector<uint32_t>tempPath;
			uint32_t temp=path[row][col];
			tempPath.push_back(row);
			tempPath.push_back(temp);
			while(temp!=col)
			{
				temp=path[temp][col];
				tempPath.push_back(temp);
			}
			m_paths[make_pair(row,col)]=tempPath;
				
		}
	cout<<"calculate shortest paths complete!"<<endl;
}

void
Topology::ReadPath(const string &pathFileName)
{
	cout<<"read path from file:"<<pathFileName<<endl;
	ifstream ifs(pathFileName.c_str());
	if(!ifs.good())
	{
		cerr<<"open file error:"<<pathFileName<<endl;
		exit(1);
	}
	m_paths.clear();
	string line;
	istringstream lineBuffer;
	uint32_t src,dst;

	while(getline(ifs,line))
	{
		lineBuffer.clear();
		lineBuffer.str(line);
		lineBuffer>>src>>dst;
		vector<uint32_t>tempPath;

		getline(ifs,line);
		lineBuffer.clear();
		lineBuffer.str(line);
		uint32_t temp;
		while(lineBuffer>>temp)
			tempPath.push_back(temp);
		m_paths[make_pair(src,dst)]=tempPath;
	}
	ifs.close();
	cout<<"read path from file complete!"<<endl;


}

void
Topology::WritePath(const string &pathFileName)
{
	cout<<"write path to file..."<<endl;

	ofstream ofs(pathFileName.c_str());
	if(!ofs.good())
	{
		cerr<<"file write error:"<<pathFileName<<endl;
		exit(1);
	}
	for(auto it=m_paths.begin();it!=m_paths.end();it++)
	{
		ofs<<it->first.first<<" "<<it->first.second<<endl;
		for(auto node:it->second)
			ofs<<node<<" ";
		ofs<<endl;
	}
	ofs.close();
	cout<<"write path to file complete!"<<endl;

}

uint32_t
Topology::GetPath(uint32_t src,uint32_t dst,vector<uint32_t>&path)
{
	map<pair<uint32_t,uint32_t>,vector<uint32_t> >::iterator it;
	it=m_paths.find(make_pair(src,dst));
	if(it!=m_paths.end())
	{
		path=it->second;
		return path.size();
	}
	else 
	{
		return 0;
	}
}
