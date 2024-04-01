#include <bits/stdc++.h>
#include <vector>
using namespace std;
typedef long long li;
#define vi vector<li>
#define vvi vector<vi>
#define pii pair<li, li>
#define mmp map<li, li> mp
#define rep(i, a, b) for (li i = a; i < b; i++)

int num;                // global variable for the number of websites
class HTTPRequestClass; // h

class Website
{
public:
    int website_id, owner_id, bandwidth, processing_power, total_weight;
    queue<pair<int, int>> queue;

    Website(int id, int owner, int bw, int power)
    {
        website_id = id;
        owner_id = owner;
        bandwidth = bw;
        processing_power = power;
    }

    void add(int request_id, int processing_time)
    {
        auto p = make_pair(request_id, processing_time);
        queue.push(p);
    }
};

vector<Website *> websites; // storing all the websites in this vector

class HTTPRequestClass
{

public:
    int request_id, website_id, processing_time;

    HTTPRequestClass(int x, int y, int z)
    {
        request_id = x;
        website_id = y;
        processing_time = z;
    }

    void add()
    {
        for (int i = 0; i < websites.size(); i++)
        {
            if (websites[i]->website_id == this->website_id)
            {
                websites[i]->add(request_id, processing_time);
                break;
            }
        }
    }
};

class LoadBalancer
{
public:
    void add_website(int website_id, int owner_id, int bandwidth, int processing_power)
    {
        Website *w = new Website(website_id, owner_id, bandwidth, processing_power);
        websites.push_back(w);
    }

    void enqueue_request(HTTPRequestClass r)
    {
        r.add();
    }
    void dequeue_request(int id)
    {
        for (int i = 0; i < num; i++)
        {
            if (websites[i]->website_id == id)
            {
                websites[i]->queue.pop();
                break;
            }
        }
    }
    void WFQ_Scheduling()
    {
        map<int, double> weights;
        for (int i = 0; i < num; i++)
            weights[i] = (websites[i]->total_weight);
        int cnt = 0, val;
        for (auto &it : weights)
        {
            if (cnt == 0)
            {
                val = it.second;
                it.second = 1;
                cnt++;
            }
            else
                it.second = double(it.second + val - 1.0) / (1.0 * val);
        }
        // Calculating Virtual Finish times
        multiset<pair<double, pair<pair<li, li>, li>>> s;
        for (int i = 0; i < num; i++)
        {
            double time = 0;
            while (!websites[i]->queue.empty())
            {
                auto curr = websites[i]->queue.front();
                websites[i]->queue.pop();
                time += double(curr.second) / weights[i];
                s.insert({time, {{curr.first, websites[i]->website_id}, curr.second}});
            }
        }
        vector<pair<pair<li, li>, li>> ord;
        while (!s.empty())
        {
            auto curr = *(s.begin());
            s.erase(s.begin());
            ord.push_back(curr.second);
        }
        li actual_time = 0;
        for (auto it : ord)
        {
            actual_time += it.second;
            dequeue_request(it.first.second);
            cout << "Request no " << it.first.first << " with required website " << it.first.second << " dequeued at " << actual_time << endl;
        }
    }
};

LoadBalancer l;

int main()
{
    cout << "Enter the number of Websites: \n";
    cin >> num;

    cout << "Enter the websites in the order :- website_id, owner_id, bandwidth, processing power \n";
    for (int i = 0; i < num; i++)
    {
        int web_id, owner_id, bandwidth, processing_power;
        cin >> web_id >> owner_id >> bandwidth >> processing_power;
        l.add_website(web_id, owner_id, bandwidth, processing_power);
    }
    cout << "Starting the WFQ alogirthm...\n";
    cout << "Enter the number of HTTP requests : \n";
    int sz;
    cin >> sz;
    cout << "Enter input HTTP requests(request_id, website_id, processing_time) :\n";
    for (int i = 0; i < sz; i++)
    {
        int x, y, z;
        cin >> x >> y >> z;
        HTTPRequestClass r(x, y, z);
        l.enqueue_request(r);
    }
    l.WFQ_Scheduling();
    cout << "Exiting...\n";
}
