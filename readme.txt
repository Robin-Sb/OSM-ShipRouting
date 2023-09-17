The README.md file contains instruction on how to run the application.
Attention: In order to generate the graph, the planet-coastlinespbf-cleanedosm.pbf files needs 
to be present in the files subfolder

In the first run, only the graph and the transit nodes are generated. 
In the second run, a simple benchmark is started, which compares the transit node algorithm to dijkstra. 
The results are stored in the /eval/ subfolder.

Overview:
The respective subfolders are 
- files: contains planet-coastlinespbf-cleanedosm.pbf
- src: contains all the source files, including:
    - main.cpp: starting point of the application
    - graph: contains the graph implementation, including dijkstra and the graph generation algorithm
    - io: contains utility to read/write files
    - utils: contains general utility functions
    - transit_nodes: contains the tranit nodes implementation including
        - TransitNodesDef: Contains multiple data structures for transit node routing
        - TransitNodesRouting: Main file, entry point for the transit node sweepline algorithm
        - SingleTnPass: implementation of a single pass of the sweepline, main part of the algorithm
        - TransitNodesQuery: contains the implementation of the query
- tns: contains the transit nodes (including distances), stored as binary data
- eval: evaluation results
- graphs: graph files

The implementation is based on the algorithm presented in: Bast, H., Funke S., Matijevic, D., TRANSIT:
Ultrafast Shortest-Path Queries with Linear-Time Preprocessing, 2006

In its core, the algorithm works as follows: 
For every vertex v along a sweepline, look at 5 gridcells to the left and to the right of v.
Between every node on the boundary of one of the cells on the left, compute the shortest path
to one of the nodes on the right.
If v is on the of these shortest path, it becomes a transit node.
