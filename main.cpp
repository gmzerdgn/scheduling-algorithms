#include <iostream>

using namespace std;

/*
TODO 
[x] cpp syntax/commenting
[ ] read from external file 
[ ] processes as objects (period, time) (and other input forms)
[ ] output in a nice way 
*/

class Process {
    private:
        float delay; 
        float period; 
        float executionTime;
        float deadline;
        string name;
    public:
        Process (string name, float period, float executionTime); //constructor with period and execution time only
        Process (string name, float period, float executionTime, float deadline);
};

Process::Process (string name, float period, float executionTime){
    name = name;
    period = period;
    executionTime = executionTime;
};

Process::Process (string name, float period, float executionTime, float deadline){
    name = name;
    period = period;
    executionTime = executionTime;
    deadline = deadline;
}


