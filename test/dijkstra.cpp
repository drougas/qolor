// Downloaded from:
// http://solarianprogrammer.com/2012/07/21/compiling-gcc-4-7-1-mac-osx-lion

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <list>
#include <string>
#include <limits>
#include <qolor/all.hpp>
#include "testfn.h"

int main()
{
	std::map<int,double> estimated_distances;
	std::map<int,std::vector<std::pair<int,double>>> connections;
	std::map<int,std::pair<double,int>> distances;
	std::ifstream input("dijkstra.csv");
	bool success = true;

	success = ECHO_IF_FAILED2("cannot open dijkstra.csv", input.is_open()) && success;
	if (!success)
		return -1;

	auto iterable = qolor::from_csv(input)
		.select([](const std::vector<std::string>& v) {
			return std::make_tuple(std::stoi(v[0]), std::stoi(v[1]), std::stod(v[2]), std::stod(v[3]));
		});

	for (auto const& v : iterable) {
		int node0 = std::get<0>(v);
		int node1 = std::get<1>(v);
		double dist = std::get<2>(v);
		distances[node0] = distances[node1] = std::make_pair(std::numeric_limits<double>::max(), -1);
		estimated_distances[node1] = std::get<3>(v);
		connections[node0].push_back(std::make_pair(node1, dist));
		connections[node1].push_back(std::make_pair(node0, dist));
	}

	std::list<int> queue;

	distances[1] = std::make_pair(0.0, -1);
	queue.push_back(1);

	while (!queue.empty()) {
		int node = queue.front();
		double dist = distances[node].first;
		queue.pop_front();

		auto iter = qolor::from(connections[node])
			.where([&](const std::pair<int,double>& p) { return distances[p.first].first > dist + p.second; });
		for (auto const& neighbor : iter) {
			distances[neighbor.first] = std::make_pair(dist + neighbor.second, node);
			queue.push_back(neighbor.first);
		}
	}

	static const int correct_nodes[] = { 5, 6, 3, 1 };
	static const double correct_distances[] = { 20, 11, 9, 0 };
	static const int num_elements = sizeof(correct_nodes) / sizeof(correct_nodes[0]);

	int index = 0, node = correct_nodes[0];

	while ((node >= 0) && (index < num_elements)) {
		double dist = distances[node].first;
		double cdist = correct_distances[index];
		int cnode = correct_nodes[index];
		ECHO_IF_FAILED2(std::to_string(node) + ", correct node is " + std::to_string(cnode), (node == cnode));
		ECHO_IF_FAILED2(std::to_string(dist) + ", correct distance is node " + std::to_string(cdist), (dist == cdist));
		//std::cout << node << "  " << dist << std::endl;
		node = distances[node].second;
		++index;
	}
	ECHO_IF_FAILED2("Arriving at node 1", (index == num_elements));
	ECHO_IF_FAILED2("Finding the beginning", (node < 0));

	return 0;
}
