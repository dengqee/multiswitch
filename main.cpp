/*************************************************************************
  > File Name: main.cpp
  > Author: dengqi
  > Mail: 906262038@qq.com 
  > Created Time: 2019年07月05日 星期五 15时56分50秒
 ************************************************************************/

#include<iostream>
#include"topology.h"
#include"network.h"
#include<string>

using namespace std;
int main()
{
	string dir="./data/";
	string topoName="Geant";
	string topoFileName=dir+"Topology_"+topoName+".txt";

	string flowFileName=dir+topoName+"flow.txt"; 

	string pathFileName=dir+topoName+"Path.txt";

	//Network geant(topoFileName,flowFileName);


	Topology geant(topoFileName);
	geant.ShortestPathGen();
	//geant.WritePath(pathFileName);
	geant.ReadPath(pathFileName);

	vector<uint32_t> path;
	if(geant.GetPath(231,10,path))
		for(auto i:path)
			cout<<i<<" ";
	else
		cout<<"can't find path"<<endl;



	return 0;
}
