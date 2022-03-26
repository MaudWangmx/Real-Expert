#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <queue>
#include <map>
#include <algorithm>
#include <cmath>

using namespace std;


typedef struct{
    int id;
    string site_name;
    int bandwidth;
    int qos_num;
    int bandwidth_95; //各个节点95%的值 可以放进site里面 初始为带宽*1 迭代完一轮后排序得到
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
int demandSum; // 某一时刻所有用户需求之和

// 判题器所用变量
vector<vector<info>> *best_info; //最佳输出（初始为空）
int best_ans =  0x3f3f3f3f; //最佳分数（初始为最大值）
//vector<int> num_5; //可填满的数量（第一遍迭代为T or 0.05T?）
vector<vector<int>> this_time_use;
map<string,int> sites_name_to_index;


int* site_full;
int* site_used;


string datafile_root = "/";


// this_time_result为所有客户某一时刻的分配方案
void sum_before_judge(vector<info> this_time_result[]){
    vector<int> sites_used(site_num, 0);

    for (int i=0; i<customer_num; i++){
        int length = this_time_result[i].size();
        for (int j=0; j<length; j++){
            int index = sites_name_to_index[this_time_result[i][j].site_name_from];
            sites_used[index] += this_time_result[i][j].bandwidth;
        }
    }
//    for ( int i=0; i<site_num; i++){
//        cout << sites_used[i] << " ";
//    }
//    cout << endl;
    this_time_use.push_back(sites_used);
}

int judge(){
//     for (int i=0; i<T; i++){
//         for(int j=0; j<site_num; j++){
//             cout << this_time_use[i][j] << " ";
//         }
//         cout << endl;
//     }
    int sum_score = 0;
    int time_95 = ceil((double)T * 0.95) - 1;
    int bandwidth_in_time[T]; // 某边缘节点每一时刻使用带宽序列 声明？
    for (int j=0; j<site_num; j++){
        for (int i=0; i<T; i++){
            bandwidth_in_time[i] = this_time_use[i][j];
        }
        sort(bandwidth_in_time, bandwidth_in_time+T);
        sum_score += bandwidth_in_time[time_95];
        sites[j].bandwidth_95 = bandwidth_in_time[time_95];
        //cout << bandwidth_in_time[time_95] << " ";
    }
    cout << sum_score << endl;
    return sum_score;
}

bool bandwidth95Cmp(int x, int y){
    return ((double)sites[x].bandwidth_95/sites[x].bandwidth) > ((double)sites[y].bandwidth_95/sites[y].bandwidth);
}

void getSiteOrderby95(vector<int> &sitesOrder){
    stable_sort(sitesOrder.begin(), sitesOrder.end(), bandwidth95Cmp);
}

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

void check(vector<int> &nodes, int t){
    static ofstream outfile(datafile_root + "output/check.txt");
    for (int i = 0; i < site_num; i++){
        outfile << "[" << nodes[i] << "]"<< (float)site_used_bandwidth[nodes[i]] / sites[nodes[i]].bandwidth << " (" << site_used_times[nodes[i]] << ")      ";
    }
    outfile << endl << endl;
    outfile << t << endl;
}

bool siteUsedTimeCmp(int x, int y){
    //cout << x << " " << y << endl;
    return (sites[x].qos_num == 0 || sites[y].qos_num == 0 || (sites[x].bandwidth/sites[x].qos_num > sites[y].bandwidth/sites[y].qos_num));
}

// 95策略：每一时刻初始根据site_used_times数组排序，其中使用次数未达timeP次的排在前面
void sortBetweenTimes(vector<int> &nodes, int t){
    for ( int i=0; i<site_num-1; i++){
        int min_ind = i;
        for (int j=i+1; j<site_num; j++){
            if(siteUsedTimeCmp(j,min_ind))
                min_ind = j;
        }
        int t = nodes[i];
        nodes[i] = nodes[min_ind];
        nodes[min_ind] = t;

    }
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
        new_site.bandwidth_95 = new_site.bandwidth+1;
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

//    for (int i=0; i<site_num; i++){
//        num_5.push_back(T);
//    }
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
        sites_name_to_index[vect[t+1][0]]= idx_t;
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
    //sort_cust(customers, customer_num);
    // for (int i=0; i<customer_num; i++){
    //     qsort_site(customers[i].qos, 0, int(customers[i].qos.size())-1, sites);
    // }

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
int dinic(int t, map<int, int> &infoMap, vector<int>& sitesOrder, double threshold){
    // 每一时刻初始化图
    memset(graphic, 0, sizeof(graphic));
    numP = site_num + customer_num + 2; // 客户：1 ~ customer_num (id + 1)
    // 边缘节点：customer_num + 1 ~ customer_num + site_num (customer_num + id + 1)
    for(int i = 0; i < customer_num; i++){
        int u = i + 1; // 客户表示的点在图中的标号
        graphic[0][u] = customers[i].bandwidth_need[t]; // +=
        demandSum += customers[i].bandwidth_need[t];
        for(int j = 0; j < customers[i].qos.size(); j++){
            int v = customer_num + customers[i].qos[j] + 1;
            int w = min(customers[i].bandwidth_need[t], sites[customers[i].qos[j]].bandwidth);
            graphic[u][v] = w; // +=
        }
    }
    for(int i = 0; i < site_num; i++){ // 边缘节点连接到汇点
        int u = customer_num + i + 1; // 边缘节点表示的点在图中的标号
        int w = sites[i].bandwidth;
        if(site_used_times[i] >= timeP){
            w = threshold * w;
        }
        graphic[u][numP - 1] = w;
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


int rearrangeDinic(int t, map<int, int> &infoMap, vector<int>& sitesOrder, int mode, int max_use, double threshold){
    // 每一时刻初始化图
    memset(graphic, 0, sizeof(graphic));
    numP = site_num + customer_num + 2; // 客户：1 ~ customer_num (id + 1)
    // 边缘节点：customer_num + 1 ~ customer_num + site_num (customer_num + id + 1)
    for(int i = 0; i < customer_num; i++){
        int u = i + 1; // 客户表示的点在图中的标号
        graphic[0][u] = customers[i].bandwidth_need[t]; // +=
        demandSum += customers[i].bandwidth_need[t];
        for(int j = 0; j < customers[i].qos.size(); j++){
            int v = customer_num + customers[i].qos[j] + 1;
            int w = min(customers[i].bandwidth_need[t], sites[customers[i].qos[j]].bandwidth);
            graphic[u][v] = w; // +=
        }
    }
    //cout << t << endl;
    for(int i = 0; i < site_num; i++){ // 边缘节点连接到汇点
        int u = customer_num + sitesOrder[i] + 1; // 边缘节点表示的点在图中的标号
        int w = sites[sitesOrder[i]].bandwidth;


//        if (site_used[t] > mode){
//            if (i >= mode && i < max_use)
//                w = 0;
//        }
//        if(i >= max_use && site_used_times[i] >= timeP){
//            w = sites[i].bandwidth_95;
//        }



        if (i >= mode && site_used_times[sitesOrder[i]] >= timeP)
            w = 0;

        if(site_used_times[sitesOrder[i]] >= timeP){
            w = threshold * w;
        }
        graphic[u][numP - 1] = w;
        //cout << w/sites[sitesOrder[i]].bandwidth << " ";
    }
    //cout<< endl;

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
int dinicAlgorithm(double threshold){
    memset(site_used_times, 0, site_num * sizeof(int));

    for(int i = 0; i < customer_num; i++){
        customers[i].infos.clear();
        customers[i].infos.resize(T);
    }

    for(int t = 0; t < T; t++){
        map<int, int> infoMap; // 当前时刻分配方案
        vector<info> this_time_result[customer_num + 5];
        memset(site_used_bandwidth, 0, site_num * sizeof(int)); // 初始化当前时刻边缘节点已用带宽
        demandSum = 0;

        // 每一时刻选取边缘节点的顺序
        vector<int> sitesOrder;
        for(int i = 0; i < site_num; i++){
            sitesOrder.push_back(i);
        }
        //sortBetweenTimes(sitesOrder, t);

        int ans = dinic(t, infoMap, sitesOrder, threshold);
        if(ans != demandSum){
            cout << "ERROR! 参数不合法！";
            return -1;
        }

        vector<int> usedFlags(site_num, 0);
        // 处理输出
        for(auto & it : infoMap) {
//            cout << it.first << " " << it.second << "\n";
            int iCus = it.first % 100;
            int iSite = it.first / 100;
            if(it.second != 0) {
                customers[iCus].infos[t].push_back({sites[iSite].site_name, it.second});
                this_time_result[iCus].push_back({sites[iSite].site_name, it.second});
            }
        }
        for(int i = 0; i < site_num; i++){
            if(site_used_bandwidth[i]>= sites[i].bandwidth_95){
                site_used_times[i]++;
            }
            if ((float)site_used_bandwidth[i] / sites[i].bandwidth == 1.0)
                site_full[t] += 1;
            if (site_used_bandwidth[i] > 0)
                site_used[t] += 1;
        }
        sum_before_judge(this_time_result);

    }
    return judge();
}




int rearrangeHighCost(vector<int> &sitesOrder, int mode, int max_use, double threshold){
    memset(site_used_times, 0, site_num * sizeof(int));

    for(int i = 0; i < customer_num; i++){
        customers[i].infos.clear();
        customers[i].infos.resize(T);
    }

    for(int t = 0; t < T; t++){
        map<int, int> infoMap; // 当前时刻分配方案
        vector<info> this_time_result[customer_num + 5];
        memset(site_used_bandwidth, 0, site_num * sizeof(int)); // 初始化当前时刻边缘节点已用带宽
        demandSum = 0;

        //sortBetweenTimes(sitesOrder, t);

        int ans = rearrangeDinic(t, infoMap, sitesOrder, mode, max_use, threshold);
        if(ans != demandSum){
            cout << "ERROR! 参数不合法！";
            return -1;
        }

        vector<int> usedFlags(site_num, 0);
        // 处理输出
        for(auto & it : infoMap) {
//            cout << it.first << " " << it.second << "\n";
            int iCus = it.first % 100;
            int iSite = it.first / 100;
            if(it.second != 0) {
                customers[iCus].infos[t].push_back({sites[iSite].site_name, it.second});
                this_time_result[iCus].push_back({sites[iSite].site_name, it.second});
            }
        }
        for(int i = 0; i < site_num; i++){
            if(site_used_bandwidth[i]>= sites[i].bandwidth_95 && site_used_bandwidth[i] != 0){
                site_used_times[i]++;
            }
        }
        sum_before_judge(this_time_result);

    }
    return judge();
}



void rearrangeHighCostAlgorithm(){
    site_used_bandwidth = new int[site_num];
    site_used_times = new int[site_num];
    best_info = new vector<vector<info>>[customer_num];
    site_full = new int[T];
    site_used = new int[T];

    timeP = (int) (T * 0.05) ;
    memset(site_full, 0, T * sizeof(int));
    memset(site_used, 0, T * sizeof(int));

    best_ans = dinicAlgorithm(1.0);
    for(int i = 0; i < customer_num; i++){
        best_info[i] = customers[i].infos;
    }
    cout << best_ans;

    vector<int> sitesOrder;
    for(int i = 0; i < site_num; i++){
        sitesOrder.push_back(i);
    }
    getSiteOrderby95(sitesOrder);
    int max = 0 , mode = 0, max_use = 0;
    int* modeUse = new int[site_num];
    memset(modeUse, 0, site_num * sizeof(int));
    for (int i = 0; i < T; i++){
        if (site_full[i] > max)
            max = site_full[i];
        if (site_used[i] > max_use)
            max_use = site_used[i];
        modeUse[site_full[i]]++;
    }
    int mode_num = 0;
    for (int i = 0; i <= max; i++){
        if (mode_num <= modeUse[i]) {
            mode = i;
            mode_num = modeUse[i];
        }
        //cout << modeUse[i] <<  " ";
    }
//    cout << endl;
//    cout <<mode << endl;
// cout << max << " " << max_use << endl;

    rearrangeHighCost(sitesOrder,mode, max_use, 1.0);
    int best_mode = max_use;
    for(int i = max_use; i > 0; i--){
        //getSiteOrderby95(sitesOrder);
        this_time_use.clear();

        int ans = rearrangeHighCost(sitesOrder, i, max_use, 1.0);

        if(ans == -1){ // threshold = 1.0 不可能出现该情况
            break;
        }

        if(ans < best_ans){
            best_mode = i;
            best_ans = ans;
            for(int i = 0; i < customer_num; i++){
                best_info[i] = customers[i].infos;
            }
        }
    }
    cout << best_mode << endl;
    for(double threshold = 1.0; threshold > 0.2; threshold -= 0.002){
        this_time_use.clear();

        int ans = rearrangeHighCost(sitesOrder, best_mode, max_use, threshold);

        if(ans == -1){ // threshold = 1.0 不可能出现该情况
            break;
        }

        if(ans < best_ans){
            best_ans = ans;
            for(int i = 0; i < customer_num; i++){
                best_info[i] = customers[i].infos;
            }
        }
    }
    for(int i = 0; i < customer_num; i++) {
        customers[i].infos = best_info[i];
    }

}
void iterativeDinicAlgorithm(){
    site_used_bandwidth = new int[site_num];
    site_used_times = new int[site_num];
    best_info = new vector<vector<info>>[customer_num];

    timeP = (int) (T * 0.05) ;

    for(double threshold = 1.0; threshold > 0.2; threshold -= 0.02){
        this_time_use.clear();

        int ans = dinicAlgorithm(threshold);

        if(ans == -1){ // threshold = 1.0 不可能出现该情况
            break;
        }

        if(ans < best_ans){
            best_ans = ans;
            for(int i = 0; i < customer_num; i++){
                best_info[i] = customers[i].infos;
            }
        }
    }

    for(int i = 0; i < customer_num; i++){
        customers[i].infos = best_info[i];
    }
}

int main(){
    initializeData();
    rearrangeHighCostAlgorithm();
    //iterativeDinicAlgorithm();

//    site_used_bandwidth = new int[site_num];
//    site_used_times = new int[site_num];
//    best_info = new vector<vector<info>>[customer_num];
//    site_full = new int[T];
//    site_used = new int[T];
//
//    timeP = (int) (T * 0.05) ;
//    memset(site_full, 0, T * sizeof(int));
//    memset(site_used, 0, T * sizeof(int));

//    dinicAlgorithm(1.0);
    outputRes();
    return 0;
}
