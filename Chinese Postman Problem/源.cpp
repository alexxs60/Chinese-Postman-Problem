#include <iostream>
#include <vector>
#include <stack>
#include <climits>
#include <algorithm>
#include <fstream>
#include <set>

using namespace std;

const int MAX_NODES = 105;
const int INF = INT_MAX / 2;

vector<vector<int>> adjMatrix(MAX_NODES, vector<int>(MAX_NODES, INF));
vector<vector<int>> parent(MAX_NODES, vector<int>(MAX_NODES, -1));
vector<int> nodeDegrees(MAX_NODES, 0);
vector<vector<pair<int, int>>> multigraph(MAX_NODES);

int nodes, edges, startNode;
long long originalLength = 0;

void readGraphFromKeyboard() {
    cout << "请输入节点数、边数和起始节点：";
    cin >> nodes >> edges >> startNode;
    if (nodes <= 0 || edges < 0 || startNode < 0 || startNode >= nodes) {
        cout << "输入无效！" << endl;
        exit(1);
    }
    for (int i = 0; i < edges; i++) {
        int from, to, weight;
        cout << "请输入边 " << i + 1 << " 的两个节点和权重：";
        cin >> from >> to >> weight;
        if (from < 0 || from >= nodes || to < 0 || to >= nodes || weight <= 0) {
            cout << "边输入无效！" << endl;
            exit(1);
        }
        adjMatrix[from][to] = adjMatrix[to][from] = min(adjMatrix[from][to], weight);
        multigraph[from].push_back({ to, weight });
        multigraph[to].push_back({ from, weight });
        nodeDegrees[from]++;
        nodeDegrees[to]++;
        originalLength += weight;
    }
}

void readGraphFromFile(const string& filename) {
    ifstream file(filename);
    if (!file) {
        cout << "文件打开失败！" << endl;
        exit(1);
    }
    file >> nodes >> edges >> startNode;
    if (nodes <= 0 || edges < 0 || startNode < 0 || startNode >= nodes) {
        cout << "文件输入无效！" << endl;
        exit(1);
    }
    for (int i = 0; i < edges; i++) {
        int from, to, weight;
        file >> from >> to >> weight;
        if (from < 0 || from >= nodes || to < 0 || to >= nodes || weight <= 0) {
            cout << "边输入无效！" << endl;
            exit(1);
        }
        adjMatrix[from][to] = adjMatrix[to][from] = min(adjMatrix[from][to], weight);
        multigraph[from].push_back({ to, weight });
        multigraph[to].push_back({ from, weight });
        nodeDegrees[from]++;
        nodeDegrees[to]++;
        originalLength += weight;
    }
    file.close();
}

void chooseInputMethod() {
    int choice;
    cout << "请选择输入方式（1. 文件输入，2. 键盘输入）：";
    cin >> choice;
    if (choice == 1) {
        string filename;
        cout << "请输入文件名：";
        cin >> filename;
        readGraphFromFile(filename);
    }
    else if (choice == 2) {
        readGraphFromKeyboard();
    }
    else {
        cout << "选择无效！" << endl;
        exit(1);
    }
}

void printGraph() {
    cout << "\n节点数: " << nodes << "  边数: " << edges << "  起始: " << startNode << endl;
    cout << "原总权重: " << originalLength << endl;
}

void floydWarshall() {
    // 初始化对角线为0
    for (int i = 0; i < nodes; i++) {
        adjMatrix[i][i] = 0;
        parent[i][i] = i;
    }
    for (int i = 0; i < nodes; i++) {
        for (int j = 0; j < nodes; j++) {
            if (adjMatrix[i][j] < INF) parent[i][j] = j;
        }
    }
    for (int k = 0; k < nodes; k++)
        for (int i = 0; i < nodes; i++)
            for (int j = 0; j < nodes; j++)
                if (adjMatrix[i][k] < INF && adjMatrix[k][j] < INF &&
                    adjMatrix[i][j] > adjMatrix[i][k] + adjMatrix[k][j]) {
                    adjMatrix[i][j] = adjMatrix[i][k] + adjMatrix[k][j];
                    parent[i][j] = parent[i][k];
                }
}

vector<int> getShortestPath(int u, int v) {
    vector<int> path;
    if (u == v) {
        path.push_back(u);
        return path;
    }
    if (parent[u][v] == -1) return path;
    int cur = u;
    while (cur != v) {
        path.push_back(cur);
        cur = parent[cur][v];
        if (cur == -1) return vector<int>();
    }
    path.push_back(v);
    return path;
}

vector<int> getOddDegreeNodes() {
    vector<int> odds;
    for (int i = 0; i < nodes; i++)
        if (nodeDegrees[i] % 2 == 1) odds.push_back(i);
    return odds;
}

void dfsMatch(int idx, const vector<int>& odds, const vector<vector<int>>& dist,
    vector<bool>& used, long long cur_cost, vector<pair<int, int>>& cur_pairs,
    long long& min_cost, vector<pair<int, int>>& best_pairs) {
    if (idx == (int)odds.size()) {
        if (cur_cost < min_cost) {
            min_cost = cur_cost;
            best_pairs = cur_pairs;
        }
        return;
    }
    if (used[idx]) {
        dfsMatch(idx + 1, odds, dist, used, cur_cost, cur_pairs, min_cost, best_pairs);
        return;
    }
    for (int j = idx + 1; j < (int)odds.size(); ++j) {
        if (!used[j] && dist[odds[idx]][odds[j]] < INF) {
            used[idx] = used[j] = true;
            cur_pairs.push_back(make_pair(odds[idx], odds[j]));
            dfsMatch(idx + 1, odds, dist, used, cur_cost + dist[odds[idx]][odds[j]], cur_pairs, min_cost, best_pairs);
            cur_pairs.pop_back();
            used[idx] = used[j] = false;
        }
    }
}

long long minimumMatching(const vector<int>& odds, vector<pair<int, int>>& best_pairs) {
    int n = odds.size();
    if (n == 0) return 0;
    if (n % 2 != 0) return -1;

    vector<vector<int>> dist = adjMatrix;  // 复制一份
    long long min_cost = LLONG_MAX / 2;
    vector<bool> used(n, false);
    vector<pair<int, int>> cur_pairs;
    dfsMatch(0, odds, dist, used, 0LL, cur_pairs, min_cost, best_pairs);
    return (min_cost >= INF) ? -1LL : min_cost;
}

void addDuplicateEdges(const vector<pair<int, int>>& pairs) {
    for (auto& p : pairs) {
        int u = p.first, v = p.second;
        vector<int> path = getShortestPath(u, v);
        if (path.empty()) continue;
        for (size_t i = 02; i + 1 < path.size(); ++i) {
            int a = path[i], b = path[i + 1];
            multigraph[a].push_back({ b, adjMatrix[a][b] });
            multigraph[b].push_back({ a, adjMatrix[a][b] });
        }
    }
}

vector<int> findEulerCircuit(int start) {
    vector<int> circuit;
    stack<int> stk;
    stk.push(start);

    while (!stk.empty()) {
        int u = stk.top();
        if (multigraph[u].empty()) {
            circuit.push_back(u);
            stk.pop();
        }
        else {
            auto& edges = multigraph[u];
            auto it = edges.begin();
            int v = it->first;
            edges.erase(it);
            // 移除反向边
            auto& rev_edges = multigraph[v];
            for (auto jt = rev_edges.begin(); jt != rev_edges.end(); ++jt) {
                if (jt->first == u) {
                    rev_edges.erase(jt);
                    break;
                }
            }
            stk.push(v);
        }
    }
    reverse(circuit.begin(), circuit.end());
    return circuit;
}

bool isGraphConnected() {
    // 从startNode出发检查所有节点可达
    for (int i = 0; i < nodes; i++) {
        if (adjMatrix[startNode][i] >= INF) return false;
    }
    return true;
}

void calculateAndPrintOptimalRoute() {
    floydWarshall();

    if (!isGraphConnected()) {
        cout << "图不连通，无法计算！" << endl;
        return;
    }

    vector<int> odds = getOddDegreeNodes();
    long long added_cost = 0;
    vector<pair<int, int>> pairs;

    if (!odds.empty()) {
        if (odds.size() > 10) {
            cout << "奇节点太多，匹配可能慢！" << endl;
        }
        added_cost = minimumMatching(odds, pairs);
        if (added_cost < 0) {
            cout << "匹配失败！" << endl;
            return;
        }
        cout << "添加成本: " << added_cost << endl;
        addDuplicateEdges(pairs);
    }

    vector<int> euler = findEulerCircuit(startNode);

    // 计算实际路径长度
    long long total_length = 0;
    for (size_t i = 0; i + 1 < euler.size(); ++i) {
        int u = euler[i], v = euler[i + 1];
        if (adjMatrix[u][v] < INF) {
            total_length += adjMatrix[u][v];
        }
        else {
            cout << "路径错误，无法计算长度！" << endl;
            return;
        }
    }

    cout << "路线: ";
    for (size_t i = 0; i < euler.size(); ++i) {
        cout << euler[i];
        if (i + 1 < euler.size()) cout << " -> ";
    }
    cout << endl;
    cout << "总路程: " << total_length << endl;
}

int main() {
    chooseInputMethod();
    printGraph();
    calculateAndPrintOptimalRoute();
    return 0;
}