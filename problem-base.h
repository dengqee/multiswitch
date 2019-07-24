/*************************************************************************
  > File Name: problem-base.h
  > Author: dengqi
  > Mail: 906262038@qq.com 
  > Created Time: 2019年07月05日 星期五 10时43分22秒
 ************************************************************************/
#ifndef PROBLEM_BASE_H
#define PROBLEM_BASE_H

#include<memory>
#include<stdint.h>
#include<iostream>
#include"topology.h"
#include"network.h"
using namespace std;
class ProblemBase
{
protected:
	shared_ptr<Network> m_network;
		
public:
	ProblemBase();

	ProblemBase(shared_ptr<Network> network);

	virtual	~ProblemBase();

	virtual void SetNetwork(const shared_ptr<Network> network) = 0;

	virtual shared_ptr<Network> GetNetwork() = 0;

	virtual void run() = 0;

	virtual void Print()=0;

	virtual void OutPut(const string &) = 0;

};
#endif /* PROBLEM_BASE_H */
