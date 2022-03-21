#include<iostream>
#include<istream>
#include<vector>
#include<map>
#include<algorithm>
#include<sstream>
#include<fstream>
using namespace std;

// 边缘节点
typedef struct{
    int id;
    string site_name;
    int bandwidth;
    int qos_num;
} site;

// 分配信息
typedef struct{
    string site_name_from;
    int bandwidth;
} info;

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

class SubCustomer{
public:
    SubCustomer(int subID, customer *supCustomer){
        this->subID = subID;
        this->supCustomer = supCustomer;
    }
    customer *supCustomer;  // 保存所属客户节点指针
    int subID;
    vector<int> bandwidthNeeded; // 需求带宽
};


int site_num = 0; //number of sites
int customer_num = 0; // number of customers
int T = 0; //number of time
vector<site> sites; //边缘节点的集合a
vector<customer> customers; //客户的集合
int Qos = 0;

string datafile_root = "../";


int str_to_int(string s){ //str转int
    int l = s.size();
    if (s[l-1] == '\r')
        l--;
    int ans = 0;
    for ( int i = 0; i<l; i++){
        ans = ans * 10 + s[i]-'0';
    }
    return ans;
}


vector<vector<string>> read_csv(string filename) //读csv文件
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


int read_qos(string filename){ //读取qos
    ifstream inFile(filename);
    string lineStr;

    getline(inFile,lineStr);
    getline(inFile,lineStr);
    return str_to_int(lineStr.substr(15));
}


void initializeData(){
    int Qos = read_qos(datafile_root + "data/config.ini");

    vector<vector<string>> vect = read_csv(datafile_root + "data/site_bandwidth.csv"); //
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
        new_site.qos_num = 0;
        sites.push_back(new_site);
    }
    vect = read_csv(datafile_root + "data/demand.csv");
    ite = vect.begin();
    temp_vect = *ite;


    for (vector<string>::iterator itee = ++temp_vect.begin(); itee != temp_vect.end(); itee++){
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
        for (vector<string>::iterator itee = ++temp_vect.begin(); itee != temp_vect.end(); itee++){
            itc->bandwidth_need.push_back(str_to_int(*itee));
            itc++;
        }
    }
    vect = read_csv(datafile_root + "data/qos.csv");
    int t = 0;
    for (ite = ++vect.begin(); ite != vect.end(); ite++){
        temp_vect = *ite;
        itc = customers.begin();
        for (vector<string>::iterator itee = ++temp_vect.begin(); itee != temp_vect.end(); itee++){
            if(str_to_int(*itee) < Qos){
                itc->qos.push_back(t);
                sites[t].qos_num++;
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
            if(n != 0){
                int j = 0;
                for (; j < n - 1; j++){
                    outfile << "<" << customers[i].infos[t][j].site_name_from << "," << customers[i].infos[t][j].bandwidth << ">" << ",";
                }
                outfile << "<" << customers[i].infos[t][j].site_name_from << "," << customers[i].infos[t][j].bandwidth << ">";
            }
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


void sortSites(vector<int> &a, int low, int high, vector<site> &sites){
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
    sortSites(a, low, j-1, sites);
    sortSites(a,j+1, high, sites);
}


 void sortCustoms(vector<int> &a, int low, int high, vector<customer> &customers){
     if (high <= low) return;
     int i = low;
     int j = high;
     int key = customers[a[low]].qos.size(); // 排序依据
     while(i < j){
         while (i < j && customers[a[j]].qos.size() <= key){
             j--;
         }
         while (i < j && customers[a[i]].qos.size() >= key){
             i++;
         }
         if (i >= j) break;
         int temp = a[i];
         a[i] = a[j];
         a[j] = temp;
     }
     int temp = a[low];
     a[low] = a[j];
     a[j] = temp;
     sortCustoms(a, low, j-1, customers);
     sortCustoms(a, j+1, high, customers);
}


vector<SubCustomer> preprocess(int n, vector<site> &sites, vector<customer> &customers){
    vector<SubCustomer> ret;

    for(int i = 0; i < sites.size(); i++){
        if(sites[i].qos_num == 0) { // 删除无效边缘节点
            // sites.erase(begin(sites) + i);
            // i--;
            // site_num--;
            sites[i].qos_num = -1; // 无效标志
        }
    }

    //sort(customers.begin(), customers.end(), cmpCustomer);
    vector<int> a;
    for(int i = 0; i < customer_num; i++){
        a.push_back(i);
    }
    sortCustoms(a, 0, customer_num - 1, customers);

    for(int i = 0; i < customer_num; i++){ // 遍历a
        sortSites(customers[a[i]].qos, 0, customers[a[i]].qos.size() - 1, sites);
        vector<int> tempBandwidthNeeded;
        vector<int> tempLastBandwidthNeeded;
        for(int t = 0; t < T; t++){
            tempBandwidthNeeded.push_back(customers[a[i]].bandwidth_need[t] / n);
            tempLastBandwidthNeeded.push_back(customers[a[i]].bandwidth_need[t] - (customers[a[i]].bandwidth_need[t] / n * (n - 1)));
        }
        for(int j = 0; j < n; j++){ // 均分为n份
            SubCustomer tempSubCustomer = SubCustomer(j, &customers[a[i]]);
            if(j < n - 1){
                tempSubCustomer.bandwidthNeeded = tempBandwidthNeeded;
            }
            else{
                tempSubCustomer.bandwidthNeeded = tempLastBandwidthNeeded;
            }
            ret.push_back(tempSubCustomer);
        }
    }
    return ret;
}


 bool dfs(vector<SubCustomer> &subCustomers, vector<int> &bandwidthLeft, int index, int t, map<int, int> &infoMap){
     if(index == subCustomers.size()) return true;

     vector<int> tempQos = subCustomers[index].supCustomer->qos; // 能满足当前客户节点的边缘节点列表

     for(int i = 0; i < tempQos.size(); i++){ // tempQos[i]为site的标号
         // subCustomers[index]选sites[tempQos[i]]
         if(subCustomers[index].bandwidthNeeded[t] > bandwidthLeft[tempQos[i]]){
             continue;
         }

         int hash = tempQos[i] * 100 + subCustomers[index].supCustomer->id;
         if(infoMap.count(hash) == 0 || infoMap[hash] == 0){
             infoMap[hash] = subCustomers[index].bandwidthNeeded[t];
         }
         else{
             infoMap[hash] += subCustomers[index].bandwidthNeeded[t];
         }
         bandwidthLeft[tempQos[i]] -= subCustomers[index].bandwidthNeeded[t];
         if(dfs(subCustomers, bandwidthLeft, index + 1, t, infoMap)){
             return true;
         }
         else{
             infoMap[hash] -= subCustomers[index].bandwidthNeeded[t];
             bandwidthLeft[tempQos[i]] += subCustomers[index].bandwidthNeeded[t];
         }
     }

     return false;
 }


void dfsAlgorithm(){
    vector<SubCustomer> subCustomers = preprocess(10, sites, customers); // n = 10
//    for(auto & subCustomer : subCustomers){
//    }
    vector<int> bandwidth;
    for(int i = 0; i < site_num; i++){
        bandwidth.push_back(sites[i].bandwidth);
    }
    for(int i = 0; i < customer_num; i++){
        customers[i].infos.resize(T);
    }
    for(int t = 0; t < T; t++){
        map<int, int> infoMap; // 边缘节点ID * 100 + 客户节点ID, 需求量
        vector<int> bandwidthLeft = bandwidth;
        if(!dfs(subCustomers, bandwidthLeft, 0, t, infoMap)) cout << "ERROR!\n";

        for(auto & it : infoMap){
            // cout << it.first << " " << it.second << "\n";

            int iCus = it.first % 100;
            int iSite = it.first / 100;
            if(it.second != 0) {
                customers[iCus].infos[t].push_back({sites[iSite].site_name, it.second});
            }
//            if(it.second == 0){
//                cout << t << " " << sites[iSite].site_name << " " << customers[iCus].customer_name << " " << it.second << "\n";
//            }
        }
        // cout << "---------------------------------\n\n";

    }


}


int main(){
    initializeData();

    dfsAlgorithm();
    //vanillaAlgorithm();

    outputRes();
    return 0;
}
