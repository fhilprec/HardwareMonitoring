//
// Created by aleks-tu on 8/14/24.
//

#ifndef GRAPH_H
#define GRAPH_H
#include <unordered_set>
#include <vector>
#include <stdexcept>
#include <algorithm>

template <typename T>
class Graph {
    std::vector<T> nodes;
    std::vector<std::vector<bool>> adjancancyMatrix;
    void addNode(const T& node);
public:
    Graph() : Graph(std::vector<T>()){}
    explicit Graph(const std::vector<T>& nodes): nodes(nodes), adjancancyMatrix(std::vector<std::vector<bool>>(nodes.size()))
    {
        for (int i = 0; i < nodes.size(); ++i)
        {
            adjancancyMatrix.push_back(std::vector<bool>(nodes.size(),false));
        }
    }
    void addEdge(const T& node, std::vector<T> otherNodes);
    std::vector<T> topologicalSort();
};

template <typename T>
void Graph<T>::addNode(const T& node)
{
    nodes.push_back(node);
    for (auto & adjancancy_matrix : adjancancyMatrix)
    {
        adjancancy_matrix.push_back(false);
    }
    adjancancyMatrix.push_back(std::vector<bool>(nodes.size(),false));
}

template <typename T>
void Graph<T>::addEdge(const T& node, std::vector<T> otherNodes)
{
    const size_t index = std::distance(nodes.begin(),std::find(nodes.begin(), nodes.end(), node));
    if(index == nodes.size())
    {
        addNode(node);
    }
    for (const auto & otherNode : otherNodes)
    {
        const size_t otherIndex = std::distance(nodes.begin(),std::find(nodes.begin(), nodes.end(), otherNode));
        if(otherIndex == nodes.size())
        {
            addNode(otherNode);
        }
        adjancancyMatrix[otherIndex][index] = true;
    }
}

template <typename T>
std::vector<T> Graph<T>::topologicalSort()
{
    std::vector<T> result;
    std::vector<T> nodesWithNoIncomingEdge;

    for (int i = 0; i < nodes.size(); ++i)
    {
        size_t nodesPointingToCurrentNode = 0;
        for (int j = 0; j < nodes.size(); ++j)
        {
            nodesPointingToCurrentNode += adjancancyMatrix[j][i];
        }
        if(!nodesPointingToCurrentNode)
        {
            nodesWithNoIncomingEdge.push_back(nodes[i]);
        }
    }

    while (!nodesWithNoIncomingEdge.empty())
    {
        T currentNode = nodesWithNoIncomingEdge.back();
        nodesWithNoIncomingEdge.pop_back();
        result.push_back(currentNode);

        const size_t index = std::distance(nodes.begin(),std::find(nodes.begin(), nodes.end(), currentNode));
        for (int otherIndex = 0; otherIndex < nodes.size(); ++otherIndex)
        {
            if(adjancancyMatrix[index][otherIndex])
            {
                adjancancyMatrix[index][otherIndex] = false;
                size_t incomgingEdges = 0;
                for (int i = 0; i < nodes.size(); ++i)
                {
                    incomgingEdges += adjancancyMatrix[i][otherIndex];
                }
                if(!incomgingEdges)
                {
                    nodesWithNoIncomingEdge.push_back(nodes.at(otherIndex));
                }
            }
        }
    }

    for (int i = 0; i < nodes.size(); ++i)
    {
        for (int j = 0; j < nodes.size(); ++j)
        {
            if(adjancancyMatrix[i][j])
            {
                throw std::runtime_error("Graph has Cycles");
            }
        }
    }
    return result;
}


#endif //GRAPH_H
