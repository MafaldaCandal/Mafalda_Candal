#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include "trainsDijkstra.h"

/*
  Dijkstra's algorithm
*/

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
