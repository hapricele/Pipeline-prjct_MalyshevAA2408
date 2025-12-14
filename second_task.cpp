#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <limits>
#include <ctime>
#include <sstream>
#include <algorithm>

using namespace std;

// логгер для правильного открытия и закрытия файла
class Logger
{
    ofstream logFile;

    string getCurrentTime()
    {
        time_t now = time(nullptr);
        tm *timeinfo = localtime(&now);
        char buffer[20];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
        return string(buffer);
    }

public:
    Logger(const string &filename = "pipeline_log.txt")
    {
        logFile.open(filename, ios::app);
    }
    ~Logger()
    {
        if (logFile.is_open())
            logFile.close();
    }

    void log(const string &action)
    {
        if (logFile.is_open())
        {
            logFile << "[" << getCurrentTime() << "] " << action << "\n";
            logFile.flush();
        }
    }
};

Logger g_logger; // глобальная переменная чтобы вызывать

// структуры
struct Pipe
{
    static int nextId;
    int id;
    string name;
    double length;
    int diameter;
    bool underRepair;

    Pipe() : id(++nextId), length(0), diameter(0), underRepair(false) {}
    // функции валидации ввода
    double getLength() const { return length; }
    int getDiameter() const { return diameter; }
    bool isUnderRepair() const { return underRepair; }
    void setRepairStatus(bool status) { underRepair = status; }
};

int Pipe::nextId = 0;

struct CompressorStation
{
    static int nextId;
    int id;
    string name;
    int totalWorkshops;
    int workingWorkshops;
    int stationClass;

    CompressorStation() : id(++nextId), totalWorkshops(0), workingWorkshops(0), stationClass(0) {}

    double getUnusedPercent() const
    {
        return totalWorkshops == 0 ? 0 : (double)(totalWorkshops - workingWorkshops) / totalWorkshops * 100;
    }
    void adjustWorkshops(int delta)
    {
        workingWorkshops = max(0, min(totalWorkshops, workingWorkshops + delta));
    }
};

int CompressorStation::nextId = 0;

// ============ INPUT VALIDATION ============
double readPositiveDouble(const string &prompt)
{
    double value;
    cout << prompt;
    while (!(cin >> value) || value <= 0)
    {
        cout << "Invalid input. Enter positive number: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    return value;
}

int readPositiveInt(const string &prompt)
{
    int value;
    cout << prompt;
    while (!(cin >> value) || value <= 0)
    {
        cout << "Invalid input. Enter positive number: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        // чтобы программа не падала и не зацикливалась при кривом вводе
    }
    return value;
}

int readInt(const string &prompt, int minVal, int maxVal)
{
    int value;
    cout << prompt;
    while (!(cin >> value) || value < minVal || value > maxVal)
    {
        cout << "Invalid input. Enter number between " << minVal << " and " << maxVal << ": ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    return value;
}

string readString(const string &prompt)
{
    cout << prompt;
    cin.ignore();
    string result;
    getline(cin, result);
    return result;
}

//  PIPE OPERATIONS
void addPipe(vector<Pipe> &pipes)
{
    Pipe pipe;
    pipe.name = readString("Enter pipe name: ");
    pipe.length = readPositiveDouble("Enter pipe length (km): ");
    pipe.diameter = readPositiveInt("Enter pipe diameter (mm): ");

    pipes.push_back(pipe);
    cout << "Pipe added (ID: " << pipe.id << ")\n";
    g_logger.log("Added pipe - ID: " + to_string(pipe.id) + ", Name: " + pipe.name);
}

void displayPipe(const Pipe &pipe)
{
    cout << "[ID:" << pipe.id << "] " << pipe.name
         << " | " << pipe.length << "km, Ø" << pipe.diameter << "mm"
         << " | " << (pipe.underRepair ? "REPAIR" : "OK") << "\n";
}

void displayAllPipes(const vector<Pipe> &pipes)
{
    if (pipes.empty())
    {
        cout << "No pipes\n";
        return;
    }
    cout << "\n=== PIPES ===\n";
    for (const auto &p : pipes)
        displayPipe(p);
}

vector<Pipe *> searchPipesByName(vector<Pipe> &pipes, const string &name) // возвращаются указатели на элементы внутри исходного
{
    vector<Pipe *> results;
    for (auto &p : pipes)
        if (p.name.find(name) != string::npos)
            results.push_back(&p);
    g_logger.log("Search pipes by name: '" + name + "' -> " + to_string(results.size()));
    return results;
}

vector<Pipe *> searchPipesByRepair(vector<Pipe> &pipes, bool repair)
{
    vector<Pipe *> results;
    for (auto &p : pipes)
        if (p.underRepair == repair)
            results.push_back(&p);
    g_logger.log("Search pipes by repair: " + string(repair ? "yes" : "no") + " -> " + to_string(results.size()));
    return results;
}

void deletePipesFromVector(vector<Pipe> &pipes, const vector<Pipe *> &toDelete)
{
    for (auto pipePtr : toDelete)
    {
        pipes.erase(remove_if(pipes.begin(), pipes.end(),
                              [pipePtr](const Pipe &p)
                              { return p.id == pipePtr->id; }),
                    pipes.end());
    }
}

void batchEditPipes(vector<Pipe> &pipes, vector<Pipe *> &results, int action)
{
    if (results.empty())
    {
        cout << "No pipes found\n";
        return;
    }

    cout << "\nFound " << results.size() << " pipe(s):\n";
    for (size_t i = 0; i < results.size(); i++)
    {
        cout << (i + 1) << ". ";
        displayPipe(*results[i]);
    }

    cout << "\n1=Edit all, 2=Select, 0=Cancel: ";
    int choice;
    cin >> choice;

    vector<Pipe *> selected;
    if (choice == 1)
        selected = results;
    else if (choice == 2)
    {
        cout << "Enter numbers (1-" << results.size() << ") space-separated: ";
        cin.ignore();
        string input;
        getline(cin, input);
        istringstream iss(input);
        int num;
        while (iss >> num)
            if (num > 0 && num <= (int)results.size())
                selected.push_back(results[num - 1]);
    }
    else
        return;

    if (action == 1)
    {
        for (auto p : selected)
            p->underRepair = !p->underRepair;
        cout << "Repair status changed: " << selected.size() << " pipe(s)\n";
        g_logger.log("Batch: toggled repair on " + to_string(selected.size()) + " pipe(s)");
    }
    else if (action == 2)
    {
        cout << "Delete " << selected.size() << " pipe(s)? (y/n): ";
        char c;
        cin >> c;
        if (c == 'y' || c == 'Y')
        {
            deletePipesFromVector(pipes, selected);
            cout << "Deleted: " << selected.size() << " pipe(s)\n";
            g_logger.log("Batch: deleted " + to_string(selected.size()) + " pipe(s)");
        }
    }
}

// ============ STATION OPERATIONS ============
void addStation(vector<CompressorStation> &stations)
{
    CompressorStation st;
    st.name = readString("Enter station name: ");
    st.totalWorkshops = readPositiveInt("Enter total workshops: ");
    st.workingWorkshops = readInt("Enter working workshops: ", 0, st.totalWorkshops);
    st.stationClass = readPositiveInt("Enter station class: ");

    stations.push_back(st);
    cout << "Station added (ID: " << st.id << ")\n";
    g_logger.log("Added station - ID: " + to_string(st.id) + ", Name: " + st.name);
}

void displayStation(const CompressorStation &st)
{
    cout << "[ID:" << st.id << "] " << st.name
         << " | " << st.workingWorkshops << "/" << st.totalWorkshops << " working"
         << " | Unused: " << fixed << (int)st.getUnusedPercent() << "%"
         << " | Class:" << st.stationClass << "\n";
}

void displayAllStations(const vector<CompressorStation> &stations)
{
    if (stations.empty())
    {
        cout << "No stations\n";
        return;
    }
    cout << "\n=== STATIONS ===\n";
    for (const auto &s : stations)
        displayStation(s);
}

vector<CompressorStation *> searchStationsByName(vector<CompressorStation> &stations, const string &name)
{
    vector<CompressorStation *> results;
    for (auto &s : stations)
        if (s.name.find(name) != string::npos)
            results.push_back(&s);
    g_logger.log("Search stations by name: '" + name + "' -> " + to_string(results.size()));
    return results;
}

vector<CompressorStation *> searchStationsByUnused(vector<CompressorStation> &stations, double minPercent)
{
    vector<CompressorStation *> results;
    for (auto &s : stations)
        if (s.getUnusedPercent() >= minPercent)
            results.push_back(&s);
    g_logger.log("Search stations by unused >= " + to_string((int)minPercent) + "% -> " + to_string(results.size()));
    return results;
}

void editStation(CompressorStation &st)
{
    cout << "1=Start workshop, 2=Stop workshop, 0=Back: ";
    int choice;
    cin >> choice;
    if (choice == 1)
    {
        st.adjustWorkshops(1); // запуск или остановка цеха
        cout << "Working: " << st.workingWorkshops << "/" << st.totalWorkshops << "\n";
        g_logger.log("Station " + to_string(st.id) + ": started workshop");
    }
    else if (choice == 2)
    {
        st.adjustWorkshops(-1);
        cout << "Working: " << st.workingWorkshops << "/" << st.totalWorkshops << "\n";
        g_logger.log("Station " + to_string(st.id) + ": stopped workshop");
    }
}

// ============ FILE I/O ============
void saveToFile(const vector<Pipe> &pipes, const vector<CompressorStation> &stations)
{
    string filename = readString("Enter filename to save: ");
    if (filename.empty())
        filename = "pipeline_data.txt";

    ofstream file(filename);
    if (!file.is_open())
    {
        cout << "Error: cannot open file\n";
        return;
    }

    file << "PIPES " << pipes.size() << "\n";
    for (const auto &p : pipes)
        file << p.id << "|" << p.name << "|" << p.length << "|" << p.diameter << "|" << p.underRepair << "\n";

    file << "STATIONS " << stations.size() << "\n";
    for (const auto &s : stations)
        file << s.id << "|" << s.name << "|" << s.totalWorkshops << "|" << s.workingWorkshops << "|" << s.stationClass << "\n";

    file.close();
    cout << "Saved to '" << filename << "'\n";
    g_logger.log("Saved to '" + filename + "' - pipes:" + to_string(pipes.size()) + ", stations:" + to_string(stations.size()));
}

void loadFromFile(vector<Pipe> &pipes, vector<CompressorStation> &stations)
{
    string filename = readString("Enter filename to load: ");
    if (filename.empty())
        filename = "pipeline_data.txt";

    ifstream file(filename);
    if (!file.is_open())
    {
        cout << "Error: cannot open file\n";
        return;
    }

    pipes.clear();
    stations.clear();

    string line, section;
    while (getline(file, line))
    {
        if (line.empty())
            continue;
        istringstream iss(line);
        string word;
        iss >> word;

        if (word == "PIPES")
            section = "PIPES";
        else if (word == "STATIONS")
            section = "STATIONS";
        else if (section == "PIPES")
        {
            int id, diameter;
            string name;
            double length;
            bool repair;
            char delim;
            istringstream iss(line);
            iss >> id >> delim >> name >> delim >> length >> delim >> diameter >> delim >> repair;
            Pipe p;
            p.id = id;
            p.name = name;
            p.length = length;
            p.diameter = diameter;
            p.underRepair = repair;
            pipes.push_back(p);
        }
        else if (section == "STATIONS")
        {
            int id, total, working, cls;
            string name;
            char delim;
            istringstream iss(line);
            iss >> id >> delim >> name >> delim >> total >> delim >> working >> delim >> cls;
            CompressorStation s;
            s.id = id;
            s.name = name;
            s.totalWorkshops = total;
            s.workingWorkshops = working;
            s.stationClass = cls;
            stations.push_back(s);
        }
    }

    file.close();
    cout << "Loaded from '" << filename << "' - " << pipes.size() << " pipes, " << stations.size() << " stations\n";
    g_logger.log("Loaded from '" + filename + "' - pipes:" + to_string(pipes.size()) + ", stations:" + to_string(stations.size()));
}

void viewLog()
{
    ifstream file("pipeline_log.txt");
    if (!file.is_open())
    {
        cout << "No log file\n";
        return;
    }
    cout << "\n=== LOG ===\n";
    string line;
    while (getline(file, line))
        cout << line << "\n";
    file.close();
}

// ============ MENU ============
void showMenu()
{
    cout << "\n=== PIPELINE MANAGEMENT ===\n";
    cout << "PIPES: 1=Add, 2=View, 3=Search by name, 4=Search by repair, 5=Edit pipes\n";
    cout << "STATIONS: 6=Add, 7=View, 8=Search by name, 9=Search by unused, 10=Edit station\n";
    cout << "FILES: 11=Save, 12=Load, 13=View log\n";
    cout << "0=Exit\nChoice: ";
}

// ============ MAIN ============
int main()
{
    vector<Pipe> pipes;
    vector<CompressorStation> stations;
    int choice;

    g_logger.log("=== Program started ===");

    while (true)
    {
        showMenu();
        cin >> choice;

        switch (choice)
        {
        case 1:
            addPipe(pipes);
            break;
        case 2:
            displayAllPipes(pipes);
            break;
        case 3:
        {
            string name = readString("Search pipe name: ");
            auto r = searchPipesByName(pipes, name);
            if (!r.empty())
            {
                cout << "\nFound:\n";
                for (auto p : r)
                    displayPipe(*p);
            }
            else
                cout << "Not found\n";
            break;
        }
        case 4:
        {
            cout << "1=Under repair, 2=Operational: ";
            int c;
            cin >> c;
            auto r = searchPipesByRepair(pipes, c == 1);
            if (!r.empty())
            {
                cout << "\nFound:\n";
                for (auto p : r)
                    displayPipe(*p);
            }
            else
                cout << "Not found\n";
            break;
        }
        case 5:
        {
            string name = readString("Search pipe name: ");
            auto r = searchPipesByName(pipes, name);
            if (!r.empty())
            {
                cout << "1=Toggle repair, 2=Delete: ";
                int act;
                cin >> act;
                batchEditPipes(pipes, r, act);
            }
            else
                cout << "Not found\n";
            break;
        }
        case 6:
            addStation(stations);
            break;
        case 7:
            displayAllStations(stations);
            break;
        case 8:
        {
            string name = readString("Search station name: ");
            auto r = searchStationsByName(stations, name);
            if (!r.empty())
            {
                cout << "\nFound:\n";
                for (auto s : r)
                    displayStation(*s);
            }
            else
                cout << "Not found\n";
            break;
        }
        case 9:
        {
            double pct = readPositiveDouble("Min unused %: ");
            auto r = searchStationsByUnused(stations, pct);
            if (!r.empty())
            {
                cout << "\nFound:\n";
                for (auto s : r)
                    displayStation(*s);
            }
            else
                cout << "Not found\n";
            break;
        }
        case 10:
        {
            displayAllStations(stations);
            if (!stations.empty())
            {
                cout << "Enter station ID: ";
                int id;
                cin >> id;
                for (auto &s : stations)
                    if (s.id == id)
                    {
                        editStation(s);
                        break;
                    }
            }
            break;
        }
        case 11:
            saveToFile(pipes, stations);
            break;
        case 12:
            loadFromFile(pipes, stations);
            break;
        case 13:
            viewLog();
            break;
        case 0:
            g_logger.log("=== Program exited ===");
            return 0;
        default:
            cout << "Invalid\n";
        }
    }
    return 0;
}
// g++ second_task.cpp -o second_task.exe
// second_task.exe
