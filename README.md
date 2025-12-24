# Pipeline Management System - Third Task

## Project Overview
Enhanced gas transportation pipeline management system with network graph formation and topological sorting.

## Key Features

### Task 1: Pipe and Station Management
- Create and manage pipes with: name, length, diameter, repair status
- Create and manage compressor stations with workshops data
- Search and filter pipes/stations by various criteria
- Edit multiple pipes/stations in batch operations
- Log all operations to pipeline_log.txt

### Task 2: Gas Transport Network Formation
- **Connect Stations**: Link compressor stations using pipes
- **User Input**: Source Station ID, Destination Station ID, Required pipe diameter (500, 700, 1000, 1400 mm)
- **Smart Pipe Allocation**: Find available pipe or create new one
- Ensure pipes are not under repair before using

### Task 3: Topological Sorting
- Build directed acyclic graph from connections
- Implement Kahn's Algorithm for topological sorting
- O(V + E) time complexity
- Display network graph structure
- Export network topology to file

## Data Structures

```cpp
struct Edge {
    int pipeId;        // Connected pipe
    int toStationId;   // Destination
    int diameter;      // Diameter mm
};

struct NetworkGraph {
    map<int, vector<Edge>> adjacencyList;
    map<int, int> inDegree;
    vector<int> topologicalSort();
};
```
