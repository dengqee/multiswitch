/*************************************************************************
	> File Name: plasement-problem.h
	> Author: dengqi
	> Mail: 906262038@qq.com 
	> Created Time: 2019年07月08日 星期一 10时16分18秒
 ************************************************************************/

#ifndef PLASEMENT_PROBLEM_H
#define PLASEMENT_PROBLEM_H

#include<iostream>
#include<stdint.h>
#include"problem-base.h"
using namespace std;

class PlasementProblem:public ProblemBase
{
private:
	uint32_t m_measureNodeNum;
	set<uint32_t> m_measureNodes;
	bool m_solved;
public:
	PlasementProblem();

	PlasementProblem(shared_ptr<Network> network,uint32_t k);

	virtual ~PlasementProblem();

	virtual void SetNetwork(const shared_ptr<Network> network);

	void SetMeasureNodeNum(uint32_t k);

	uint32_t GetMeasureNodeNum();

	set<uint32_t> GetMeasureNodes();

	virtual shared_ptr<Network> GetNetwork();

	virtual void run();//greedy algrothm

	virtual void Print();
	
	void OutPut(const string &outfile);


};

#endif /* PLASEMENT_PROBLEM_H */
