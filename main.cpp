#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>  // For formatting table
#include <numeric>  // For std::gcd
#include <algorithm>  // For std::sort, std::lcm

using namespace std;

class Instance {
public:
    string name;
    float deadline;
    float executionTime;
    float nextStartTime;
    int priority;  // Priority for DMA scheduling

    Instance(string name, float deadline, float executionTime, float nextStartTime, int priority = 0)
        : name(name), deadline(deadline), executionTime(executionTime), nextStartTime(nextStartTime), priority(priority) {}
};



class Process {
private:
    string name;
    float period;
    float executionTime;

public:
    vector<Instance> instances;  // To store instances

    Process(string name, float period, float executionTime)
        : name(name), period(period), executionTime(executionTime) {}

    float getPeriod() const { return period; }
    float getExecutionTime() const { return executionTime; }
    string getName() const { return name; }

    void addInstance(const Instance& instance) {
        instances.push_back(instance);
    }
};

void createProcessInstances(vector<Process>& processes, int hyperperiod) {
    for (size_t i = 0; i < processes.size(); ++i) {
        int count = hyperperiod / processes[i].getPeriod();
        for (int j = 1; j <= count; ++j) {
            string instanceName = processes[i].getName() + "-" + to_string(j);
            float deadline = j * processes[i].getPeriod();
            float nextStartTime = (j - 1) * processes[i].getPeriod();
            int priority = static_cast<int>(processes.size() - i);  // Higher priority for earlier processes
            processes[i].addInstance(Instance(instanceName, deadline, processes[i].getExecutionTime(), nextStartTime, priority));
        }
    }
}

int gcd(int a, int b) {
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}
int lcm(int a, int b) {
    return abs(a * b) / gcd(a, b);
}

int findHyperperiod(const vector<Process>& processes) {
    int hyperperiod = processes[0].getPeriod();
    for (size_t i = 1; i < processes.size(); ++i) {
        hyperperiod = lcm(hyperperiod, static_cast<int>(processes[i].getPeriod()));
    }
    return hyperperiod;
}

void printConsolidatedTable(const vector<pair<string, pair<int, int>>>& timeline) {
    cout << "\nConsolidated Schedule Table:\n";
    cout << left << setw(15) << "Process/Idle" << setw(10) << "Start" << "Finish" << endl;
    cout << string(35, '-') << endl;

    if (timeline.empty()) return;

    string currentName = timeline[0].first;
    int startTime = timeline[0].second.first;
    int endTime = timeline[0].second.second;

    for (size_t i = 1; i < timeline.size(); ++i) {
        if (timeline[i].first == currentName) {
            // Extend the current entry
            endTime = timeline[i].second.second;
        } else {
            // Print the consolidated entry
            cout << left << setw(15) << currentName << setw(10) << startTime << endTime << endl;

            // Start a new entry
            currentName = timeline[i].first;
            startTime = timeline[i].second.first;
            endTime = timeline[i].second.second;
        }
    }

    // Print the last entry
    cout << left << setw(15) << currentName << setw(10) << startTime << endTime << endl;
}

vector<pair<string, pair<int, int>>> checkMissedDeadlines(
    const vector<pair<string, pair<int, int>>>& timeline,
    const vector<Instance>& instances) {
    for (const auto& instance : instances) {
        int completionTime = -1;

        // Find the last execution time for this instance
        for (const auto& entry : timeline) {
            if (entry.first == instance.name) {
                completionTime = max(completionTime, entry.second.second);
            }
        }

        // Check if the instance misses its deadline
        if (completionTime > instance.deadline) {
            cout << instance.name << " missed its deadline. Scheduling failed.\n";
            return {};  // Return an empty timeline to indicate failure
        }
    }

    return timeline;  // Return the original timeline if no deadlines were missed
}


vector<pair<string, pair<int, int>>> rateMonotonicScheduling(const vector<Process>& processes, int hyperperiod) {
    vector<Instance> schedule;
    for (const auto& process : processes) {
        for (const auto& instance : process.instances) {
            schedule.push_back(instance);
        }
    }

    sort(schedule.begin(), schedule.end(), [](const Instance& a, const Instance& b) {
        return a.nextStartTime < b.nextStartTime;  // Sort by period (static priority)
    });

    vector<pair<string, pair<int, int>>> timeline;
    int currentTime = 0;

    while (currentTime < hyperperiod) {
        bool executed = false;
        for (auto& instance : schedule) {
            if (instance.executionTime > 0 && currentTime >= instance.nextStartTime) {
                int start = currentTime;
                instance.executionTime--;
                executed = true;
                timeline.push_back({instance.name, {start, start + 1}});
                currentTime++;
                break;
            }
        }
        if (!executed) {
            timeline.push_back({"Idle", {currentTime, currentTime + 1}});
            currentTime++;
        }
    }

    // Check for missed deadlines
    return checkMissedDeadlines(timeline, schedule);
}

vector<pair<string, pair<int, int>>> dmaScheduling(const vector<Process>& processes, int hyperperiod) {
    vector<Instance> schedule;

    // Flatten all instances into a single vector for scheduling
    for (const auto& process : processes) {
        for (const auto& instance : process.instances) {
            schedule.push_back(instance);
        }
    }

    // Sort instances by static deadlines (shorter deadlines = higher priority)
    sort(schedule.begin(), schedule.end(), [](const Instance& a, const Instance& b) {
        return a.deadline < b.deadline;  // Static priority based on deadline
    });

    vector<pair<string, pair<int, int>>> timeline;
    int currentTime = 0;

    while (currentTime < hyperperiod) {
        bool executed = false;
        for (auto& instance : schedule) {
            if (instance.executionTime > 0 && currentTime >= instance.nextStartTime) {
                // Execute the instance with the highest static priority
                int start = currentTime;
                instance.executionTime--;
                executed = true;
                timeline.push_back({instance.name, {start, start + 1}});
                currentTime++;
                break;
            }
        }
        if (!executed) {
            // If no tasks are ready, record idle time
            timeline.push_back({"Idle", {currentTime, currentTime + 1}});
            currentTime++;
        }
    }
    
    // Check for missed deadlines
    return checkMissedDeadlines(timeline, schedule);
}

vector<pair<string, pair<int, int>>> edfScheduling(const vector<Process>& processes, int hyperperiod) {
    vector<Instance> schedule;
    for (const auto& process : processes) {
        for (const auto& instance : process.instances) {
            schedule.push_back(instance);
        }
    }

    sort(schedule.begin(), schedule.end(), [](const Instance& a, const Instance& b) {
        return a.deadline < b.deadline;  // Sort by deadline (dynamic priority)
    });

    vector<pair<string, pair<int, int>>> timeline;
    int currentTime = 0;

    while (currentTime < hyperperiod) {
        bool executed = false;
        for (auto& instance : schedule) {
            if (instance.executionTime > 0 && currentTime >= instance.nextStartTime) {
                int start = currentTime;
                instance.executionTime--;
                executed = true;
                timeline.push_back({instance.name, {start, start + 1}});
                currentTime++;
                break;
            }
        }
        if (!executed) {
            timeline.push_back({"Idle", {currentTime, currentTime + 1}});
            currentTime++;
        }
    }

    // Check for missed deadlines
    return checkMissedDeadlines(timeline, schedule);
}

vector<pair<string, pair<int, int>>> lstScheduling(const vector<Process>& processes, int hyperperiod) {
    vector<Instance> schedule;

    // Flatten all instances into a single vector for scheduling
    for (const auto& process : processes) {
        for (const auto& instance : process.instances) {
            schedule.push_back(instance);
        }
    }

    vector<pair<string, pair<int, int>>> timeline;
    int currentTime = 0;

    while (currentTime < hyperperiod) {
        // Sort instances dynamically based on slack time
        sort(schedule.begin(), schedule.end(), [currentTime](const Instance& a, const Instance& b) {
            int slackA = a.deadline - (currentTime + a.executionTime);
            int slackB = b.deadline - (currentTime + b.executionTime);
            return slackA < slackB;  // Least slack time gets higher priority
        });

        bool executed = false;
        for (auto& instance : schedule) {
            if (instance.executionTime > 0 && currentTime >= instance.nextStartTime) {
                // Execute the instance with the least slack time
                int start = currentTime;
                instance.executionTime--;
                executed = true;
                timeline.push_back({instance.name, {start, start + 1}});
                currentTime++;
                break;
            }
        }
        if (!executed) {
            // If no tasks are ready, record idle time
            timeline.push_back({"Idle", {currentTime, currentTime + 1}});
            currentTime++;
        }
    }

    // Check for missed deadlines
    return checkMissedDeadlines(timeline, schedule);
}

vector<Process> readProcessesFromFile(const string& filename) {
    vector<Process> processes;
    ifstream inputFile(filename);

    if (!inputFile.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return processes;
    }

    string line;
    while (getline(inputFile, line)) {
        size_t start = line.find('(');
        size_t end = line.find(')');
        if (start == string::npos || end == string::npos) {
            cerr << "Error: Invalid format in line: " << line << endl;
            continue;
        }

        string processName = line.substr(0, start);
        string parameters = line.substr(start + 1, end - start - 1);
        stringstream ss(parameters);
        float period, executionTime;

        if (ss >> period && ss.ignore() && ss >> executionTime) {
            processes.emplace_back(processName, period, executionTime);
        } else {
            cerr << "Error: Invalid parameters in line: " << line << endl;
        }
    }

    inputFile.close();
    return processes;
}

int main() {
    string filename = "processes.txt";

    // Step 1: Read processes from file
    vector<Process> processes = readProcessesFromFile(filename);

    if (processes.empty()) {
        cerr << "No processes loaded. Exiting...\n";
        return 1;
    }

    // Step 2: Find hyperperiod
    int hyperperiod = findHyperperiod(processes);
    cout << "Hyperperiod: " << hyperperiod << endl;

    // Step 3: Create process instances
    createProcessInstances(processes, hyperperiod);

    // Step 4: Apply Rate Monotonic Scheduling (RMS)
    cout << "\n=== Rate Monotonic Scheduling (RMS) ===\n";
    auto rmsTimeline = rateMonotonicScheduling(processes, hyperperiod);
    if (!rmsTimeline.empty()) {
        printConsolidatedTable(rmsTimeline);
    }

    // Step 5: Apply Deadline Monotonic Algorithm (DMA)
    cout << "\n=== Deadline Monotonic Algorithm (DMA) ===\n";
    auto dmaTimeline = dmaScheduling(processes, hyperperiod);
    if (!dmaTimeline.empty()) {
        printConsolidatedTable(dmaTimeline);
    }

    // Step 6: Apply Earliest Deadline First (EDF)
    cout << "\n=== Earliest Deadline First (EDF) ===\n";
    auto edfTimeline = edfScheduling(processes, hyperperiod);
    if (!edfTimeline.empty()) {
        printConsolidatedTable(edfTimeline);
    }

    // Step 7: Apply Least Slack Time (LST) Scheduling
    cout << "\n=== Least Slack Time (LST) Scheduling ===\n";
    auto lstTimeline = lstScheduling(processes, hyperperiod);
    if (!lstTimeline.empty()) {
        printConsolidatedTable(lstTimeline);
    }

    return 0;
}
