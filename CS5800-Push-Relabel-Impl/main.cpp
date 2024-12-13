//
//  main.cpp
//  CS5800-Push-Relabel-Impl
//
//  Created by Logan Gill on 12/10/24.
//

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <fstream>
#include <sstream>

using std::vector;
using std::map;
using std::ifstream;
using std::string;
using std::cin;
using std::cout;
using std::endl;

/// Represents a node in a graph with a number, height, and eccess flow
class Node {
public:
    int num, height, ef;
    
    Node(int n) {
        num = n;
        height = 0;
        ef = 0;
    }
    
    Node() {
        num = -1;
        height = 0;
        ef = 0;
    }
    
    bool operator==(Node o) {
        return num == o.num;
    }
    
    bool operator!=(Node o) {
        return !(*this == o);
    }
    
    bool operator==(int o) {
        return num == o;
    }
    
    bool operator!=(int o) {
        return !(*this == o);
    }
};

bool operator==(const Node& lhs, const Node& rhs) {
    return lhs.num == rhs.num;
}

bool operator!=(const Node& lhs, const Node& rhs) {
    return !(lhs == rhs);
}

bool operator==(const Node& lhs, const int& rhs) {
    return lhs.num == rhs;
}

bool operator==(const int& lhs, const Node& rhs) {
    return rhs == lhs;
}

bool operator<(const Node& lhs, const Node& rhs) {
    return lhs.num < rhs.num;
}

bool operator>(const Node& lhs, const Node& rhs) {
    return lhs.num > rhs.num;
}

/// Represents a directed edge in a graph with a capacity, flow, from node, and to node
class Edge {
public:
    int capacity, flow;
    Node& from;
    Node& to;
    
    Edge(int fl, int c, Node& f, Node& t): from(f), to(t) {
        capacity = c;
        flow = fl;
        from = f;
        to = t;
    }
    
    /*
    Edge(int c, int f, int t): from(f), to(t) {
        capacity = c;
        flow = 0;
    }
    */
    
    Edge(int c, Node& f, Node& t): from(f), to(t)  {
        capacity = c;
        flow = 0;
        from = f;
        to = t;
    }
};


/// The push relabel algorithm
class PushRelabel {
private:
    map<int, Node> nodes;
    map<Node, vector<Edge>> edges;
    
    /// Check if there is a node that is experencing an overflow, returns -1 if not
    int checkOverflow(Node& s, Node& t) {
        for (auto nn = nodes.begin(); nn != nodes.end(); ++nn) {
            //cout << "Name: " << nn->second.num << "Curr ef: " << nn->second.ef << endl;
            if (nn->second.ef > 0 && nn->second != s && nn->second != t)
                return nn->first;
        }
        /*
        auto nn = std::find(nodes.begin(), nodes.end(), [](const auto& n){return n.second.ef > 0;});
        if (nn != nodes.end())
            return nn->first;
         */
        return -1;
    }
    
public:
    PushRelabel() {
    }
    
    /// Adds a new node to the graph
    void addNode(int n) {
        if (nodes.contains(n)) {
            throw std::invalid_argument("Cannot have two nodes with the same name");
        }
        
        Node newNode = Node(n);
        nodes.insert(std::pair<int, Node>(n, newNode));
        //nodes[n] = newNode;
    }
    
    /// Adds a new edge between the given two nodes
    void addEdge(int f, int d, int c) {
        if (!nodes.contains(f)) {
            addNode(f);
        }
        if (!nodes.contains(d)) {
            addNode(d);
        }
        
        Edge newEdge = Edge(c, nodes[f], nodes[d]);
        if (edges.find(nodes[f]) != edges.end()) {
            edges[nodes[f]].push_back(newEdge);
        } else {
            vector<Edge> ev;
            ev.push_back(newEdge);
            edges.insert(std::pair<Node, vector<Edge>>(nodes[f], ev));
        }
    }
    
    /// Sets the preflow for the graph
    void preflow(Node& s) {
        for (int i = 0; i < edges[s].size(); ++i) {
            //edges[0].from.height = static_cast<int>(edges.size());
            // Set the edge's flow to it's capacity
            edges[s][i].flow = edges[s][i].capacity;
            //cout << "Flow: " << edges[s][i].flow << endl;
            // Set the to node's overflow to the flow of this edge
            edges[s][i].to.ef = edges[s][i].flow;
            //cout << "Ef: " << edges[s][i].to.ef << endl;
            // Add reverse edge
            edges[edges[s][i].to].push_back(Edge(-edges[s][i].flow, 0, edges[s][i].to, s));
            continue;
        }
    }
    
    /// Pushes excess flow to a new node, or returns false if not possible
    bool push(Node& n) {
        // For each edge coming from this node
        for (int i = 0; i < edges[n].size(); ++i) {
            // If flow is already at capacity, it cannot be increased, so continue
            if (edges[n][i].flow == edges[n][i].capacity)
                continue;
            
            // If from height is greater than to height, then it can be increased
            if (n.height > edges[n][i].to.height) {
                Node& dn = edges[n][i].to;
                // Calculate flow to push
                int pushFlow = std::min(edges[n][i].capacity - edges[n][i].flow, n.ef);
                // Add new flow to existing flow
                edges[n][i].flow += pushFlow;
                // Remove the pushed flow from this node's overflow
                n.ef -= pushFlow;
                // Add to to node's overflow the push flow
                dn.ef += pushFlow;
                
                // Update/Add reverse edge to graph
                for (int j = 0; j < edges[dn].size(); ++j) {
                    // If there is already a maching reverse edge, update it
                    if (edges[dn][j].to == n) {
                        edges[dn][j].flow -= pushFlow;
                        return true;
                    }
                }
                
                // If there is no reverse edge already, create a new one
                edges[dn].push_back(Edge(-pushFlow, 0, dn, n));
                return true;
            }
        }
        
        // If there is no avalable edge to push the excess to, return false
        return false;
    }
    
    /// Relabels n based on the adjacent nodes' height
    void relabel(Node& n) {
        n.height = n.height + 1;
        // For each edge coming from n
        for (int i = 0; i < edges[n].size(); ++i) {
            // If flow is already at capacity, then no relabeling is done
            if (edges[n][i].flow == edges[n][i].capacity)
                continue;
            
            // Relabel based on min height
            if (edges[n][i].to.height < n.height - 1)
                n.height = edges[n][i].to.height + 1;
        }
    }
    
    /// Gets the max flow of the graph
    int getMaxFlow(int si, int ti) {
        Node& s = nodes[si];
        Node& t = nodes[ti];
        
        // Reset/ensure correct starting values
        for (int i = 0; i < edges.size(); ++i) {
            for (int j = 0; j < edges[i].size(); ++j) {
                //cout << i << " " << j << " " << edges[i][j].to.num << " " << edges[i][j].capacity << endl;
                edges[i][j].from.height = 0;
                edges[i][j].to.height = 0;
                edges[i][j].from.ef = 0;
                edges[i][j].to.ef = 0;
                edges[i][j].flow = 0;
            }
        }
        
        s.height = static_cast<int>(nodes.size());
        //cout << nodes[si].height << endl;
        preflow(s);
        
        while (true) {
            //cout << t.ef << endl;
            int overInt = checkOverflow(s, t);
            if (overInt == -1) {
                break;
            }
            Node& n = nodes[overInt];
            if (!push(n))
                relabel(n);
        }
        
        return t.ef;
    }
    
    /// Gets the max flow of the graph from 0 to the last node in the list (highest number)
    int getMaxFlow() {
        return getMaxFlow(0, std::prev(nodes.end())->second.num);
    }
    
    /// Displays the max flow and the resulting graph
    friend std::ostream& operator<<(std::ostream& out, PushRelabel& pr) {
        out << "------------------------------" << endl;
        cout << "Max flow: " << pr.getMaxFlow() << endl;
        for (auto nn = pr.nodes.begin(); nn != pr.nodes.end(); ++nn) {
            out << "---------------" << endl;
            out << "-- " << nn->second.num << " --" << endl << "h=" << nn->second.height << ", ef=" << nn->second.ef << endl;
            for (auto e = pr.edges.at(nn->second).begin(); e != pr.edges.at(nn->second).end(); ++e) {
                out << nn->second.num << "----->" << e->to.num << ":" << endl
                << "C=" << e->capacity << ", F=" << e->flow << endl << endl;
            }
        }
        out << "------------------------------" << endl;
        return out;
    }
};

int main(int argc, const char * argv[]) {
    PushRelabel pr = PushRelabel();
    
    string fileName = "input";
    ifstream inputFile;
    inputFile.open(fileName + ".txt");
    
    if (!inputFile) {
        cout << "Cannot find file" << endl;
        return -1;
    }
    
    string line;
    while(getline(inputFile, line)) {
        std::istringstream el(line);
        string fs;
        getline(el, fs, ',');
        int from = stoi(fs);
        getline(el, fs, ',');
        int to = stoi(fs);
        getline(el, fs, ',');
        int c = stoi(fs);
        //cout << line << endl;
        pr.addEdge(from, to, c);
    }
    cout << pr << endl;
    
    /*
     Default example:
     0, 1, 16
     0, 2, 13
     2, 1, 4
     1, 2, 10
     1, 3, 12
     3, 2, 9
     2, 4, 14
     4, 3, 7
     3, 5,20
     4, 5, 4
     
     Another example:
     0, 1, 10
     0, 2, 12
     1, 3, 15
     2, 1, 5
     2, 4, 6
     3, 4, 8
     3, 5, 3
     4, 5, 17
     */
    return 0;
}
