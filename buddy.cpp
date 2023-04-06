#include <vector>
#include <iostream>
#include <cmath>
#include <fstream>
#include <string>
#include <cstring>
#include <chrono>
#include <algorithm>

using namespace std;

chrono::high_resolution_clock::time_point start_time;

class memblock
{
public:
    bool isfree;
    string P;

    chrono::high_resolution_clock::time_point edited;

    unsigned long time_created;

    int U;
    int L;

    int start;
    int end;

    int occupied_size;

    memblock *left;
    memblock *right;

    int size();
    memblock(int U, int L, int addr_s, int addr_e);
    ~memblock();
};

memblock::memblock(int U, int L, int addr_s, int addr_e)
{
    this->edited = chrono::high_resolution_clock::now();
    this->time_created = chrono::duration_cast<chrono::nanoseconds>(this->edited - start_time).count();
    this->L = L;
    this->U = U;
    this->start = addr_s;
    this->end = addr_e;
    this->isfree = true;
    this->left = NULL;
    this->right = NULL;
}

memblock::~memblock()
{
}

int memblock::size()
{
    return this->end - this->start;
}

class testcase
{
public:
    int U;
    int L;

    vector<pair<string, int>> Requests;
};
vector<testcase> T;

void parseinput(string input)
{
    int N;
    ifstream reqfile(input);

    char *buffer = (char *)malloc(sizeof(char) * 100);

    reqfile.getline(buffer, 100); // contains num testcases
    reqfile.getline(buffer, 100); // contains empty space

    while (!reqfile.eof())
    {
        reqfile.getline(buffer, 100); // contains U, L

        char *P = strtok(buffer, " ");
        char *req = strtok(NULL, " ");

        int U = atoi(P);
        int L = atoi(req);

        testcase t;
        t.U = U;
        t.L = L;

        reqfile.getline(buffer, 100); // contains empty
        buffer[0] = 'a';

        while (strlen(buffer) != 0)
        {
            reqfile.getline(buffer, 100); // contains P:S or emptyline
            char *P = strtok(buffer, " ");
            char *req = strtok(NULL, " ");

            if (P)
            {
                string s(P);

                pair<string, int> request;
                request.first = s;
                request.second = atoi(req);

                t.Requests.push_back(request);
            }
        }

        T.push_back(t);
    }
}

void find_bestfit_or_split(pair<string, int> &req, memblock *mem, vector<memblock *> &bestfit, vector<memblock *> &bestsplit)
{

    if (mem->right == NULL && mem->left == NULL)
    {
        int bsize = mem->size();

        if (bsize / 2 < req.second && req.second <= bsize && mem->isfree)
        {
            bestfit.push_back(mem);
            if (req.first == "E")
            {
            }
            return;
        }

        if (bsize >= req.second && mem->isfree)
        {
            // cout << "req is " << req.first << "," << req.second << " block is " << mem->start << "," << mem->end << "\n";
            bestsplit.push_back(mem);
        }

        return;
    }

    if (mem->left != NULL)
    {
        find_bestfit_or_split(req, mem->left, bestfit, bestsplit);
    }

    if (mem->right != NULL)
    {
        find_bestfit_or_split(req, mem->right, bestfit, bestsplit);
    }

    return;
}

void split_alloc(pair<string, int> &req, memblock *mem)
{

    int bsize = mem->size();

    if (bsize <= (int)pow(2, mem->L))
    {
        mem->P = req.first;
        mem->isfree = false;
        mem->occupied_size = req.second;
        return;
    }

    if ((bsize / 2 < req.second && req.second <= bsize))
    {
        mem->P = req.first;
        mem->isfree = false;
        mem->occupied_size = req.second;
        return;
    }

    int mid = (int)((mem->start + mem->end) / 2);
    mem->isfree = false;
    mem->left = new memblock(mem->U, mem->L, mem->start, mid);
    mem->right = new memblock(mem->U, mem->L, mid, mem->end);

    split_alloc(req, mem->left);
}

void buddy_allocate(pair<string, int> &req, memblock *mem)
{
    vector<memblock *> bestfit_vec;
    vector<memblock *> bestsplit_vec;

    memblock *bestfit = NULL;
    memblock *bestsplit = NULL;

    find_bestfit_or_split(req, mem, bestfit_vec, bestsplit_vec);

    if (bestfit_vec.size() != 0)
    {
        for (int i = 0; i < bestfit_vec.size(); i++)
        {
            memblock *prev = bestfit;
            bestfit = bestfit_vec[i];

            if (prev != NULL)
            {
                if (prev->time_created > bestfit->time_created)
                {
                    bestfit = prev;
                }
            }
        }

        bestfit->P = req.first;
        bestfit->isfree = false;
        bestfit->occupied_size = req.second;
    }
    else
    {
        sort(bestsplit_vec.begin(), bestsplit_vec.end(), [](memblock *a, memblock *b)
             { return (a->time_created < b->time_created); });
        int sz = bestsplit_vec[0]->size();

        for (int i = bestsplit_vec.size() - 1; i >= 0; i--)
        {
            if (bestsplit_vec[i]->size() > sz)
            {
                bestsplit_vec.pop_back();
            }
            else
            {
                break;
            }
        }

        for (int i = 0; i < bestsplit_vec.size(); i++)
        {
            memblock *prev_split = bestsplit;
            bestsplit = bestsplit_vec[i];

            if (prev_split != NULL)
            {
                if (prev_split->time_created > bestsplit->time_created)
                {
                    bestsplit = prev_split;
                }
            }
        }

        split_alloc(req, bestsplit);
    }
}

void recursive_free(pair<string, int> &req, memblock *mem)
{

    if (mem->right == NULL && mem->left == NULL)
    {
        if (mem->P == req.first)
        {
            mem->P = "";
            mem->isfree = true;
            mem->edited = chrono::high_resolution_clock::now();
            mem->time_created = chrono::duration_cast<chrono::nanoseconds>(mem->edited - start_time).count();
        }

        return;
    }

    if (mem->left != NULL)
    {
        recursive_free(req, mem->left);
    }

    if (mem->right != NULL)
    {
        recursive_free(req, mem->right);
    }

    return;
}

void recursive_merge(memblock *mem)
{

    if (mem->left != NULL)
    {
        recursive_merge(mem->left);
    }

    if (mem->right != NULL)
    {
        recursive_merge(mem->right);
    }

    if (mem->left != NULL && mem->right != NULL)
    {
        if (mem->left->isfree && mem->right->isfree)
        {
            delete mem->left;
            delete mem->right;
            mem->left = NULL;
            mem->right = NULL;
            mem->isfree = true;
            mem->P = "";

            mem->edited = chrono::high_resolution_clock::now();
            mem->time_created = chrono::duration_cast<chrono::nanoseconds>(mem->edited - start_time).count();
        }
    }

    return;
}

void buddy_deallocate(pair<string, int> &req, memblock *mem)
{

    // free the blocks with mem.P == req.first
    recursive_free(req, mem);

    // merge the children into parents.
    recursive_merge(mem);
}

void print_mem(memblock *mem)
{

    if (mem->left == NULL && mem->right == NULL)
    {
        if (mem->P == "")
        {
            cout << "Free Block: " << mem->size() << "\n";
        }
        else
        {
            cout << mem->P << ": " << mem->occupied_size << "\n";
        }

        return;
    }

    if (mem->left != NULL)
    {
        print_mem(mem->left);
    }

    if (mem->right != NULL)
    {
        print_mem(mem->right);
    }
}

void simulate_testcase(testcase &test)
{
    memblock *mem_root = new memblock(test.U, test.L, 0, (int)pow(2, test.U));

    for (pair<string, int> req : test.Requests)
    {

        if (req.second == 0)
        {
            buddy_deallocate(req, mem_root);
        }
        else
        {
            buddy_allocate(req, mem_root);
        }
        int x = 5;
    }

    print_mem(mem_root);
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        cout << "Error usage ./buddy <path to input file>"
             << "\n";
        cout << "Only accepting input through file"
             << "\n";
        exit(0);
    }

    string input(argv[1]);
    parseinput(input);

    // cout << "Input parsed \n";

    start_time = chrono::high_resolution_clock::now();

    int i = 1;
    for (testcase test : T)
    {
        // cout << "Started case " << i << " U,L " << test.U << ", " << test.L << "\n";
        simulate_testcase(test);
        if (i != T.size())
        {
            cout << "\n";
        }

        i += 1;
    }

    return 0;
}