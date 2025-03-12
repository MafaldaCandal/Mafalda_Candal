#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INF INT_MAX
#define MAX_STATIONS 12                   // The amount and names of the stations are fixed, so they are defined as global variables
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
int get_station_index(const char* name) {
  for (int i = 0; i < MAX_STATIONS; i++) {
    if (strcmp(name, station_names[i]) == 0) {
      return i;
    }
  }
  return -1;  // Return -1 if not found
}

/*
  Helper functions for adjacency list:
    create_node()
    add_edge()
    remove_edge()
*/

// Given a station index and a travel time (inputs), returns a pointer to a new adjacency-list node (output).
Node* create_node(int station, int travel_time) {
  Node* new_node = (Node*)malloc(sizeof(Node));  // allocate space
  new_node->station = station;                   // initialise
  new_node->travel_time = travel_time;
  new_node->next = NULL;
  return new_node;
}

// Given two station names and a travel time (inputs), adds a bidirectional edge
// between them in the adjacency list (no output).
void add_edge(const char* from, const char* to, int travel_time) {
  int from_index = get_station_index(from);
  int to_index = get_station_index(to);

  Node* new_node = create_node(to_index, travel_time);
  new_node->next = graph[from_index].head;  // add in the beginning of the adjacency list
  graph[from_index].head = new_node;        // update head node of adjacency list

  new_node = create_node(from_index, travel_time);  // graph is undirected, so same for other side
  new_node->next = graph[to_index].head;
  graph[to_index].head = new_node;
}

// Given two station names (inputs), removes the bidirectional edge between them (no output).
void remove_edge(const char* from, const char* to) {
  int from_index = get_station_index(from);
  int to_index = get_station_index(to);

  Node** current = &graph[from_index].head;  // create temporary copy of neighbourhood
  while (*current) {                         // iterate through the copy of the neighbourhood
    if ((*current)->station == to_index) {
      Node* temp = *current;
      *current = (*current)->next;
      free(temp);
      break;
    }
    current = &(*current)->next;
  }

  current = &graph[to_index].head;
  while (*current) {
    if ((*current)->station == from_index) {
      Node* temp = *current;
      *current = (*current)->next;
      free(temp);
      break;
    }
    current = &(*current)->next;
  }
}

/*
  Helper functions for min-heap:
    create_min_heap()
    swap_nodes()
    downheap()
    remove_min()
    decrease_dist()
    is_in_min_heap()
*/

// Given the capacity (input), returns a pointer to a newly allocated MinHeap (output).
MinHeap* create_min_heap(int capacity) {
  MinHeap* heap = (MinHeap*)malloc(sizeof(MinHeap));
  heap->array = (MinHeapNode*)malloc(capacity * sizeof(MinHeapNode));
  heap->size = 0;
  heap->capacity = capacity;
  heap->position = (int*)malloc(capacity * sizeof(int));
  return heap;
}

// Given two MinHeapNode pointers (input), swaps their contents (no output).
void swap_nodes(MinHeapNode* a, MinHeapNode* b) {
  MinHeapNode temp = *a;
  *a = *b;
  *b = temp;
}

// Given a pointer to a MinHeap and an index (inputs), solves possible conflicts between
// the parent and the children nodes until the heap order is restored (no output).
void downheap(MinHeap* heap, int index) {
  int smallest = index;
  int left = 2 * index + 1;
  int right = 2 * index + 2;

  if (left < heap->size && heap->array[left].distance < heap->array[smallest].distance)
    smallest = left;

  if (right < heap->size && heap->array[right].distance < heap->array[smallest].distance)
    smallest = right;

  if (smallest != index) {
    MinHeapNode smallestNode = heap->array[smallest];
    MinHeapNode indexNode = heap->array[index];

    heap->position[smallestNode.station] = index;
    heap->position[indexNode.station] = smallest;

    swap_nodes(&heap->array[smallest], &heap->array[index]);
    downheap(heap, smallest);
  }
}

// Given a pointer to a MinHeap (input), removes and returns the node with the smallest distance (output).
// If the heap is empty, returns a node with station = -1 and distance = INF.
MinHeapNode remove_min(MinHeap* heap) {
  if (heap->size == 0)
    return (MinHeapNode){-1, INF};

  MinHeapNode root = heap->array[0];
  MinHeapNode lastNode = heap->array[--heap->size];  // Size is decremented in this line
  heap->array[0] = lastNode;                         // Move the last node to the root

  heap->position[root.station] = heap->size;  // Root is now outside of heap, important for is_in_min_heap()
  heap->position[lastNode.station] = 0;

  downheap(heap, 0);
  return root;
}

// Given a pointer to a MinHeap, a station, and a new distance (inputs),
// updates that station's distance and adjusts its position (no output).
void decrease_dist(MinHeap* heap, int station, int distance) {
  int index = heap->position[station];
  heap->array[index].distance = distance;

  // Bubble up
  while (index && heap->array[index].distance < heap->array[(index - 1) / 2].distance) {
    heap->position[heap->array[index].station] = (index - 1) / 2;
    heap->position[heap->array[(index - 1) / 2].station] = index;
    swap_nodes(&heap->array[index], &heap->array[(index - 1) / 2]);
    index = (index - 1) / 2;
  }
}

// Given a pointer to a MinHeap and a station index (inputs),
// returns 1 if the station is still in the heap, or 0 otherwise (output).
int is_in_min_heap(MinHeap* heap, int station) {  // avoids linear search
  return heap->position[station] < heap->size;
}

/*
  Dijkstra's algorithm
*/

// Given start and goal station indices (inputs), runs Dijkstra's
// shortest path algorithm and prints the path and distance (no output).
void dijkstra(int start, int goal) {
  int distances[MAX_STATIONS];
  int previous[MAX_STATIONS];

  for (int i = 0; i < MAX_STATIONS; i++) {  // Ensure proper initialization of distances and previous
    distances[i] = INF;
    previous[i] = -1;
  }

  MinHeap* heap = create_min_heap(MAX_STATIONS);  // Create and set up the min-heap
  for (int i = 0; i < MAX_STATIONS; i++) {
    heap->array[i].station = i;
    heap->array[i].distance = INF;
    heap->position[i] = i;
  }
  heap->array[start].distance = 0;
  distances[start] = 0;
  decrease_dist(heap, start, 0);
  heap->size = MAX_STATIONS;

  while (heap->size > 0) {  // Extract the station with the smallest distance and relax edges
    MinHeapNode minNode = remove_min(heap);
    int u = minNode.station;

    Node* current = graph[u].head;
    while (current) {
      int v = current->station;
      if (is_in_min_heap(heap, v) && distances[u] != INF &&
          distances[u] + current->travel_time < distances[v]) {
        distances[v] = distances[u] + current->travel_time;
        previous[v] = u;
        decrease_dist(heap, v, distances[v]);
      }
      current = current->next;
    }
  }

  if (distances[goal] == INF) {  // Print the result
    printf("UNREACHABLE\n");
  } else {
    int path[MAX_STATIONS];
    int index = 0;

    for (int v = goal; v != -1; v = previous[v]) {  // Reconstruct the path using 'previous'
      path[index++] = v;
    }

    // Now 'index' is the total number of stations in the path.
    for (int i = index - 1; i >= 0; i--) {  // Print the path in correct order
      printf("%s\n", station_names[path[i]]);
    }
    printf("%d\n", distances[goal]);
  }

  free(heap->array);     // free memory
  free(heap->position);  // it seemed too simple and obvious to add a helper functions (there are so many already)
  free(heap);
}

/*
  Helper functions for graph representation
    initialize_graph()
    free_graph()
*/

// Builds the initial graph by setting all
// adjacency list heads to NULL, then adding predefined edges. (no input and no output)
void initialize_graph() {
  for (int i = 0; i < MAX_STATIONS; i++) {
    graph[i].head = NULL;
  }

  add_edge("Amsterdam", "Den Haag", 46);
  add_edge("Amsterdam", "Den Helder", 77);
  add_edge("Amsterdam", "Utrecht", 26);
  add_edge("Den Haag", "Eindhoven", 89);
  add_edge("Eindhoven", "Maastricht", 63);
  add_edge("Eindhoven", "Nijmegen", 55);
  add_edge("Eindhoven", "Utrecht", 47);
  add_edge("Enschede", "Zwolle", 50);
  add_edge("Groningen", "Leeuwarden", 34);
  add_edge("Groningen", "Meppel", 49);
  add_edge("Leeuwarden", "Meppel", 40);
  add_edge("Maastricht", "Nijmegen", 111);
  add_edge("Meppel", "Zwolle", 15);
  add_edge("Nijmegen", "Zwolle", 77);
  add_edge("Utrecht", "Zwolle", 51);
}

// Frees all dynamically allocated nodes, preventing memory leaks. (no input and no output)
void free_graph() {
  for (int i = 0; i < MAX_STATIONS; i++) {
    Node* current = graph[i].head;
    while (current) {
      Node* temp = current;
      current = current->next;
      free(temp);
    }
    graph[i].head = NULL;
  }
}

int main() {
  initialize_graph();

  // Deal with disruptions
  int num_disruptions;  // no need to initialise, because of scanf below
  scanf("%d", &num_disruptions);
  for (int i = 0; i < num_disruptions; i++) {
    char from[50], to[50];
    scanf(" %49[^\n]", from);  // Read station names safely (avoid buffer overflow)
    scanf(" %49[^\n]", to);    // [^\n] means "read any character except \n"

    int from_index = get_station_index(from);
    int to_index = get_station_index(to);

    if (from_index == -1) {
      printf("Error: station '%s' does not exist.\n", from);
      continue;  // Skip removal
    }
    if (to_index == -1) {
      printf("Error: station '%s' does not exist.\n", to);
      continue;  // Skip removal
    }
    remove_edge(from, to);
  }

  // Deal with queries
  char from[50], to[50];
  while (1) {  // Don't know ahead of time how many queries
    scanf(" %49[^\n]", from);
    if (from[0] == '!')  // Check for the termination character
      break;
    scanf(" %49[^\n]", to);

    int from_index = get_station_index(from);
    int to_index = get_station_index(to);

    if (from_index == -1 || to_index == -1) {
      printf("Error: one or both stations are invalid.\n");
      continue;  // Skip the route calculation
    }

    dijkstra(from_index, to_index);
  }

  free_graph();
  return 0;
}
