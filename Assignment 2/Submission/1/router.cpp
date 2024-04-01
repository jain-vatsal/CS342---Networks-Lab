#include<bits/stdc++.h>
using namespace std;

class routers
{
    public:
    map<int, int> routing_table;   // routing table information for router
    int router_id;  // routing id of router
    map<routers *, int> neighbours;  // map which stores neighbouring router mapped to its cost


    //
    void add_neighbor(routers *neighbour, int c)
    {
        neighbours[neighbour] = c;
    }
    
    
    routers(int id) // constructor to initialize router id for this router
    {
        router_id = id;
    }

//   finding minimum cost to reach all other routers.
    void update_routing_table() 
    {
        map<int, int> curr_distance;  // to store minimum distance from this router to other nodes
        
        priority_queue<pair<int, routers *>> pq;  // use priority queue to take cost in increasing order, which distance is less add it to queue
        // pair description => ( -cost , router link) 
        
        map<int, int> parent;   // to store parent of any node which is in path of current node to destination node
      
      
        curr_distance[router_id] = 0;  // current node distance is 0
        
        pq.push({0, this}); 

        
        parent[router_id] = router_id;
        map<int, int> visited_nodes;   
        
        
        while (!pq.empty())
        {
            auto current_node = pq.top();
        
            pq.pop();
            
            if (visited_nodes[current_node.second->router_id])
                continue;

            visited_nodes[current_node.second->router_id] = 1;
            
            auto node = current_node.second;
           
           
            auto node_id = current_node.second->router_id;
            
           
            for (auto &it : node->neighbours) //checking for all neghbours of currently taken router in list with minimum distance
            {
                if (curr_distance.count(it.first->router_id)) // if node is already discovered and has early cost value then check if we can minimize it by including the link between node and node->neighbour
                {
                    if (curr_distance[it.first->router_id] > curr_distance[node_id] + it.second)
                    {
                        // this section is execute to include this link and update minimum cost to reach node->neighbour and also its parent
                        pq.push({-curr_distance[it.first->router_id], it.first});
                        parent[it.first->router_id] = node_id;
                        curr_distance[it.first->router_id] = curr_distance[node_id] + it.second;
                    }
                }
                else // if node is not discovered then include this link and add to priority queue
                {

                    pq.push({-curr_distance[it.first->router_id], it.first});
                    parent[it.first->router_id] = node_id;
                    curr_distance[it.first->router_id] = curr_distance[node_id] + it.second;
                }
            }
        }

        for (auto &it : curr_distance)  // for all nodes just make a while to trace the path to its parent to know which neighbouring node of current node leads to particular destination optimally
        {
            int current_node = it.first;
            int n = current_node;
            while (parent[current_node] != router_id)
            {
                current_node = parent[current_node];
            }
            routing_table[n] = current_node; // storing answer in routing table

        }
    }



    void print_routing_table() // to print routing table for a node
    {
        for (auto &it : routing_table)
        {
            cout << "Destination: ";
            cout << it.first << " ";
            cout<<"-> ";
            if (it.first == router_id)
            {
                //
                cout << "Already at destination...\n";
            }
            else
            {
                //
                cout << "Adjacent node to which data is sent: ";
                cout << it.second << endl;
            }
        }
    }
};




int main()
{
    
    // considering all routers are reacheable from one another 

    cout << "Enter number of nodes = ";
    int n;

    cin >> n;
    cout<<endl;
    vector<routers *> nodes(n + 1);

    for (int i = 1;  i  <=  n; i++)
    {
        nodes[i] = new routers(i);
    }

    cout << "Enter number of edges = ";
    int m;
    cin >> m;
    cout<<endl;


    cout << "Enter edge info (x y z, nodes x and y are connected at cost c) :- "<<endl;
    
    
    for (int i = 1; i <= m; i++)
    {
        int x, y, c;
        cin >> x >> y >> c;
        nodes[x]->add_neighbor(nodes[y], c);
        nodes[y]->add_neighbor(nodes[x], c);
    }

    // compute forwarding table of all node.

    for (int i = 1; i <= n; i++)
    {
        nodes[i]->update_routing_table();
    }

    while (1)
    {
        cout << "Enter node_id of routing table to be printed [to exit enter -1]: \n";
        int x;
        cin >> x;
        if (x == -1)
        {
            
            break;
        }
        else if(x<=0 || x > n )
        {
            cout<<"Invalid Input"<<endl;
        }
        else
        {
            cout << "Routing table of " << x << " :- \n";
            nodes[x]->print_routing_table();
        }
    }
}