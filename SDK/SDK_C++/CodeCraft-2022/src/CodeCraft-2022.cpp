#include<iostream>
#include<istream>
#include<vector>
#include<sstream>
#include<fstream>
using namespace std;

typedef struct{
    int id;
    string site_name;
    int bandwidth;
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

int main(){
    
    initializeData();

    vanillaAlgorithm();
    
    outputRes();
    return 0;
}