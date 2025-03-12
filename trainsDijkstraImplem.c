#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int get_station_index(const char* name) {
  for (int i = 0; i < MAX_STATIONS; i++) {
    if (strcmp(name, station_names[i]) == 0) {
      return i;
    }
  }
  return -1;  // Return -1 if not found
}

Node* create_node(int station, int travel_time) {
  Node* new_node = (Node*)malloc(sizeof(Node));  // allocate space
  new_node->station = station;                   // initialise
  new_node->travel_time = travel_time;
  new_node->next = NULL;
  return new_node;
}

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

MinHeap* create_min_heap(int capacity) {
  MinHeap* heap = (MinHeap*)malloc(sizeof(MinHeap));
  heap->array = (MinHeapNode*)malloc(capacity * sizeof(MinHeapNode));
  heap->size = 0;
  heap->capacity = capacity;
  heap->position = (int*)malloc(capacity * sizeof(int));
  return heap;
}

void swap_nodes(MinHeapNode* a, MinHeapNode* b) {
  MinHeapNode temp = *a;
  *a = *b;
  *b = temp;
}

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

int is_in_min_heap(MinHeap* heap, int station) {  // avoids linear search
  return heap->position[station] < heap->size;
}

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