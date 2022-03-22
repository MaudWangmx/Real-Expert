#include<iostream>
#include<istream>
#include<vector>
#include<sstream>
#include<cstring>
#include<fstream>
#include<math.h>
using namespace std;

typedef struct{
    int id;
    string site_name;
    int bandwidth;
    vector<int> servable_customers;
} site; //边缘节点

typedef struct{
    string site_name_from;
    int bandwidth;
} info; //分配信息


class customer{ //客户
    public:
        customer(int id, string customer_name):id(id), customer_name(customer_name){}
        int id;
        string customer_name;
        vector<int> bandwidth_need; //需求带宽
        vector<int> qos; //需求qos
        vector<vector<info>> infos; //分配的结果
};



int site_num = 0; //number of sites
int customer_num = 0; // number of customers
int T = 0; //number of time
vector<site> sites; //边缘节点的集合
vector<customer> customers; //客户的集合
int Qos = 0;

string datafile_root = "/";

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
                sites[t].servable_customers.push_back(itc->id);
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


bool arrangeBaseOnCustomers(int* sitesUsed, int* customerSatisfied, int t, int** arrangement = nullptr){
    for (int c = 0; c < customer_num; c++){
        int customer_unsatisfied = customers[c].bandwidth_need[t] - customerSatisfied[c];
        if (customer_unsatisfied == 0)
            continue;
        int n = customers[c].qos.size();
        if (n == 0 && customer_unsatisfied > 0)
            return false;
        int average_bandwidth = ceil((double)customer_unsatisfied / n);

        //n = n / 4 != 0 ? n/4 : n;
        if (n > 4)
            average_bandwidth = ceil(customer_unsatisfied / (n/4));
        int bandwidth_need = average_bandwidth;
        int s = 0;
        for (; s < n - 1; s++){     // s for the site index in one customer's qos list
            int site_id = customers[c].qos[s];
            int site_left = sites[site_id].bandwidth - sitesUsed[site_id];

            if (customers[c].bandwidth_need[t] - customerSatisfied[c] < bandwidth_need)
                bandwidth_need = customers[c].bandwidth_need[t] - customerSatisfied[c];
            if (bandwidth_need == 0)
                break;
            if (site_left >= bandwidth_need){
                if (arrangement != nullptr)
                    arrangement[c][s] += bandwidth_need;
                else
                    customers[c].infos[t].push_back({sites[site_id].site_name, bandwidth_need});
                sitesUsed[site_id] += bandwidth_need;
                customerSatisfied[c] += bandwidth_need;
                bandwidth_need = average_bandwidth;
            }else{
                if (arrangement != nullptr)
                    arrangement[c][s] += site_left;
                else
                    customers[c].infos[t].push_back({sites[site_id].site_name, site_left});
                sitesUsed[site_id] += site_left;
                customerSatisfied[c] += site_left;
                bandwidth_need = average_bandwidth + (bandwidth_need - site_left);
            }
        }
        int site_id = customers[c].qos[s];
        int site_left = sites[site_id].bandwidth - sitesUsed[site_id];
        bandwidth_need = customers[c].bandwidth_need[t] - customerSatisfied[c];
        if (bandwidth_need == 0)
            continue;
        if (site_left >= bandwidth_need){
            if (arrangement != nullptr)
                arrangement[c][s] += bandwidth_need;
            else
                customers[c].infos[t].push_back({sites[site_id].site_name, bandwidth_need});
            sitesUsed[site_id] += bandwidth_need;
            customerSatisfied[c] += bandwidth_need;
            bandwidth_need = average_bandwidth;
        }else{
            return false;
        }

    }
    return true;
}

bool completelyOnCustomers(){
    int* site_used = new int[site_num];
    int* customer_satisfied = new int[customer_num];

    for (int t = 0; t < T; t++) {
        memset(site_used, 0, site_num * sizeof(int));
        memset(customer_satisfied, 0, customer_num * sizeof(int));
        for (int i = 0; i < customer_num; i++) {
            vector<info> temp_info;
            customers[i].infos.push_back(temp_info);
        }
        arrangeBaseOnCustomers(site_used, customer_satisfied, t);
    }
    return true;
}

int findSiteIndexInQosList(int customer_id, int site_id){
    int i = 0;
    while(customers[customer_id].qos[i] != site_id)
        i++;
    return i;
}


void arrangeOneSite(int site_id, int site_average, int t, int* customer_satisfied, int* site_used, int** arrangement){
    int s = site_id;
    int n = sites[s].servable_customers.size();
    if (n == 0)
        return;
    //int site_average_c = floor(site_average / n);
    int site_weights[n];
    int total_weight = 0;

    for (int i = 0; i < n; i++){
        if (customers[sites[s].servable_customers[i]].qos.size() == 0){
            site_weights[i] = 0;
            continue;
        }
        site_weights[i] = customers[sites[s].servable_customers[i]].bandwidth_need[t] / customers[sites[s].servable_customers[i]].qos.size();
        total_weight += site_weights[i];
    }
    if (total_weight == 0)
        return;
    int site_average_c = floor(site_average / total_weight);
    for (int i = 0; i < n; i++){        // i for customer index in one site's servable_customers list
        int customer_id = sites[s].servable_customers[i];

        int customer_unsatisfied = customers[customer_id].bandwidth_need[t] - customer_satisfied[customer_id];
        int site_index = findSiteIndexInQosList(customer_id, s);
        int site_weighted_give = site_average_c * site_weights[i];
        // cout << customer_unsatisfied << endl;
        if (site_weighted_give <= customer_unsatisfied){
            arrangement[customer_id][site_index] += site_weighted_give;
            //customers[customer_id].infos[t].push_back({sites[s].site_name, site_average_c});
            site_used[s] += site_weighted_give;
            customer_satisfied[customer_id] += site_weighted_give;
        }else{
            arrangement[customer_id][site_index] += customer_unsatisfied;
            //customers[customer_id].infos[t].push_back({sites[s].site_name, customer_unsatisfied});
            site_used[s] += customer_unsatisfied;
            customer_satisfied[customer_id] += customer_unsatisfied;
        }

    }
}
bool arrangeBaseOnSites(){

    int* site_used = new int[site_num];
    int* customer_satisfied = new int[customer_num];

    int** arrangement = new int*[customer_num];
    for (int i = 0; i < customer_num; i++) {
        arrangement[i] = new int[customers[i].qos.size()];
        memset(arrangement[i], 0, customers[i].qos.size() * sizeof(int));
    }

    for (int t = 0; t < T; t++) {
        int total_bandwidth = 0;
        memset(site_used, 0, site_num * sizeof(int));
        memset(customer_satisfied, 0, customer_num * sizeof(int));

        for (int i = 0; i < customer_num; i++) {
            total_bandwidth += customers[i].bandwidth_need[t];
            vector<info> temp_info;
            customers[i].infos.push_back(temp_info);
            memset(arrangement[i], 0, customers[i].qos.size() * sizeof(int));
        }
        int site_average = ceil((double)total_bandwidth/site_num);
        for (int s = 0; s < site_num; s++){
            arrangeOneSite(s, site_average, t, customer_satisfied, site_used, arrangement);
            /*
            int n = sites[s].servable_customers.size();
            if (n == 0)
                continue;
            //int site_average_c = floor(site_average / n);
            int site_weights[n];
            int total_weight = 0;

            for (int i = 0; i < n; i++){
                site_weights[i] = customers[sites[s].servable_customers[i]].bandwidth_need[t] / customers[sites[s].servable_customers[i]].qos.size();
                total_weight += site_weights[i];
            }
            int site_average_c = floor(site_average / total_weight);
            for (int i = 0; i < n; i++){        // i for customer index in one site's servable_customers list
                int customer_id = sites[s].servable_customers[i];

                int customer_unsatisfied = customers[customer_id].bandwidth_need[t] - customer_satisfied[customer_id];
                int site_index = findSiteIndexInQosList(customer_id, s);
                int site_weighted_give = site_average_c * site_weights[i];
                // cout << customer_unsatisfied << endl;
                if (site_weighted_give <= customer_unsatisfied){
                    arrangement[customer_id][site_index] = site_weighted_give;
                    //customers[customer_id].infos[t].push_back({sites[s].site_name, site_average_c});
                    site_used[s] += site_weighted_give;
                    customer_satisfied[customer_id] += site_weighted_give;
                }else{
                    arrangement[customer_id][site_index] = customer_unsatisfied;
                    //customers[customer_id].infos[t].push_back({sites[s].site_name, customer_unsatisfied});
                    site_used[s] += customer_unsatisfied;
                    customer_satisfied[customer_id] += customer_unsatisfied;
                }

            }
            */
        }
        arrangeBaseOnCustomers(site_used, customer_satisfied, t, arrangement);
        for (int i = 0; i < customer_num; i++){
            int n = customers[i].qos.size();
            for (int j = 0; j < n; j++){
                if (arrangement[i][j] == 0)
                    continue;
                customers[i].infos[t].push_back({sites[customers[i].qos[j]].site_name, arrangement[i][j]});
            }
        }
        //delete [] site_used;
        //delete [] customer_satisfied;
    }

    return true;
}

bool trick(){
    int* site_used = new int[site_num];
    int* customer_satisfied = new int[customer_num];

    int** arrangement = new int*[customer_num];
    for (int i = 0; i < customer_num; i++) {
        arrangement[i] = new int[customers[i].qos.size()];
        memset(arrangement[i], 0, customers[i].qos.size() * sizeof(int));
    }
    int timeP = (int) T * 0.05 - 1;     // clicks of a time piece
    if (timeP > 0){
        int site_num_per_tp = (int)(site_num - 1) * timeP / T;  // num of sites chosen per tp
        int timeP_num = ceil((double)T / timeP);    // num of time pieces
        for (int inter = 0; inter < timeP_num && timeP * inter < T; inter++ ){
            /* in a time piece */
            int start_site = inter * site_num_per_tp;
            for (int tp = 0; tp < timeP; tp++){
                /* in a click of a time piece */
                int t = timeP * inter + tp;
                if (t >= T)
                    break;
                int total_bandwidth = 0;
                memset(site_used, 0, site_num * sizeof(int));
                memset(customer_satisfied, 0, customer_num * sizeof(int));

                for (int i = 0; i < customer_num; i++) {
                    total_bandwidth += customers[i].bandwidth_need[t];
                    vector<info> temp_info;
                    customers[i].infos.push_back(temp_info);
                    memset(arrangement[i], 0, customers[i].qos.size() * sizeof(int));
                }
                for (int site_focus = start_site; site_focus < start_site + site_num_per_tp && site_focus < site_num; site_focus++){
                    /* fill one site*/
                    arrangeOneSite(site_focus, sites[site_focus].bandwidth, t, customer_satisfied, site_used, arrangement);
                    total_bandwidth -= site_used[site_focus];
                }
                if (site_num - site_num_per_tp <= 0){
                    if (total_bandwidth == 0)
                        return true;
                    else
                        return false;
                }
                int site_average = total_bandwidth / (site_num - site_num_per_tp);
                for (int site_focus = 0; site_focus < start_site; site_focus++){
                    arrangeOneSite(site_focus, site_average, t, customer_satisfied, site_used, arrangement);
                }
                for (int site_focus = start_site + site_num_per_tp; site_focus < site_num; site_focus++){
                    arrangeOneSite(site_focus, site_average, t, customer_satisfied, site_used, arrangement);
                }
                arrangeBaseOnCustomers(site_used, customer_satisfied, t, arrangement);
                for (int i = 0; i < customer_num; i++){
                    int n = customers[i].qos.size();
                    for (int j = 0; j < n; j++){
                        if (arrangement[i][j] == 0)
                            continue;
                        customers[i].infos[t].push_back({sites[customers[i].qos[j]].site_name, arrangement[i][j]});
                    }
                }
            }
        }
        return true;
    }
    else
        return false;
}

int main(){
    
    initializeData();
    //arrangeBaseOnSites();
    //completelyOnCustomers();
    //vanillaAlgorithm();
    if (!trick())
        completelyOnCustomers();
    outputRes();
    return 0;
}