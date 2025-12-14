#include <iostream>
#include <fstream>
#include <string>
#include <limits>

using namespace std;

struct Pipe
{
    string name;
    double length;
    int diameter;
    bool underRepair;
};

struct CompressorStation
{
    string name;
    int totalWorkshops;
    int workingWorkshops;
    int stationClass;
};

void inputPipe(Pipe &pipe)
{
    cout << "Enter pipe name: ";
    cin.ignore();
    getline(cin, pipe.name);

    cout << "Enter pipe length (km): ";
    while (!(cin >> pipe.length) || pipe.length <= 0)
    {
        cout << "Invalid input. Enter positive number: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    cout << "Enter pipe diameter (mm): ";
    while (!(cin >> pipe.diameter) || pipe.diameter <= 0)
    {
        cout << "Invalid input. Enter positive number: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    pipe.underRepair = false;
    cout << "Pipe added successfully!\n";
}

void displayPipe(const Pipe &pipe)
{
    if (pipe.name.empty())
    {
        cout << "No pipe data available\n";
        return;
    }
    cout << "Pipe: " << pipe.name
         << ", Length: " << pipe.length << " km"
         << ", Diameter: " << pipe.diameter << " mm"
         << ", Under repair: " << (pipe.underRepair ? "Yes" : "No") << endl;
}

void togglePipeRepair(Pipe &pipe)
{
    if (pipe.name.empty())
    {
        cout << "No pipe to edit\n";
        return;
    }
    pipe.underRepair = !pipe.underRepair;
    cout << "Pipe repair status changed to: " << (pipe.underRepair ? "Under repair" : "Operational") << endl;
}

void inputStation(CompressorStation &station)
{
    cout << "Enter station name: ";
    cin.ignore();
    getline(cin, station.name);

    cout << "Enter total number of workshops: ";
    while (!(cin >> station.totalWorkshops) || station.totalWorkshops <= 0)
    {
        cout << "Invalid input. Enter positive number: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    cout << "Enter number of working workshops: ";
    while (!(cin >> station.workingWorkshops) || station.workingWorkshops < 0 || station.workingWorkshops > station.totalWorkshops)
    {
        cout << "Invalid input. Enter number between 0 and " << station.totalWorkshops << ": ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    cout << "Enter station class: ";
    while (!(cin >> station.stationClass) || station.stationClass <= 0)
    {
        cout << "Invalid input. Enter positive number: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    cout << "Compressor station added successfully!\n";
}

void displayStation(const CompressorStation &station)
{
    if (station.name.empty())
    {
        cout << "No station data available\n";
        return;
    }
    cout << "Station: " << station.name
         << ", Workshops: " << station.workingWorkshops << "/" << station.totalWorkshops
         << ", Class: " << station.stationClass << endl;
}

void startWorkshop(CompressorStation &station)
{
    if (station.name.empty())
    {
        cout << "No station to edit\n";
        return;
    }
    if (station.workingWorkshops < station.totalWorkshops)
    {
        station.workingWorkshops++;
        cout << "Workshop started. Now working: " << station.workingWorkshops << "/" << station.totalWorkshops << endl;
    }
    else
    {
        cout << "All workshops are already working\n";
    }
}

void stopWorkshop(CompressorStation &station)
{
    if (station.name.empty())
    {
        cout << "No station to edit\n";
        return;
    }
    if (station.workingWorkshops > 0)
    {
        station.workingWorkshops--;
        cout << "Workshop stopped. Now working: " << station.workingWorkshops << "/" << station.totalWorkshops << endl;
    }
    else
    {
        cout << "No working workshops to stop\n";
    }
}

void saveToFile(const Pipe &pipe, const CompressorStation &station, const string &filename)
{
    ofstream file(filename);
    if (file.is_open())
    {
        file << "PIPE\n";
        file << pipe.name << "\n";
        file << pipe.length << "\n";
        file << pipe.diameter << "\n";
        file << pipe.underRepair << "\n";

        file << "STATION\n";
        file << station.name << "\n";
        file << station.totalWorkshops << "\n";
        file << station.workingWorkshops << "\n";
        file << station.stationClass << "\n";

        cout << "Data saved to " << filename << " successfully!\n";
    }
    else
    {
        cout << "Error saving file\n";
    }
}

void loadFromFile(Pipe &pipe, CompressorStation &station, const string &filename)
{
    ifstream file(filename);
    if (file.is_open())
    {
        string line;

        getline(file, line);
        getline(file, pipe.name);
        file >> pipe.length;
        file >> pipe.diameter;
        file >> pipe.underRepair;
        file.ignore();

        getline(file, line);
        getline(file, station.name);
        file >> station.totalWorkshops;
        file >> station.workingWorkshops;
        file >> station.stationClass;

        cout << "Data loaded from " << filename << " successfully!\n";
    }
    else
    {
        cout << "Error loading file\n";
    }
}

void displayMenu()
{
    cout << "\n=== Pipeline Management System ===\n";
    cout << "1. Add pipe\n";
    cout << "2. Add compressor station\n";
    cout << "3. View all objects\n";
    cout << "4. Edit pipe (toggle repair status)\n";
    cout << "5. Edit station\n";
    cout << "6. Save to file\n";
    cout << "7. Load from file\n";
    cout << "0. Exit\n";
    cout << "Choose an option: ";
}

void displayStationMenu()
{
    cout << "\n=== Station Editing ===\n";
    cout << "1. Start workshop\n";
    cout << "2. Stop workshop\n";
    cout << "0. Back to main menu\n";
    cout << "Choose an option: ";
}

int main()
{
    Pipe pipe;
    CompressorStation station;
    int choice;
    const string filename = "pipeline_data.txt";

    pipe.name = "";
    station.name = "";

    do
    {
        displayMenu();
        cin >> choice;

        switch (choice)
        {
        case 1:
            inputPipe(pipe);
            break;
        case 2:
            inputStation(station);
            break;
        case 3:
            cout << "\n--- Current Objects ---\n";
            displayPipe(pipe);
            displayStation(station);
            break;
        case 4:
            togglePipeRepair(pipe);
            break;
        case 5:
            if (station.name.empty())
            {
                cout << "No station to edit\n";
            }
            else
            {
                int stationChoice;
                do
                {
                    displayStationMenu();
                    cin >> stationChoice;
                    switch (stationChoice)
                    {
                    case 1:
                        startWorkshop(station);
                        break;
                    case 2:
                        stopWorkshop(station);
                        break;
                    case 0:
                        break;
                    default:
                        cout << "Invalid option\n";
                    }
                } while (stationChoice != 0);
            }
            break;
        case 6:
            saveToFile(pipe, station, filename);
            break;
        case 7:
            loadFromFile(pipe, station, filename);
            break;
        case 0:
            cout << "Exiting program. Goodbye!\n";
            break;
        default:
            cout << "Invalid option. Please try again.\n";
        }

        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

    } while (choice != 0);

    return 0;
}