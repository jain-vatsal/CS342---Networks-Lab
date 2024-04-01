#include <iostream>
#include <queue>
#include <random>
#include <chrono>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <climits>
using namespace std;
#define SIMULATION_TIME 200000.0
double gt = 0;
class Passenger
{
public:
    int inter_arrival_time;
    int global_arrival_time; // Time it takes for the passenger to arrive at the checkpoint (in milliseconds).
    int processing_time;     // Time it takes for the security scanner to process the passenger (in milliseconds).
    Passenger(double arrival_rate, double service_rate)
    {
        std::default_random_engine generator(std::random_device{}());
        std::exponential_distribution<double> arrival_distribution(arrival_rate);
        std::exponential_distribution<double> processing_distribution(service_rate);

        inter_arrival_time = static_cast<int>(arrival_distribution(generator) * 1000); // Convert to milliseconds.
        gt += inter_arrival_time;
        global_arrival_time = gt;
        processing_time = static_cast<int>(processing_distribution(generator) * 1000); // Convert to milliseconds.
        std::cout << "Created passenger has values : " << inter_arrival_time << " " << processing_time << "\n";
    }
};

int passenger_cnt = 0;
int queue_length = 0;
double time_prev = 0;
double total_service = 0;
int single_buffer_size = 5;
double arrival_rate = 3; // λ
double total_waiting_time = 0.0;
double service_rate = 6; // μ
double start_proc_time = 0.0;
double total_queue_length = 0;
int K = 10;
int m = 4; // No. of scanners
vector<double> start_proc_vec(m, 0);
// int buffer_size;     // K
// int num_scanners;    // Number of security scanners
// int buffer_m;        // Buffer size in front of m security scanners
int total_passengers = 0;
std::queue<Passenger> security_line;
vector<queue<Passenger>> security_line_vec(m);
// std::vector<std::queue<Passenger>> multi_scanner_lines;
// std::vector<std::queue<Passenger>> buffered_multi_scanner_lines;
bool flag = false;
std::mutex mtx;
vector<mutex> mtx_vec(m);

// Function to simulate passenger arrivals.
void simulateArrivals_singleServer_infiniteBuffers()
{
    while (true)
    {
        Passenger passenger(arrival_rate, service_rate);
        if (gt > SIMULATION_TIME)
        {
            flag = true;
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(passenger.inter_arrival_time));

        mtx.lock();
        security_line.push(passenger);
        total_queue_length += (passenger.global_arrival_time - time_prev) * queue_length;
        cout << "tot : " << total_queue_length << endl;
        queue_length++;
        time_prev = passenger.global_arrival_time;
        start_proc_time = std::max(start_proc_time, gt);

        std::cout << "arrival_time " << gt << "\n";
        mtx.unlock();
        // cv.notify_all();
    }
}

void simulateArrivals_singleServer_finiteBuffer()
{
    while (true)
    {
        Passenger passenger(arrival_rate, service_rate);

        if (gt > SIMULATION_TIME)
        {
            flag = true;
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(passenger.inter_arrival_time));

        if (queue_length < single_buffer_size)
        {
            mtx.lock();
            security_line.push(passenger);
            total_queue_length += (passenger.global_arrival_time - time_prev) * queue_length;
            cout << "tot : " << total_queue_length << endl;
            queue_length++;
            time_prev = passenger.global_arrival_time;
            start_proc_time = std::max(start_proc_time, gt);

            std::cout << "arrival_time " << gt << "\n";
            mtx.unlock();
        }
        // cv.notify_all();
    }
}
void simulateArrivals_multiServer_infiniteBuffer()
{
    while (true)
    {
        Passenger passenger(arrival_rate, service_rate);
        if (gt > SIMULATION_TIME)
        {
            flag = true;
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(passenger.inter_arrival_time));
        for (int i = 0; i < m; i++)
        {
            mtx_vec[i].lock();
        }
        int minq = -1;
        int minlen = INT_MAX;
        for (int i = 0; i < m; i++)
        {
            if (security_line_vec[i].size() < minlen)
            {
                minlen = security_line_vec[i].size();
                minq = i;
            }
        }
        for (int i = 0; i < m; i++)
        {
            if (i == minq)
                continue;
            mtx_vec[i].unlock();
        }
        security_line_vec[minq].push(passenger);
        // mtx.lock();
        time_prev = passenger.global_arrival_time;
        start_proc_vec[minq] = max(start_proc_vec[minq], gt);
        // start_proc_time = std::max(start_proc_time, gt);

        std::cout << "arrival_time " << gt << "\n";
        mtx_vec[minq].unlock();
        // cv.notify_all();
    }
}
void simulateArrivals_multiServer_finiteBuffer()
{
    while (true)
    {
        Passenger passenger(arrival_rate, service_rate);
        if (gt > SIMULATION_TIME)
        {
            flag = true;
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(passenger.inter_arrival_time));
        for (int i = 0; i < m; i++)
        {
            mtx_vec[i].lock();
        }
        int minq = -1;
        int minlen = INT_MAX;
        for (int i = 0; i < m; i++)
        {
            if (security_line_vec[i].size() < minlen)
            {
                minlen = security_line_vec[i].size();
                minq = i;
            }
        }
        for (int i = 0; i < m; i++)
        {
            if (i == minq)
                continue;
            mtx_vec[i].unlock();
        }
        // bool flag = false;
        if (security_line_vec[minq].size() < K)
        {
            security_line_vec[minq].push(passenger);
            // mtx.lock();
            time_prev = passenger.global_arrival_time;
            start_proc_vec[minq] = max(start_proc_vec[minq], gt);
            // start_proc_time = std::max(start_proc_time, gt);

            std::cout << "arrival_time " << gt << "\n";
        }
        mtx_vec[minq].unlock();
        // cv.notify_all();
    }
}
void simulateProcessing_multiServer_infiniteBuffers(int index)
{
    while (true)
    {
        if (flag)
            break;
        // std::unique_lock<std::mutex> lock(mtx);
        mtx_vec[index].lock();
        int proc_time = 0;
        if (!security_line_vec[index].empty())
        {
            Passenger passenger = security_line_vec[index].front();
            proc_time = passenger.processing_time;
            std::cout << "start_proc_time : " << start_proc_time << "\n";
            security_line_vec[index].pop();
            // time_prev = start_proc_time + passenger.processing_time;
            total_waiting_time += start_proc_vec[index] - passenger.global_arrival_time;
            std::cout << "wait: " << total_waiting_time << "\n";
            start_proc_vec[index] += proc_time;
            passenger_cnt++;
            total_service += proc_time;
        }

        mtx_vec[index].unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(proc_time));
        // Process the passenger.
    }
}
// Function to process passengers.
void simulateProcessing_singleServer_infiniteBuffers()
{
    while (true)
    {
        if (flag)
            break;
        // std::unique_lock<std::mutex> lock(mtx);
        mtx.lock();
        int proc_time = 0;
        // cv.wait(lock, []
        //         { return !security_line.empty(); });
        // if (security_line.front())
        if (!security_line.empty())
        {
            Passenger passenger = security_line.front();
            proc_time = passenger.processing_time;
            std::cout << "start_proc_time : " << start_proc_time << "\n";
            security_line.pop();
            total_queue_length += queue_length * (start_proc_time + passenger.processing_time - time_prev);
            cout << "tot : " << total_queue_length << endl;
            queue_length--;
            time_prev = start_proc_time + passenger.processing_time;
            total_waiting_time += start_proc_time - passenger.global_arrival_time;
            std::cout << "wait: " << total_waiting_time << "\n";
            start_proc_time += proc_time;
            passenger_cnt++;
            total_service += proc_time;
        }

        mtx.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(proc_time));
        // Process the passenger.
    }
}

// Function to process passengers.
void simulateProcessing_singleServer_finiteBuffer()
{
    while (true)
    {
        if (flag)
            break;
        // std::unique_lock<std::mutex> lock(mtx);
        mtx.lock();
        int proc_time = 0;
        // cv.wait(lock, []
        //         { return !security_line.empty(); });
        // if (security_line.front())
        if (!security_line.empty())
        {
            Passenger passenger = security_line.front();
            proc_time = passenger.processing_time;
            std::cout << "start_proc_time : " << start_proc_time << "\n";
            security_line.pop();
            total_queue_length += queue_length * (start_proc_time + passenger.processing_time - time_prev);
            cout << "tot : " << total_queue_length << endl;
            queue_length--;
            time_prev = start_proc_time + passenger.processing_time;
            total_waiting_time += start_proc_time - passenger.global_arrival_time;
            std::cout << "wait: " << total_waiting_time << "\n";
            start_proc_time += proc_time;
            passenger_cnt++;
            total_service += proc_time;
        }

        mtx.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(proc_time));
        // Process the passenger.
    }
}

void initializeVariables()
{
    gt = 0;
    passenger_cnt = 0;
    queue_length = 0;
    time_prev = 0;
    total_service = 0;
    single_buffer_size = 5;
    arrival_rate = 3; // λ
    total_waiting_time = 0.0;
    service_rate = 6; // μ
    start_proc_time = 0.0;
    total_queue_length = 0;
    // buffer_size;     // K
    // num_scanners;    // Number of security scanners
    // buffer_m;        // Buffer size in front of m security scanners
    total_passengers = 0;
}

void printSimulation_singleServer()
{
    std::cout << "\n\nTheoretical average waiting time " << 1000 * arrival_rate / (service_rate * (service_rate - arrival_rate));
    std::cout << "\nEmpirical average waiting time " << total_waiting_time / passenger_cnt << " \n";

    std::cout << "Theoretical average queue length " << (arrival_rate * arrival_rate) / (service_rate * (service_rate - arrival_rate));
    std::cout << "\nEmpirical average queue length " << total_waiting_time / SIMULATION_TIME << "\n";

    std::cout << "Theoretical Service Utilization " << arrival_rate / service_rate << "\n";
    std::cout << "Empirical Service Utilization " << total_service / SIMULATION_TIME << "\n";
}
void printSimulation_multiServer()
{
    // std::cout << "\n\nTheoretical average waiting time " << 1000 * arrival_rate / (m * service_rate * (m * service_rate - arrival_rate));
    std::cout << "\nEmpirical average waiting time " << total_waiting_time / passenger_cnt << " \n";

    // std::cout << "Theoretical average queue length " << (arrival_rate * arrival_rate) / (m * service_rate * (m * service_rate - arrival_rate));
    std::cout << "\nEmpirical average queue length " << total_waiting_time / SIMULATION_TIME << "\n";

    // std::cout << "Theoretical Service Utilization " << arrival_rate / (m * service_rate) << "\n";
    std::cout << "Empirical Service Utilization " << total_service / SIMULATION_TIME << "\n";
}
void simulate_singleServer_infiniteBuffers()
{
    initializeVariables();

    // Create arrival and processing threads.
    std::thread arrivals(simulateArrivals_singleServer_infiniteBuffers);
    std::thread processing_thread(simulateProcessing_singleServer_infiniteBuffers);

    // Wait for the threads to finish.
    arrivals.join();
    processing_thread.join();

    printSimulation_singleServer();
}

void simulate_singleServer_finiteBuffer()
{
    initializeVariables();

    std::thread arrivals(simulateArrivals_singleServer_finiteBuffer);
    std::thread processing_thread(simulateProcessing_singleServer_finiteBuffer);

    // Wait for the threads to finish.
    arrivals.join();
    processing_thread.join();

    printSimulation_singleServer();
}
void simulate_multiServer_infiniteBuffer()
{
    initializeVariables();
    std::thread arrivals(simulateArrivals_multiServer_infiniteBuffer);
    vector<thread> scanners;
    for (int i = 0; i < m; i++)
    {
        thread t1(simulateProcessing_multiServer_infiniteBuffers, i);
        scanners.push_back(move(t1));
    }

    arrivals.join();
    for (int i = 0; i < m; i++)
    {
        scanners[i].join();
    }

    printSimulation_multiServer();
}
int main()
{
    // Set user-defined parameters.
    // Initialize random number generators for passenger arrivals and processing times.

    simulate_singleServer_infiniteBuffers();

    // simulate_multiServer_infiniteBuffer();

    return 0;
}