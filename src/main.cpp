#define HEAD_INFO

#include "sfmt/SFMT.h"
#include "memoryusage.h"
#include "head.h"
using namespace std;
namespace fs = std::filesystem;

class Argument
{
public:
    string dataset;
    vector<string> mode;
    double T;
    int seed_random;
    int64 size;
    int numIters;
};

class Task
{
public:
    string base_dir;
    map<string, set<int>> modeWithBudgets;
    map<string, map<int, vector<int>>> modeWithBudgetsAndEpochs;
};

#include "graph.h"
#include "infgraph.h"

void readConfiguration(Argument &A, Task &T)
{
    string conf = "config";
    string base_dir;
    ifstream cin(conf.c_str());
    ASSERT(!cin == false);
    string line;
    while (cin >> line)
    {
        if (line.substr(0, 8) == "base_dir")
        {
            base_dir = line.substr(9);
        }
    }
    cin.close();
    T.base_dir = base_dir + A.dataset + "/TV/test/";
    cout << "base directory: " << T.base_dir << endl;
}

bool setTasks(string base_dir, string mode, set<int> &budgets, map<int, vector<int>> &budgetsWithEpoch)
{
    string file;
    string input_pattern;
    string output_pattern;
    string folder = base_dir;
    if (mode == "gcomb")
    {
        input_pattern = "large_graph-result_RL_(\\d+)_nbs_0.003";
        output_pattern = "large_graph_reward_RL_budget_%d_nbs_0.003";
    }
    else if (mode == "epoch_gcomb")
    {
        input_pattern = "large_graph_epoch_(\\d+)-result_RL_(\\d+)_nbs_0.003";
        output_pattern = "large_graph_epoch_%d_reward_RL_budget_%d_nbs_0.003";
    }
    else if (mode == "imm" || mode == "interp_imm")
    {
        input_pattern = "large_graph_ic_imm_sol_eps0.5_num_k_(\\d+)_iter_(\\d+).txt";
        output_pattern = "imm_influence_%d.txt";
        if (mode == "imm")
            folder += "/multi_iter/";
        if (mode == "interp_imm")
            folder += "/interp/multi_iter/";
    }
    else if (mode == "DDiscount" || mode == "SingleDiscount")
    {
        input_pattern = mode + "_budget_(\\d+).*";
        output_pattern = mode + "_reward_%d.txt";
    }
    else
    {
        ASSERT(false);
    }

    if (!fs::exists(folder))
    {
        cout << folder << " not exists." << endl;
        return false;
    }

    regex re(input_pattern);
    smatch results;
    char buff[200];
    for (const auto &entry : fs::directory_iterator(folder))
    {
        string filename = entry.path().filename().string();
        if (std::regex_search(filename, results, re))
        {
            if (mode.find("epoch") != string::npos)
            {
                int budget = atoi(results[2].str().c_str());
                int epoch = atoi(results[1].str().c_str());
                snprintf(buff, sizeof(buff), output_pattern.c_str(), epoch, budget);
                string dstFile = buff;
                if (fs::exists(folder + dstFile))
                {
                    cout << "File " << dstFile << " exists, continue" << endl;
                    continue;
                }
                if (!budgetsWithEpoch.count(budget))
                {
                    budgetsWithEpoch[budget] = vector<int>();
                }
                budgetsWithEpoch[budget].push_back(epoch);
            }
            else
            {
                int budget = atoi(results[1].str().c_str());
                snprintf(buff, sizeof(buff), output_pattern.c_str(), budget);
                string dstFile = buff;
                if (fs::exists(folder + dstFile))
                {
                    cout << "File " << dstFile << " exists, continue" << endl;
                    continue;
                }
                budgets.insert(budget);
            }
        }
    }
    return budgets.size() > 0 || budgetsWithEpoch.size() > 0;
}

bool setTasks(Argument &A, Task &T)
{
    bool hasTasks = false;
    for (string mode : A.mode)
    {
        set<int> budgets;
        map<int, vector<int>> budgetsWithEpoch;
        bool _hasTask = setTasks(T.base_dir, mode, budgets, budgetsWithEpoch);
        cout << "budgets size: " << budgets.size() << " budgetsWithEpoch size: " << budgetsWithEpoch.size() << endl;
        hasTasks = hasTasks || _hasTask;
        if (!_hasTask)
        {
            cout << A.dataset << " doesn't have mode " << mode << " to process." << endl;
            continue;
        }
        if (mode == "epoch_gcomb")
        {
            T.modeWithBudgetsAndEpochs[mode] = budgetsWithEpoch;
        }
        else
        {
            T.modeWithBudgets[mode] = budgets;
        }
    }
    return hasTasks;
}

vector<string> getFileName(string base_dir, string mode, int budget, int n_iter = 0, int epoch = 0)
{
    string inputFile;
    string outputFile;
    if (mode == "gcomb")
    {
        inputFile = "large_graph-result_RL_" + to_string(budget) + "_nbs_0.003";
        outputFile = "large_graph_reward_RL_budget_" + to_string(budget) + "_nbs_0.003";
    }
    else if (mode == "epoch_gcomb")
    {
        inputFile = "large_graph_epoch_" + to_string(epoch) + "-result_RL_" + to_string(budget) + "_nbs_0.003";
        outputFile = "large_graph_epoch_" + to_string(epoch) + "_reward_RL_budget_" + to_string(budget) + "_nbs_0.003";
    }
    else if (mode == "interp_imm")
    {
        inputFile = "/interp/multi_iter/large_graph_ic_imm_sol_eps0.5_num_k_" + to_string(budget) + "_iter_" + to_string(n_iter) + ".txt";
        outputFile = "/interp/multi_iter/imm_influence_" + to_string(budget) + ".txt";
    }
    else if (mode == "imm")
    {
        inputFile = "/multi_iter/large_graph_ic_imm_sol_eps0.5_num_k_" + to_string(budget) + "_iter_" + to_string(n_iter) + ".txt";
        outputFile = "/multi_iter/imm_influence_" + to_string(budget) + ".txt";
    }
    else if (mode == "DDiscount" || mode == "SingleDiscount")
    {
        inputFile = mode + "_budget_" + to_string(budget) + ".txt";
        inputFile = mode + "_reward_" + to_string(budget) + ".txt";
    }

    inputFile = base_dir + inputFile;
    outputFile = base_dir + outputFile;
    return {inputFile, outputFile};
}

void writeResult(string file, float coverage)
{
    ofstream outputFile;
    outputFile.open(file.c_str());
    outputFile << coverage;
    outputFile.close();
}

vector<int> readSeedSet(string file)
{
    ifstream infile((file).c_str());
    string line;
    vector<int> seeds;
    while (getline(infile, line))
    {
        int a;
        if (line.empty())
        {
            break;
        }
        a = stoi(line.substr(0, line.find("\n")));
        seeds.push_back(a);
    }
    return seeds;
}

void run_with_parameter(InfGraph &g, const Argument &arg, Task &T)
{
    cout << "--------------------------------------------------------------------------------" << endl;
    cout << "To generate " << arg.size << " RR sets" << endl;
    clock_t time_start = clock();
    g.build_hyper_graph_r(arg);
    clock_t time_end = clock();
    cout << "RR sets generated, costs: " << (float)(time_end - time_start) / CLOCKS_PER_SEC << " sec"<< endl;

    for (const auto &entry : T.modeWithBudgetsAndEpochs)
    {
        string mode = entry.first;
        for (const auto &it : entry.second)
        {
            int budget = it.first;
            for (int epoch : it.second)
            {
                vector<string> f = getFileName(T.base_dir, mode, budget, 0, epoch);
                string inputFile, outputFile;
                inputFile = f[0];
                outputFile = f[1];
                vector<int> seeds = readSeedSet(inputFile);
                float coverage = g.InfluenceHyperGraph(seeds, arg);
                cout << "mode: " << mode << " budget: " << budget << " epoch: " << epoch << " coverage: " << coverage << endl;
                writeResult(outputFile, coverage);
            }
        }
    }

    for (const auto &entry : T.modeWithBudgets)
    {
        string mode = entry.first;
        for (int budget : entry.second)
        {
            string inputFile, outputFile;
            float coverage = 0.;
            for (int i = 0; i < arg.numIters; i++)
            {
                vector<string> f = getFileName(T.base_dir, mode, budget, i);
                inputFile = f[0];
                outputFile = f[1];
                vector<int> seeds = readSeedSet(inputFile);
                coverage += g.InfluenceHyperGraph(seeds, arg);
            }
            coverage /= arg.numIters;
            cout << "mode: " << mode << " budget: " << budget << " coverage: " << coverage << endl;
            writeResult(outputFile, coverage);
        }
    }
    Timer::show();
}
void Run(int argn, char **argv)
{
    Argument arg;

    for (int i = 0; i < argn; i++)
    {
        if (argv[i] == string("-help") || argv[i] == string("--help") || argn == 1)
        {
            cout << "./imm -dataset *** -size size of RR sets -num number of iterations  -mode different modes split by comma " << endl;
            return;
        }
        if (argv[i] == string("-dataset"))
            arg.dataset = argv[i + 1];

        if (argv[i] == string("-size"))
            arg.size = atoi(argv[i + 1]);

        if (argv[i] == string("-num"))
            arg.numIters = atoi(argv[i + 1]);

        if (argv[i] == string("-mode"))
        {
            if (argv[i + 1] == string("all"))
            {
                arg.mode = {"gcomb", "epoch_gcomb", "imm", "interp_imm"}; //, "SingleDiscount", "DDiscount"};
            }
            else
            {
                istringstream in(argv[i + 1]);
                string t;
                while (getline(in, t, ','))
                {
                    arg.mode.push_back(t);
                }
            }
        }
    }
    ASSERT(arg.dataset != "");
    Task T;
    readConfiguration(arg, T);
    bool hasTasks = setTasks(arg, T);
    if (!hasTasks)
    {
        cout << "No task to execute, exit" << endl;
        return;
    }
    string graph_file;
    graph_file = T.base_dir + "edges.txt";
    InfGraph g(T.base_dir, graph_file);
    g.setInfuModel(InfGraph::IC);
    INFO(arg.T);

    run_with_parameter(g, arg, T);
}

int main(int argn, char **argv)
{
    __head_version = "v1";
    OutputInfo info(argn, argv);

    Run(argn, argv);
}
