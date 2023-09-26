#include <algorithm>
#include <ctime>
#include <fstream>
#include <functional>
#include <iostream>
#include <numeric>
#include <random>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

using GraphType = std::vector<std::unordered_set<size_t>>;

enum class SortOrder { None, ASC, DESC, DESC_SHUFFLE, ASC_SHUFFLE, SHUFFLE };

class ColorigProblem {
private:
  GraphType graph;
  std::vector<size_t> indexes;
  std::vector<size_t> colors;
  size_t maxColor = 0;
  double timeSpent = 0;
  std::string problemFile;

  template <typename T>
  std::vector<size_t> findItems(std::vector<T> const &v, T target) {
    std::vector<size_t> indices;
    auto it = v.begin();
    while ((it = std::find_if(it, v.end(), [&](T const &e) {
              return e == target;
            })) != v.end()) {
      indices.push_back(static_cast<size_t>(std::distance(v.begin(), it)));
      it++;
    }
    return indices;
  }

public:
  void ReadFile(std::string filename) {
    std::ifstream file;
    file.open(filename);

    problemFile = filename;

    std::string line;

    while (!file.eof() && getline(file, line)) {
      if (line[0] == 'c')
        continue;

      std::stringstream line_input(line);
      if (line[0] == 'p') {
        std::string command, problemTitle;
        size_t numberOfNodes, numberOfEdges;
        line_input >> command >> problemTitle >> numberOfNodes >> numberOfEdges;
        graph.resize(numberOfNodes);
        colors.resize(numberOfNodes);
        indexes.resize(numberOfNodes);
      }

      if (line[0] == 'e') {
        std::string command;
        size_t node1, node2;
        line_input >> command >> node1 >> node2;
        graph[node1 - 1].insert(node2);
        graph[node2 - 1].insert(node1);
      }
    }

    std::iota(indexes.begin(), indexes.end(), 0);
  }

  void Print_Sorted_Graph() {
    std::cout << "Sorted Graph\n";

    for (size_t i = 0; i < indexes.size(); ++i) {
      std::cout << indexes[i] + 1 << ": ";

      for (auto node : graph[indexes[i]]) {
        std::cout << node << " ";
      }
      std::cout << "\n";
    }
  }

  void Print_Colors() {
    std::cout << "max color: " << maxColor << "\ncolors\n";
    for (size_t i = 0; i < colors.size(); ++i) {
      std::cout << i + 1 << ": " << colors[i] << "\n";
    }
  }

  void GreedyColoring(SortOrder order = SortOrder::None) {
    std::clock_t start = std::clock();

    switch (order) {
    case SortOrder::ASC:
      std::sort(indexes.begin(), indexes.end(), [&](size_t left, size_t right) {
        return graph[left].size() < graph[right].size();
      });
      break;

    case SortOrder::DESC:
      std::sort(indexes.begin(), indexes.end(), [&](size_t left, size_t right) {
        return graph[left].size() > graph[right].size();
      });
      break;

    case SortOrder::DESC_SHUFFLE: {
      std::sort(indexes.begin(), indexes.end(), [&](size_t left, size_t right) {
        return graph[left].size() > graph[right].size();
      });

      size_t lb = 0, rb = 0;

      for (size_t i = 0; i < indexes.size(); ++i) {
        if (graph[indexes[lb]].size() != graph[indexes[i]].size()) {
          rb = i;
          unsigned seed = static_cast<unsigned>(std::clock());
          std::shuffle(std::next(indexes.begin(), static_cast<int>(lb)),
                       std::next(indexes.begin(), static_cast<int>(rb)),
                       std::default_random_engine(seed));
          lb = i;
        }
      }

    } break;

    case SortOrder::ASC_SHUFFLE: {
      std::sort(indexes.begin(), indexes.end(), [&](size_t left, size_t right) {
        return graph[left].size() < graph[right].size();
      });

      size_t lb = 0, rb = 0;

      for (size_t i = 0; i < indexes.size(); ++i) {
        if (graph[indexes[lb]].size() != graph[indexes[i]].size()) {
          rb = i;
          unsigned seed = static_cast<unsigned>(std::clock());
          std::shuffle(std::next(indexes.begin(), static_cast<int>(lb)),
                       std::next(indexes.begin(), static_cast<int>(rb)),
                       std::default_random_engine(seed));
          lb = i;
        }
      }

    } break;

    case SortOrder::SHUFFLE: {
      unsigned seed = static_cast<unsigned>(std::clock());
      std::shuffle(indexes.begin(), indexes.end(),
                   std::default_random_engine(seed));
    }

    default:
      break;
    }

    for (auto i : indexes) {
      if (colors[i])
        continue;

      ++maxColor;

      std::unordered_set<size_t> banned_nodes(graph[i]);
      colors[i] = maxColor;
      for (size_t node = 0; node < graph.size(); ++node) {
        if (colors[node] || banned_nodes.count(node + 1)) {
          continue;
        }
        colors[node] = maxColor;
        banned_nodes.insert(graph[node].begin(), graph[node].end());
      }
    }

    timeSpent =
        static_cast<double>(std::clock() - start) / (double)CLOCKS_PER_SEC;
  }

  inline size_t getMaxColor() { return maxColor; }
  inline double getTime() { return timeSpent; }

  void resetResults() {
    maxColor = 0;
    std::fill(colors.begin(), colors.end(), 0);
    std::iota(indexes.begin(), indexes.end(), 0);
    timeSpent = 0;
  }

  void Save_Results(std::string filename) {
    std::ofstream output(filename, std::ios_base::app);
    output << problemFile << ";" << maxColor << ";" << timeSpent << ";[";

    for (size_t i = 1; i <= maxColor; ++i) {
      std::vector<size_t> indices = findItems(colors, i);
      output << "[";
      for (auto &e : indices) {
        output << e + 1 << ',';
      }

      output << ((i != maxColor) ? "]," : "]\n");
    }
    output.close();
  }
};

int main() {
  std::string baseTestPath = "tests/";
  std::vector<std::string> files = {
      "myciel3.col",   "myciel7.col",   "school1.col",   "school1_nsh.col",
      "anna.col",      "miles1000.col", "miles1500.col", "le450_5a.col",
      "le450_15b.col", "queen11_11.col"};

  std::string baseResPath = "results/";
  std::string resultFile = "res.csv";

  std::ofstream output(baseResPath + resultFile);
  output << "Problem name;Number of colors;Time\n";

  std::vector<SortOrder> sorts = {SortOrder::None, SortOrder::ASC,
                                  SortOrder::DESC};

  for (auto filename : files) {
    ColorigProblem coloringProblem;
    coloringProblem.ReadFile(baseTestPath + filename);

    size_t res = 999;
    auto time = 0.0;
    for (auto i : sorts) {
      coloringProblem.GreedyColoring(i);
      res = std::min(res, coloringProblem.getMaxColor());
      time += coloringProblem.getTime();
      coloringProblem.resetResults();
    }

    for (int i = 0; i < 100; ++i) {
      coloringProblem.GreedyColoring(SortOrder::DESC_SHUFFLE);
      res = std::min(res, coloringProblem.getMaxColor());
      time += coloringProblem.getTime();
      coloringProblem.resetResults();
    }

    for (int i = 0; i < 70; ++i) {
      coloringProblem.GreedyColoring(SortOrder::ASC_SHUFFLE);
      res = std::min(res, coloringProblem.getMaxColor());
      time += coloringProblem.getTime();
      coloringProblem.resetResults();
    }

    for (int i = 0; i < 100; ++i) {
      coloringProblem.GreedyColoring(SortOrder::SHUFFLE);
      res = std::min(res, coloringProblem.getMaxColor());
      time += coloringProblem.getTime();
      coloringProblem.resetResults();
    }
    output << filename << ";" << res << ";" << time << "\n";
  }
  return 0;
}
