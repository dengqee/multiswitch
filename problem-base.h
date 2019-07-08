/*************************************************************************
  > File Name: problem-base.h
  > Author: dengqi
  > Mail: 906262038@qq.com 
  > Created Time: 2019年07月05日 星期五 10时43分22秒
 ************************************************************************/
#ifndef PROBLEM_BASE_H
#define PROBLEM_BASE_H

#include<iostream>
#include"topology.h"
#include"network.h"
using namespace std;
class ProblemBase
{
protected:
	Network m_network;
		
public:
	ProblemBase();

	ProblemBase(Network network);

	virtual	~ProblemBase();

	virtual SetNetwork(const Network &network) = 0;

	virtual Network GetNetwork() = 0;

	virtual void run() = 0;

};
#endif /* PROBLEM_BASE_H */
