#include "trainsDijkstraImplem.c"

#define MAX_STATIONS 12                   // The amount and names of the stations are fixed
                                          // for the purpose of this exercise
                                    
char station_names[MAX_STATIONS][20] = {  // Attribute an index to each of the 12 stations (0-11)
    "Amsterdam", "Den Haag", "Den Helder", "Utrecht", "Eindhoven", "Nijmegen",
    "Maastricht", "Enschede", "Zwolle", "Groningen", "Leeuwarden", "Meppel"};

// Node in adjacency list - represents a neighbour
typedef struct Node {  // Add the word Node here so that I can use either the full struct Node
  int station;         // or the shorthand Node created by the typedef.
  int travel_time;
  struct Node* next;
} Node;

// Adjacency list structure - represents the neighbourhood of a vertex
typedef struct {
  Node* head;
} AdjacencyList;

// Array of adjacency lists with is used throughout the code
// - represents the graph (includes a neighbourhood - possibly empty - for each vertex)
AdjacencyList graph[MAX_STATIONS];

// Node in min-heap - represents a station with its current best-known distance from the source
typedef struct {
  int station;
  int distance;
} MinHeapNode;

// Min-heap structure - represents a priority queue of stations based on their current shortest distance
typedef struct {
  MinHeapNode* array;
  int size;
  int capacity;
  int* position;  // Auxiliary array -> Maps station index to position in the heap.
                  // It keeps track of where each station is located in the heap array.
                  // Finding the position of a specific station in the heap would require
                  // a linear search through the heap array, which is inefficient.
} MinHeap;

/*
  Helper function to get station index
*/

// Given a station name (input), returns the corresponding station index if it exists (output).
int get_station_index(const char* name);

/*
  Helper functions for adjacency list:
    create_node()
    add_edge()
    remove_edge()
*/

// Given a station index and a travel time (inputs), returns a pointer to a new adjacency-list node (output).
Node* create_node(int station, int travel_time);

// Given two station names and a travel time (inputs), adds a bidirectional edge
// between them in the adjacency list (no output).
void add_edge(const char* from, const char* to, int travel_time);

// Given two station names (inputs), removes the bidirectional edge between them (no output).
void remove_edge(const char* from, const char* to);

/*
  Helper functions for min-heap:
    create_min_heap()
    swap_nodes()
    downheap()
    remove_min()
    is_in_min_heap()
*/

// Given the capacity (input), returns a pointer to a newly allocated MinHeap (output).
MinHeap* create_min_heap(int capacity);

// Given two MinHeapNode pointers (input), swaps their contents (no output).
void swap_nodes(MinHeapNode* a, MinHeapNode* b);

// Given a pointer to a MinHeap and an index (inputs), solves possible conflicts between
// the parent and the children nodes until the heap order is restored (no output).
void downheap(MinHeap* heap, int index);

// Given a pointer to a MinHeap (input), removes and returns the node with the smallest distance (output).
// If the heap is empty, returns a node with station = -1 and distance = INF.
MinHeapNode remove_min(MinHeap* heap);

// Given a pointer to a MinHeap and a station index (inputs),
// returns 1 if the station is still in the heap, or 0 otherwise (output).
int is_in_min_heap(MinHeap* heap, int station);

/*
  Helper functions for graph representation
    initialize_graph()
    free_graph()
*/

// Builds the initial graph by setting all
// adjacency list heads to NULL, then adding predefined edges. (no input and no output)
void initialize_graph();

// Frees all dynamically allocated nodes, preventing memory leaks. (no input and no output)
void free_graph();

