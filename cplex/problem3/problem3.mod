/*********************************************
 * OPL 12.7.1.0 Model
 * Author: dengqi
 * Creation Date: 2019年8月29日 at 上午10:09:09
 *********************************************/
 int F=...;//粗流总数
 int S=...;//测量节点数量
 int M=...;//分段函数的段数
 range conRange=0..M-1;//分段函数条件的范围
 range flows=0..F-1;//粗流集合
 range measureNodeRange=0..S-1;//测量节点范围
 //{int} measrueNode=...;//测量节点集合,即S
 float A[conRange]=...;//斜率
 float B[conRange]=...;//截距(正数)
 {int} Si[flows]=...;//每条流分别经过的测量节点序号的集合
 int weight[flows]=...;//每条粗流的权重
 int N[measureNodeRange]=...;//测量节点的容量
  
 dvar float+ x[flows][measureNodeRange];
 dvar float phy[measureNodeRange];
 dvar int load[measureNodeRange];
 dvar int total;
 minimize
 	sum(v in measureNodeRange)phy[v];
 
 subject to
 {
// 	forall(i in flows)
// 	  forall(v in measureNodeRange)
// 	    x[i][v]>=0; 
 
 	forall(i in flows)
 	  sum(v in Si[i])x[i][v]>=1;
 	forall(v in measureNodeRange)
 	  load[v]==sum(i in flows)weight[i]*x[i][v];
 	forall(v in measureNodeRange)
	  {
	  	forall(m in conRange)
	  	  A[m]*load[v]-B[m]*N[v]<=phy[v];
	  }
	  total==sum(v in measureNodeRange)load[v];
 }
 
 