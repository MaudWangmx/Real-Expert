#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <string>
#include <queue>
#include <map>
#include <algorithm>

using namespace std;


typedef struct{
    int id;
    string site_name;
    int bandwidth;
    int qos_num;
    bool operator == (const string &name){
        return (this->site_name == name);
    }
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
    vector<string> qos_name;
    vector<vector<info>> infos; //分配的结果
    bool operator == (const string &name){
        return (this->customer_name == name);
    }
};


vector<site> sites; //边缘节点的集合
int site_num = 0;
vector<customer> customers; //客户的集合
int customer_num = 0;
int T = 0;

// dinic算法所需变量
int const INF = 0x3f3f3f3f;
int const MAX_P = 135 + 35 + 5;
int numP; // 点的个数
int graphic[MAX_P][MAX_P]; // 构建的图
int dep[MAX_P]; // 点所在的层数
// 优化所用变量
int* site_used_times; // 边缘节点当前使用次数（每一时刻若有客户使用该节点，则使用次数+1）
int* site_used_bandwidth; // 在某一时刻，边缘节点已使用的带宽（每时刻初始清零）
int timeP; // 计算百分之五的时间（向下取整）

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

void qsort_site(vector<int> &a, int low, int high, vector<site> &sites){
    if (high <= low) return;
    int i = low;
    int j = high;
    int key = sites[a[low]].bandwidth/sites[a[low]].qos_num;
    while(i<j){
        while (i < j && sites[a[j]].bandwidth/sites[a[j]].qos_num <= key){
            j--;
        }
        while (i < j && sites[a[i]].bandwidth/sites[a[i]].qos_num >= key){      
            i++;
        } 
        if ( i >= j) break;
        int temp = a[i];
        a[i] = a[j];
        a[j] = temp;
    }
    int temp = a[low];
    a[low] = a[j];
    a[j] = temp;
    qsort_site(a, low, j-1, sites);
    qsort_site(a,j+1, high, sites);
}

void check(vector<int> &nodes, int t){
    static ofstream outfile(datafile_root + "output/check.txt");
    for (int i = 0; i < site_num; i++){
        outfile << "[" << nodes[i] << "]"<< (float)site_used_bandwidth[nodes[i]] / sites[nodes[i]].bandwidth << " (" << site_used_times[nodes[i]] << ")      ";
    }
    outfile << endl << endl;
    outfile << t << endl;
}

bool siteUsedTimeCmp(int x, int y){
    return site_used_times[x] > site_used_times[y];
}

// 95策略：每一时刻初始根据site_used_times数组排序，其中使用次数未达timeP次的排在前面
void sortBetweenTimes(vector<int> &nodes, int t){
    int index;
    sort(nodes.begin(), nodes.end(), siteUsedTimeCmp);

    for (index = 0; index < site_num; index++){ // 遍历nodes
        if (site_used_times[nodes[index]] < timeP) // timeP - 1
            break;
    }
    if (index == site_num)
        return;
    vector<int> temp;
    for (int i = index; i < site_num; i++)
        temp.push_back(nodes[i]);
    for (int i = 0; i < index; i++)
        temp.push_back(nodes[i]);
    for (int i = 0; i < site_num; i++)
        nodes[i] = temp[i];

//    for(int i = 0; i < nodes.size(); i++){
//        cout << site_used_times[nodes[i]] << " ";
//    }
//    cout <<endl;
    check(nodes, t);

}

bool siteUsedBandwidthCmp(int x, int y){
    return site_used_bandwidth[x] > site_used_bandwidth[y];
}

// 一个失败的排序
void sortInTime(vector<int> &nodes, int t){
    int index;
    for (index = 0; index < site_num; index++){
        if (site_used_times[nodes[index]] >= timeP)
            break;
    }
    if (index != 0)
        stable_sort(nodes.begin(), nodes.begin() + index, siteUsedBandwidthCmp);
    if (index != site_num)
        stable_sort(nodes.begin() + index, nodes.end(), siteUsedBandwidthCmp);

    check(nodes, t);
}

// 读csv文件
vector<vector<string>> read_csv(string filename){
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
    vector<int> cust_index;
    vector<vector<string>> vect = read_csv(datafile_root + "data/site_bandwidth.csv");
    //遍历用 以下读取数据
    vector<string> temp_vect;
    vector<vector<string>>::iterator ite;
    vector<customer>::iterator itc;

    for (ite = ++vect.begin(); ite != vect.end(); ite++)
    {
        site new_site;
        temp_vect = *ite;
        new_site.qos_num = 0;
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
    for (int i=0; i<customer_num; i++){
        string temp_string = vect[0][i+1];
        if (temp_string[temp_string.size()-1] == '\r'){
            temp_string = temp_string.substr(0, temp_string.size()-1);
        }
        if (temp_string == customers[i].customer_name){
            cust_index.push_back(i);
        }
        else{
            vector<customer>::iterator result_index = find(customers.begin(), customers.end(), vect[0][i+1]);
            cust_index.push_back(distance(customers.begin(), result_index));
        }
    }
    for (ite = ++vect.begin(); ite != vect.end(); ite++){
        int idx_t = t;
        if (sites[t].site_name == vect[t+1][0]){
            idx_t = t;
        }
        else{
            vector<site>::iterator result_index = find(sites.begin(), sites.end(), vect[t+1][0]);
            idx_t =  distance(sites.begin(), result_index);
        }
        temp_vect = *ite;
        int c_i = 0;
        for (auto itee = ++temp_vect.begin(); itee != temp_vect.end(); itee++){
            if(str_to_int(*itee) < Qos){
                customers[cust_index[c_i]].qos.push_back(idx_t);
                sites[idx_t].qos_num++;
            }
            c_i++;
        }
       
        t++;
    }
    for (int i=0; i<customer_num; i++){
        qsort_site(customers[i].qos, 0, int(customers[i].qos.size())-1, sites);
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
// 源点在第0层，客户节点在第1层，边缘节点在第2层，汇点在第3层
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
// sitesOrder为排序后的site下标
int dinicDfs(int s, int minFlow, int t, map<int, int> &infoMap, vector<int> sitesOrder){
    if(s == t){
        return minFlow;
    }

    int tmp;
    for(int v = 0; v <= customer_num; v++){
        if(graphic[s][v] > 0 && dep[v] == dep[s] + 1 && (tmp = dinicDfs(v, min(minFlow, graphic[s][v]), t, infoMap, sitesOrder))){
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
                site_used_bandwidth[v - customer_num - 1] += tmp;
            }
            else if(v >= 1 && v <= customer_num && s >= customer_num + 1 && s <= customer_num + site_num){
                int hash = (s - customer_num - 1) * 100 + (v - 1);
                infoMap[hash] -= tmp;
                site_used_bandwidth[s - customer_num - 1] -= tmp;
                if(infoMap[hash] < 0){
                    cout << "ERROR! infoMap[hash] < 0" << endl;
                }
            }

            return tmp;
        }
    }

    // for(int v = customer_num + 1; v <= numP - 1; v++){
    for(int num = 0, v; num <= sitesOrder.size(); num++){
        if(num == sitesOrder.size()){
            v = numP - 1;
        }
        else{
            v = customer_num + 1 + sitesOrder[num];
        }
        if(graphic[s][v] > 0 && dep[v] == dep[s] + 1 && (tmp = dinicDfs(v, min(minFlow, graphic[s][v]), t, infoMap, sitesOrder))){
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
                site_used_bandwidth[v - customer_num - 1] += tmp;
            }
            else if(v >= 1 && v <= customer_num && s >= customer_num + 1 && s <= customer_num + site_num){
                int hash = (s - customer_num - 1) * 100 + (v - 1);
                infoMap[hash] -= tmp;
                site_used_bandwidth[s - customer_num - 1] -= tmp;
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
int dinic(int t, map<int, int> &infoMap, vector<int>& sitesOrder){
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

    // 算法核心
    int ans = 0, tmp;
    // 源点为0，汇点为customer_num + site_num + 1(numP - 1)
    while(dinicBfs(0, numP - 1)){
        while(true){
            // sortInTime(sitesOrder, t);
            tmp = dinicDfs(0, INF, numP - 1, infoMap, sitesOrder);
            if(tmp == 0){
                break;
            }
            ans += tmp;
        }
    }
    return ans;
}

void dinicAlgorithm(){
    site_used_times = new int[site_num];
    memset(site_used_times, 0, site_num * sizeof(int));
    timeP = (int) (T * 0.05) ;

    site_used_bandwidth = new int[site_num];

    for(int i = 0; i < customer_num; i++){
        customers[i].infos.resize(T);
    }

    for(int t = 0; t < T; t++){
        map<int, int> infoMap; // 当前时刻分配方案

        memset(site_used_bandwidth, 0, site_num * sizeof(int)); // 初始化当前时刻边缘节点已用带宽

        // 每一时刻选取边缘节点的顺序
        vector<int> sitesOrder;
        for(int i = 0; i < site_num; i++){
            sitesOrder.push_back(i);
        }
        sortBetweenTimes(sitesOrder, t);

        dinic(t, infoMap, sitesOrder);

        vector<int> usedFlags(site_num, 0);
        // 处理输出
        for(auto & it : infoMap) {
            // cout << it.first << " " << it.second << "\n";
            int iCus = it.first % 100;
            int iSite = it.first / 100;
            if(it.second != 0) {
                customers[iCus].infos[t].push_back({sites[iSite].site_name, it.second});
                usedFlags[iSite] = 1;
            }
        }
        for(int i = 0; i < site_num; i++){
            if(usedFlags[i]){
                site_used_times[i]++;
            }
        }

    }

}

int main(){
    initializeData();

    //vanillaAlgorithm();
    dinicAlgorithm();

    outputRes();
    return 0;
}
