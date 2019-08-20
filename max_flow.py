from random import randint
from math import floor
def max_flow(nodes, paths, value):
    topo = make_bigraph_topo(nodes, paths, value)
    flag = 0
    while flag == 0:
        print(value)
        bi_paths = []
        itte=1
        for i in range(len(paths)):
            # if i%100==0:
            #     print(i)
            bi_path = [0] + dfs(topo, 0, [0 for j in range(len(topo))], []) + [len(topo) - 1]
            if len(bi_path) == 2:
                itte=itte+1
                value =floor(value * 1.2)+1
                topo = make_bigraph_topo(nodes, paths, value)
                break
            else:
                bi_paths.append(bi_path)
                for j in range(0, len(bi_path) - 1):
                    if bi_path[j + 1] not in topo[bi_path[j]].keys():
                        topo[bi_path[j]][bi_path[j + 1]]=0
                    topo[bi_path[j]][bi_path[j + 1]] = topo[bi_path[j]][bi_path[j + 1]] - 1
                    if bi_path[j] not in topo[bi_path[j + 1]].keys():
                        topo[bi_path[j + 1]][bi_path[j]]=0
                    topo[bi_path[j + 1]][bi_path[j]] = 1 + topo[bi_path[j + 1]][bi_path[j]]
        if len(bi_paths) == len(paths):
            flag = 1
    flows = [[] for i in nodes]
    for i in range(1,len(paths)+1):
        for key,value in topo[i].items():
            if value==0:
                flows[key-len(paths)-1].append(i-1)
    return flows,itte


def make_bigraph_topo(nodes, paths, value):
    nodenum = len(nodes) + len(paths) + 2
    linkdict={}
    for i in range(nodenum):
        linkdict[i]={}
    for i in range(len(paths)):
        linkdict[0][i+1]=1
    for i, path in enumerate(paths):
        for j in path:
            linkdict[i+1][j + len(paths) + 1]=1
    for i in nodes:
        linkdict[i + len(paths) + 1][len(paths)+len(nodes)+1]=value
    return linkdict


def dfs(topo, s, flag, path=[]):
    if len(topo) - 1 in topo[s].keys():
        if topo[s][len(topo) - 1]>0:
            return path
    for i in topo[s].keys():
        if topo[s][i]>0 and flag[i] == 0:
            flag[i] = 1
            p=dfs(topo, i, flag, path + [i])
            if len(p)!=0:
                return  p
    return ([])


if __name__ == "__main__":
    dir="./data/"
    topo="Geant"
    pathsfile=dir+topo+"_FlowPath.txt"
    paths=[]
    path=[]
    with open(pathsfile,"r") as f:
        for line in f.readlines():
            path=[int(x) for x in line.split()]
            paths.append(path)
    measureflie=dir+topo+"_MeasureNodes.txt"
    nodes=[]
    with open(measureflie,"r") as f:
        for line in f.readlines():
            nodes=[int(x) for x in line.split()]
    flows,itte=max_flow(nodes,paths,floor(len(paths)/len(nodes))+1)
    print(itte)