#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <string>
#include <queue>
#include <map>

using namespace std;


typedef struct{
    int id;
    string site_name;
    int bandwidth;
} site; // 边缘节点

typedef struct{
    string site_name_from;
    int bandwidth;
} info; // 分配信息

// 客户
class customer{
public:
    customer(int id, string customer_name):id(id), customer_name(customer_name){}
    int id;
    string customer_name;
    vector<int> bandwidth_need; //需求带宽
    vector<int> qos; //需求qos
    vector<vector<info>> infos; //分配的结果
};


vector<site> sites; //边缘节点的集合
int site_num = 0;
vector<customer> customers; //客户的集合
int customer_num = 0;
int T = 0;
int Qos = 0;

// dinic算法所需变量
int const INF = 0x3f3f3f3f;
int const MAX_P = 135 + 35 + 5;
int numP; // 点的个数
int graphic[MAX_P][MAX_P]; // 所需的图
int dep[MAX_P]; // 点所在的层数

string datafile_root = "../";


int str_to_int(string s){
    int l = s.size();
    if (s[l-1] == '\r')
        l--;
    int ans = 0;
    for ( int i = 0; i<l; i++){
        ans = ans * 10 + s[i]-'0';
    }
    return ans;
}

// 读csv文件
vector<vector<string>> read_csv(string filename)
{
    ifstream inFile(filename);
    string lineStr;
    vector<vector<string> > strArray;
    int count = 0;
    while(getline(inFile,lineStr))
    {
        count++;
        stringstream ss(lineStr);
        string str;
        vector<string> lineArray;
        while(getline(ss,str,','))
        {
            lineArray.push_back(str);
        }
        strArray.push_back(lineArray);
    }
    return strArray;
}

// 读取Qos
int read_qos(string filename){
    ifstream inFile(filename);
    string lineStr;

    getline(inFile,lineStr);
    getline(inFile,lineStr);
    return str_to_int(lineStr.substr(15));
}

void initializeData(){
    int Qos = read_qos(datafile_root + "data/config.ini");

    vector<vector<string>> vect = read_csv(datafile_root + "data/site_bandwidth.csv");
    //遍历用 以下读取数据
    vector<string> temp_vect;
    vector<vector<string>>::iterator ite;
    vector<customer>::iterator itc;

    for (ite = ++vect.begin(); ite != vect.end(); ite++)
    {
        site new_site;
        temp_vect = *ite;
        new_site.id = site_num++;
        new_site.site_name = temp_vect[0];
        new_site.bandwidth = str_to_int(temp_vect[1]);
        sites.push_back(new_site);
    }
    vect = read_csv(datafile_root + "data/demand.csv");
    ite = vect.begin();
    temp_vect = *ite;


    for (auto itee = ++temp_vect.begin(); itee != temp_vect.end(); itee++){
        string s_t = *itee;
        int l_t = s_t.size();
        if (s_t[l_t-1] == '\r'){
            s_t = s_t.substr(0,l_t-1);
        }
        customers.push_back(customer(customer_num++, s_t));
    }
    for (ite = ++vect.begin(); ite != vect.end(); ite++)
    {
        temp_vect = *ite;
        itc = customers.begin();
        T++;
        for (auto itee = ++temp_vect.begin(); itee != temp_vect.end(); itee++){
            itc->bandwidth_need.push_back(str_to_int(*itee));
            itc++;
        }
    }
    vect = read_csv(datafile_root + "data/qos.csv");
    int t = 0;
    for (ite = ++vect.begin(); ite != vect.end(); ite++){
        temp_vect = *ite;
        itc = customers.begin();
        for (auto itee = ++temp_vect.begin(); itee != temp_vect.end(); itee++){
            if(str_to_int(*itee) < Qos){
                itc->qos.push_back(t);
            }
            // cout << str_to_int(*itee) << endl;
            itc++;
        }
        t++;

    }
}

void outputRes(){
    ofstream outfile(datafile_root + "output/solution.txt");
    for (int t = 0; t < T; t++){
        for (int i = 0; i < customer_num; i++){
            outfile << customers[i].customer_name << ":";
            int n = customers[i].infos[t].size();
            if (n == 0) {
                outfile << endl;
                continue;
            }
            int j = 0;
            for (; j < n - 1; j++)
                outfile << "<" << customers[i].infos[t][j].site_name_from << "," << customers[i].infos[t][j].bandwidth << ">" << ",";
            outfile << "<" << customers[i].infos[t][j].site_name_from << "," << customers[i].infos[t][j].bandwidth << ">";
            outfile << endl;
        }
    }
}

void vanillaAlgorithm(){
    for (int time_i = 0; time_i<T; time_i++){
        vector<int> bandwidth_cust_need;
        vector<int> bandwidth_site_left;
        for (int i=0; i<customer_num; i++){
            bandwidth_cust_need.push_back(customers[i].bandwidth_need[time_i]);
        }
        for ( int i=0; i<site_num; i++){
            bandwidth_site_left.push_back(sites[i].bandwidth);
        }
        for ( int i=0; i<customer_num; i++){
            vector<info> temp_info;
            int n = customers[i].qos.size();
            for ( int j=0; j<n; j++){
                int temp_qos = customers[i].qos[j];
                if (bandwidth_cust_need[i] <= bandwidth_site_left[temp_qos]){
                    bandwidth_site_left[temp_qos] -= bandwidth_cust_need[i];
                    temp_info.push_back({sites[temp_qos].site_name, bandwidth_cust_need[i]});
                    break;
                }
                else if(bandwidth_site_left[temp_qos] != 0){
                    bandwidth_cust_need[i] -=bandwidth_site_left[temp_qos];
                    temp_info.push_back({sites[temp_qos].site_name, bandwidth_site_left[temp_qos]});
                    bandwidth_site_left[temp_qos] = 0;
                }
            }
            customers[i].infos.push_back(temp_info);
        }
    }
}

// 重新按层次建图
// s为源点，t为汇点
int dinicBfs(int s, int t){
    queue<int> q;
    while(!q.empty()){
        q.pop();
    }
    memset(dep, -1, sizeof(dep));
    dep[s] = 0; // 源点在第0层
    q.push(s);
    while(!q.empty()){
        int u = q.front();
        q.pop();
        for(int v = 0; v <= numP - 1; v++){
            if(graphic[u][v] > 0 && dep[v] == -1){// 如果点u可以到达点v，且点v还未被访问
                dep[v] = dep[u] + 1;
                q.push(v);
            }
        }
    }
    return dep[t] != -1; // 返回能否达到汇点
}

// 查找路径上的最小流量
int dinicDfs(int s, int minFlow, int t, map<int, int> &infoMap){
    if(s == t){
        return minFlow;
    }
    int tmp;
    for(int v = 0; v <= numP - 1; v++){
        if(graphic[s][v] > 0 && dep[v] == dep[s] + 1 && (tmp = dinicDfs(v, min(minFlow, graphic[s][v]), t, infoMap))){
            graphic[s][v] -= tmp;
            graphic[v][s] += tmp;

            if(s >= 1 && s <= customer_num && v >= customer_num + 1 && v <= customer_num + site_num){
                int hash = (v - customer_num - 1) * 100 + (s - 1);
                if(infoMap.count(hash) == 0 || infoMap[hash] == 0){
                    infoMap[hash] = tmp;
                }
                else{
                    infoMap[hash] += tmp;
                }
            }
            else if(v >= 1 && v <= customer_num && s >= customer_num + 1 && s <= customer_num + site_num){
                int hash = (s - customer_num - 1) * 100 + (v - 1);
                infoMap[hash] -= tmp;
                if(infoMap[hash] < 0){
                    cout << "ERROR! infoMap[hash] < 0" << endl;
                }
            }

            return tmp;
        }
    }
    return 0;
}

// t为时刻
int dinic(int t, map<int, int> &infoMap){
    // 每一时刻初始化图
    memset(graphic, 0, sizeof(graphic));
    numP = site_num + customer_num + 2; // 客户：1 ~ customer_num (id + 1)
                                        // 边缘节点：customer_num + 1 ~ customer_num + site_num (customer_num + id + 1)
    for(int i = 0; i < customer_num; i++){
        int u = i + 1; // 客户表示的点在图中的标号
        graphic[0][u] = customers[i].bandwidth_need[t]; // +=
        for(int j = 0; j < customers[i].qos.size(); j++){
            int v = customer_num + customers[i].qos[j] + 1;
            int w = min(customers[i].bandwidth_need[t], sites[customers[i].qos[j]].bandwidth);
            graphic[u][v] = w; // +=
        }
    }
    for(int i = 0; i < site_num; i++){
        int u = customer_num + i + 1;
        graphic[u][numP - 1] = sites[i].bandwidth;
    }

    int ans = 0, tmp;
    // 源点为0，汇点为customer_num + site_num + 1(numP - 1)
    while(dinicBfs(0, numP - 1)){
        while(true){
            tmp = dinicDfs(0, INF, numP - 1, infoMap);
            if(tmp == 0){
                break;
            }
            ans += tmp;
        }
    }
    return ans;
}

void dinicAlgorithm(){
    for(int i = 0; i < customer_num; i++){
        customers[i].infos.resize(T);
    }
    for(int t = 0; t < T; t++){
        map<int, int> infoMap;
        dinic(t, infoMap);

        // 处理输出
        for(auto & it : infoMap) {
            // cout << it.first << " " << it.second << "\n";
            int iCus = it.first % 100;
            int iSite = it.first / 100;
            if(it.second != 0) {
                customers[iCus].infos[t].push_back({sites[iSite].site_name, it.second});
            }
        }

    }

}

int main(){
    initializeData();

    // vanillaAlgorithm();
    dinicAlgorithm();

    outputRes();
    return 0;
}
