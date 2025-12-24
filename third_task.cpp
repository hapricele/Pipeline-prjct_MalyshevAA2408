#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <limits>
#include <ctime>
#include <sstream>
#include <algorithm>
#include <map>
#include <queue>
using namespace std;

// Logger for proper file handling
class Logger {
    ofstream logFile;
    string getCurrentTime() {
        time_t now = time(nullptr);
        tm *timeinfo = localtime(&now);
        char buffer[20];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
        return string(buffer);
    }
public:
    Logger(const string &filename = "pipeline_log.txt") {
        logFile.open(filename, ios::app);
    }
    ~Logger() {
        if (logFile.is_open())
            logFile.close();
    }
    void log(const string &action) {
        if (logFile.is_open()) {
            logFile << "[" << getCurrentTime() << "] " << action << "\n";
            logFile.flush();
        }
    }
};

Logger g_logger;

// Data structures
struct Pipe {
    static int nextId;
    int id;
    string name;
    double length;
    int diameter;
    bool underRepair;
    bool inUse;
    
    Pipe() : id(++nextId), length(0), diameter(0), underRepair(false), inUse(false) {}
    
    double getLength() const { return length; }
    int getDiameter() const { return diameter; }
    bool isUnderRepair() const { return underRepair; }
    bool isInUse() const { return inUse; }
    void setRepairStatus(bool status) { underRepair = status; }
    void setInUse(bool status) { inUse = status; }
};
int Pipe::nextId = 0;

struct CompressorStation {
    static int nextId;
    int id;
    string name;
    int totalWorkshops;
    int workingWorkshops;
    int stationClass;
    
    CompressorStation() : id(++nextId), totalWorkshops(0), workingWorkshops(0), stationClass(0) {}
    
    double getUnusedPercent() const {
        return totalWorkshops == 0 ? 0 : (double)(totalWorkshops - workingWorkshops) / totalWorkshops * 100;
    }
    void adjustWorkshops(int delta) {
        workingWorkshops = max(0, min(totalWorkshops, workingWorkshops + delta));
    }
};
int CompressorStation::nextId = 0;

// Network graph structures
struct Edge {
    int pipeId;
    int toStationId;
    int diameter;
    
    Edge(int pid, int tsid, int d) : pipeId(pid), toStationId(tsid), diameter(d) {}
};

struct NetworkGraph {
    map<int, vector<Edge>> adjacencyList;
    map<int, int> inDegree;
    
    void addEdge(int fromStation, int toStation, int pipeId, int diameter) {
        adjacencyList[fromStation].push_back(Edge(pipeId, toStation, diameter));
        inDegree[toStation]++;
        if (inDegree.find(fromStation) == inDegree.end()) {
            inDegree[fromStation] = 0;
        }
    }
    
    vector<int> topologicalSort() {
        vector<int> result;
        map<int, int> tempInDegree = inDegree;
        queue<int> q;
        
        for (auto &p : tempInDegree) {
            if (p.second == 0) {
                q.push(p.first);
            }
        }
        
        while (!q.empty()) {
            int station = q.front();
            q.pop();
            result.push_back(station);
            
            if (adjacencyList.find(station) != adjacencyList.end()) {
                for (auto &edge : adjacencyList[station]) {
                    tempInDegree[edge.toStationId]--;
                    if (tempInDegree[edge.toStationId] == 0) {
                        q.push(edge.toStationId);
                    }
                }
            }
        }
        
        return result;
    }
    
    void displayGraph() {
        cout << "\n=== NETWORK GRAPH ===\n";
        for (auto &station : adjacencyList) {
            cout << "Station " << station.first << " -> ";
            for (auto &edge : station.second) {
                cout << "Station " << edge.toStationId << " (Pipe " << edge.pipeId << ", D:" << edge.diameter << "mm) ";
            }
            cout << "\n";
        }
    }
};

// Input validation
double readPositiveDouble(const string &prompt) {
    double value;
    cout << prompt;
    while (!(cin >> value) || value <= 0) {
        cout << "Invalid input. Enter positive number: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    return value;
}

int readPositiveInt(const string &prompt) {
    int value;
    cout << prompt;
    while (!(cin >> value) || value <= 0) {
        cout << "Invalid input. Enter positive number: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    return value;
}

int readInt(const string &prompt, int minVal, int maxVal) {
    int value;
    cout << prompt;
    while (!(cin >> value) || value < minVal || value > maxVal) {
        cout << "Invalid input. Enter number between " << minVal << " and " << maxVal << ": ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    return value;
}

string readString(const string &prompt) {
    cout << prompt;
    cin.ignore();
    string result;
    getline(cin, result);
    return result;
}

// Pipe operations
void addPipe(vector<Pipe> &pipes) {
    Pipe pipe;
    pipe.name = readString("Enter pipe name: ");
    pipe.length = readPositiveDouble("Enter pipe length (km): ");
    pipe.diameter = readPositiveInt("Enter pipe diameter (mm): ");
    pipes.push_back(pipe);
    cout << "Pipe added (ID: " << pipe.id << ")\n";
    g_logger.log("Added pipe - ID: " + to_string(pipe.id) + ", Name: " + pipe.name);
}

void displayPipe(const Pipe &pipe) {
    cout << "[ID:" << pipe.id << "] " << pipe.name
         << " | " << pipe.length << "km, D" << pipe.diameter << "mm"
         << " | " << (pipe.underRepair ? "REPAIR" : "OK")
         << " | " << (pipe.inUse ? "IN USE" : "AVAILABLE") << "\n";
}

void displayAllPipes(const vector<Pipe> &pipes) {
    if (pipes.empty()) {
        cout << "No pipes\n";
        return;
    }
    cout << "\n=== PIPES ===\n";
    for (const auto &p : pipes)
        displayPipe(p);
}

vector<Pipe*> searchPipesByDiameter(vector<Pipe> &pipes, int diameter) {
    vector<Pipe*> results;
    for (auto &p : pipes) {
        if (p.getDiameter() == diameter && !p.isUnderRepair() && !p.isInUse()) {
            results.push_back(&p);
        }
    }
    return results;
}

void displayStation(const CompressorStation &st) {
    cout << "[ID:" << st.id << "] " << st.name
         << " | " << st.workingWorkshops << "/" << st.totalWorkshops << " working"
         << " | Unused: " << (int)st.getUnusedPercent() << "%"
         << " | Class:" << st.stationClass << "\n";
}

void displayAllStations(const vector<CompressorStation> &stations) {
    if (stations.empty()) {
        cout << "No stations\n";
        return;
    }
    cout << "\n=== STATIONS ===\n";
    for (const auto &s : stations)
        displayStation(s);
}

// Network connection
void connectStations(vector<Pipe> &pipes, vector<CompressorStation> &stations, NetworkGraph &graph) {
    displayAllStations(stations);
    
    if (stations.empty()) {
        cout << "No stations available\n";
        return;
    }
    
    int fromId = readPositiveInt("Enter source station ID: ");
    int toId = readPositiveInt("Enter destination station ID: ");
    int requiredDiameter = readInt("Enter required diameter (500/700/1000/1400): ", 500, 1400);
    
    bool fromExists = false, toExists = false;
    for (const auto &s : stations) {
        if (s.id == fromId) fromExists = true;
        if (s.id == toId) toExists = true;
    }
    
    if (!fromExists || !toExists) {
        cout << "Invalid station ID\n";
        return;
    }
    
    auto availablePipes = searchPipesByDiameter(pipes, requiredDiameter);
    
    Pipe* selectedPipe = nullptr;
    
    if (!availablePipes.empty()) {
        cout << "Found available pipes:\n";
        for (size_t i = 0; i < availablePipes.size(); i++) {
            cout << (i+1) << ". Pipe ID: " << availablePipes[i]->id << ", Name: " << availablePipes[i]->name << "\n";
        }
        int choice = readInt("Select pipe (0=Create new): ", 0, (int)availablePipes.size());
        if (choice > 0) {
            selectedPipe = availablePipes[choice-1];
        }
    }
    
    if (!selectedPipe) {
        cout << "Creating new pipe...\n";
        Pipe newPipe;
        newPipe.name = "Auto_Pipe_" + to_string(newPipe.id);
        newPipe.length = 50.0;
        newPipe.diameter = requiredDiameter;
        pipes.push_back(newPipe);
        selectedPipe = &pipes.back();
        cout << "New pipe created (ID: " << selectedPipe->id << ")\n";
    }
    
    selectedPipe->setInUse(true);
    graph.addEdge(fromId, toId, selectedPipe->id, requiredDiameter);
    
    cout << "Connection established: Station " << fromId << " -> Station " << toId 
         << " via Pipe " << selectedPipe->id << "\n";
    g_logger.log("Connected stations: " + to_string(fromId) + " -> " + to_string(toId) + 
                 " using pipe " + to_string(selectedPipe->id));
}

void displayTopologicalOrder(NetworkGraph &graph) {
    vector<int> order = graph.topologicalSort();
    cout << "\n=== TOPOLOGICAL ORDER ===\n";
    if (order.empty()) {
        cout << "No stations in network or cycle detected\n";
        return;
    }
    cout << "Execution order: ";
    for (size_t i = 0; i < order.size(); i++) {
        cout << order[i];
        if (i < order.size() - 1) cout << " -> ";
    }
    cout << "\n";
    g_logger.log("Topological sort completed");
}

// Main menu
void showMenu() {
    cout << "\n=== PIPELINE MANAGEMENT (TASK 3) ===\n";
    cout << "PIPES: 1=Add, 2=View\n";
    cout << "STATIONS: 3=Add, 4=View\n";
    cout << "NETWORK: 5=Connect stations, 6=View graph, 7=Topological sort\n";
    cout << "0=Exit\nChoice: ";
}

int main() {
    vector<Pipe> pipes;
    vector<CompressorStation> stations;
    NetworkGraph graph;
    int choice;
    
    g_logger.log("=== Task 3 Program started ===");
    
    while (true) {
        showMenu();
        cin >> choice;
        
        switch (choice) {
            case 1:
                addPipe(pipes);
                break;
            case 2:
                displayAllPipes(pipes);
                break;
            case 3: {
                CompressorStation st;
                st.name = readString("Enter station name: ");
                st.totalWorkshops = readPositiveInt("Enter total workshops: ");
                st.workingWorkshops = readInt("Enter working workshops: ", 0, st.totalWorkshops);
                st.stationClass = readPositiveInt("Enter station class: ");
                stations.push_back(st);
                cout << "Station added (ID: " << st.id << ")\n";
                g_logger.log("Added station - ID: " + to_string(st.id) + ", Name: " + st.name);
                break;
            }
            case 4:
                displayAllStations(stations);
                break;
            case 5:
                connectStations(pipes, stations, graph);
                break;
            case 6:
                graph.displayGraph();
                break;
            case 7:
                displayTopologicalOrder(graph);
                break;
            case 0:
                g_logger.log("=== Program exited ===");
                return 0;
            default:
                cout << "Invalid choice\n";
        }
    }
    return 0;
}

// g++ third_task.cpp -o third_task.exe
// third_task.exe
