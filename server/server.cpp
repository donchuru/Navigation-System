//----------------------------------------------
//Name: David Onchuru
//SID: 1647809
//CCID: onchuru
//CMPUT 275, Winter 2022
//
//Assignment: Trivial Navigation System(Part 2)
//----------------------------------------------

#include <iostream>
#include <cassert>
#include <fstream>
#include <string>
#include <list>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#define _packet_MAX_LENGTH 1024
#include <sstream>

#include "wdigraph.h"
#include "dijkstra.h"

struct Point {
    long long lat, lon;
};

// returns the manhattan distance between two points
long long manhattan(const Point& pt1, const Point& pt2) {
  long long dLat = pt1.lat - pt2.lat, dLon = pt1.lon - pt2.lon;
  return abs(dLat) + abs(dLon);
}

// finds the id of the point that is closest to the given point "pt"
int findClosest(const Point& pt, const unordered_map<int, Point>& points) {
  pair<int, Point> best = *points.begin();

  for (const auto& check : points) {
    if (manhattan(pt, check.second) < manhattan(pt, best.second)) {
      best = check;
    }
  }
  return best.first;
}

// read the graph from the file that has the same format as the "Edmonton graph" file
void readGraph(const string& filename, WDigraph& g, unordered_map<int, Point>& points) {
  ifstream fin(filename);
  string line;

  while (getline(fin, line)) {
    // split the string around the commas, there will be 4 substrings either way
    string p[4];
    int at = 0;
    for (auto c : line) {
      if (c == ',') {
        // start new string
        ++at;
      }
      else {
        // append character to the string we are building
        p[at] += c;
      }
    }

    if (at != 3) {
      // empty line
      break;
    }

    if (p[0] == "V") {
      // new Point
      int id = stoi(p[1]);
      assert(id == stoll(p[1])); // sanity check: asserts if some id is not 32-bit
      points[id].lat = static_cast<long long>(stod(p[2])*100000);
      points[id].lon = static_cast<long long>(stod(p[3])*100000);
      g.addVertex(id);
    }
    else {
      // new directed edge
      int u = stoi(p[1]), v = stoi(p[2]);
      g.addEdge(u, v, manhattan(points[u], points[v]));
    }
  }
} 

int create_and_open_fifo(const char * pname, int mode) {
  // creating a fifo special file in the current working directory
  // with read-write permissions for communication with the plotter
  // both proecsses must open the fifo before they can perform
  // read and write operations on it
  if (mkfifo(pname, 0666) == -1) {
    cout << "Unable to make a fifo. Ensure that this pipe does not exist already!" << endl;
    exit(-1);
  }

  // opening the fifo for read-only or write-only access
  // a file descriptor that refers to the open file description is
  // returned
  int fd = open(pname, mode);

  if (fd == -1) {
    cout << "Error: failed on opening named pipe." << endl;
    exit(-1);
  }

  return fd;
}

// keep in mind that in part 1, the program should only handle 1 request
// in part 2, you need to listen for a new request the moment you are done
// handling one request
int main() {

  // initializations
  char packet[_packet_MAX_LENGTH] ={0}; // char array with 1024 0s

  WDigraph graph;
  unordered_map<int, Point> points;

  const char *inpipe = "inpipe";
  const char *outpipe = "outpipe";

  // Open the two pipes
  int in = create_and_open_fifo(inpipe, O_RDONLY);
  cout << "inpipe opened..." << endl;
  int out = create_and_open_fifo(outpipe, O_WRONLY);
  cout << "outpipe opened..." << endl;  

  // build the graph
  readGraph("server/edmonton-roads-2.0.1.txt", graph, points);

  /// read a request 
  while(true){
    read(in, packet, 1024);

    stringstream request(packet);

    // initializations
    string startlat;
    string startlon;
    string endlat;
    string endlon;
    Point sPoint, ePoint;

    // ask for request
    request >> startlat >> startlon >> endlat >> endlon;
    // if we close the plotter program, Q is sent by the client
    if (startlat == "Q"){
      break;
    }

    // convert the points to doubles
    double startlat2 = stod(startlat);
    double startlon2 = stod(startlon);
    double endlat2 = stod(endlat);
    double endlon2 = stod(endlon);

    // put them in the right format for processing shortest path(100000-ths)
    sPoint = {static_cast<long long>(startlat2*100000), static_cast<long long>(startlon2*100000)};
    ePoint = {static_cast<long long>(endlat2*100000), static_cast<long long>(endlon2*100000)};

    // get the points closest to the two points we read
    int start = findClosest(sPoint, points), end = findClosest(ePoint, points);

    // run dijkstra's algorithm, this is the unoptimized version that
    // does not stop when the end is reached but it is still fast enough
    unordered_map<int, PIL> tree;
    dijkstra(graph, start, tree);

    // no path
    if (tree.find(end) == tree.end()) {
      write(out, "E\n", 2);
    }
    else {
      // read off the path by stepping back through the search tree
      list<int> path;
      while (end != start) {
        path.push_front(end);
        end = tree[end].first;
      }
      path.push_front(start);

        for (int v : path) {
          // STATIC CAST back to a string
          string lat = to_string(static_cast<double>(points[v].lat/100000.0));
          // remove trailing 0
          lat.pop_back();
          string lon = to_string(static_cast<double>(points[v].lon/100000.0));
          lon.pop_back();
          string printed = lat + " " + lon+ "\n";
          // write all the waypoints to the outpipe
          write(out,(printed).c_str(), printed.length());
        }
        // write E to indicate end
      write(out, "E\n", 2);
    }
  }
    // close the file descriptors
    close(in);
    close(out);

    // reclaim the backing files
    unlink(inpipe);
    unlink(outpipe);

  return 0;
}
