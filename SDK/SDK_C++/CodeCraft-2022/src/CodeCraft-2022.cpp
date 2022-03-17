#include<iostream>
#include<istream>
#include<vector>
#include<sstream>
#include<algorithm>
#include<fstream>
using namespace std;

typedef struct site{ //边缘节点
    int id;
    string site_name;
    int bandwidth;
    int qos_num;
};

typedef struct info //分配信息
{
    string site_name_from;
    int bandwidth;
};

class customer{ //客户
    public:
        customer(int id, string customer_name):id(id), customer_name(customer_name){}
        int id;
        string customer_name;
        vector<int> bandwidth_need; //需求带宽
        vector<int> qos; //需求qos
        vector<vector<info>> infos; //分配的结果
};
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

// bool dfs(vector<customer> &customers, int i, vector<int> bandwidth_cust_need, vector<int> bandwidth_site_left, vector<vector<info>> &info_result, vector<site> &sites){
//     int n = customers[i].qos.size();
//     for ( int j=0; j<n; j++){
//         int temp_qos = customers[i].qos[j];
//         int count = 0;
//         vector<info> cur_info;
//         if (bandwidth_cust_need[i] <= bandwidth_site_left[temp_qos]){
//             if (bandwidth_cust_need[i] >=1000){
//                 int used = bandwidth_cust_need[i] / 2;
//                 bandwidth_cust_need[i] -= used;
//                 bandwidth_site_left[temp_qos] -= used;
//                 info new_info;
//                 new_info.bandwidth = used;
//                 new_info.site_name_from = sites[temp_qos].site_name;
//                 cur_info.push_back(new_info);
//                 count++;
//             }
//             else{
//                 bandwidth_site_left[temp_qos] -= bandwidth_cust_need[i];
//                 bandwidth_cust_need[i] = 0;
//                 if (dfs(customers, i+1, bandwidth_cust_need, bandwidth_site_left, info_result, sites)){
//                     info new_info;
//                     new_info.bandwidth = bandwidth_cust_need[i];
//                     new_info.site_name_from = sites[temp_qos].site_name;
//                     cur_info.push_back(new_info);
//                     count++;
//                     info_result.push_back(cur_info);
//                     return true;
//                 }  
//             }
//         }
//         else if(bandwidth_site_left[temp_qos] != 0){
//             bandwidth_cust_need[i] -=bandwidth_site_left[temp_qos];
//             bandwidth_site_left[temp_qos] = 0;
//             info new_info;
//             new_info.bandwidth = bandwidth_site_left[temp_qos];
//             new_info.site_name_from = sites[temp_qos].site_name;
//             cur_info.push_back(new_info);
//             count++;
//         }
//     }
//     if (bandwidth_cust_need[i] != 0)
//         return false;
// }
void qsort_site(vector<int> &a, int low, int high, vector<site> &sites){
    if (high <= low) return;
    int i = low;
    int j = high;
    int key_index = low;
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

int main(){
    vector<vector<string>> vect = read_csv("/data/site_bandwidth.csv"); //
    int site_num = 0; //number of sites
    int customer_num = 0; // number of customers
    int T = 0; //number of time
    vector<site> sites; //边缘节点的集合
    vector<customer> customers; //客户的集合
    int Qos = read_qos("/data/config.ini");
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
    vect = read_csv("/data/demand.csv");
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
    vect = read_csv("/data/qos.csv");
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
    // for (int i=0; i<customer_num; i++){
    //     qsort_site(customers[i].qos, 0, int(customers[i].qos.size())-1, sites);
    // }    
    
    
    //数据读取完毕 以下处理数据
    ofstream outfile("/output/solution.txt");
    for (int time_i = 0; time_i<T; time_i++){
        vector<int> bandwidth_cust_need;
        vector<int> bandwidth_site_left;
        
        for (int i=0; i<customer_num; i++){
            bandwidth_cust_need.push_back(customers[i].bandwidth_need[time_i]);
        }
        for ( int i=0; i<site_num; i++){
            bandwidth_site_left.push_back(sites[i].bandwidth);
        }
        // vector<vector<info>> infos_result;

        // dfs(customers, 0, bandwidth_cust_need, bandwidth_site_left, infos_result);
        for ( int i=0; i<customer_num; i++){
            outfile << customers[i].customer_name << ":";
            int trick = 100;
            int n = customers[i].qos.size();
            int used = bandwidth_cust_need[i];
            if ( n > 13)
                used = bandwidth_cust_need[i] / (n/13);
            if (used < trick)
                used = trick;
            for ( int j=0; j<n; j++){
                int temp_qos = customers[i].qos[j];
                if (bandwidth_site_left[temp_qos] >= used && bandwidth_cust_need[i]!=0){
                    if (bandwidth_cust_need[i] >= used){
                        bandwidth_cust_need[i] -= used;
                        bandwidth_site_left[temp_qos] -= used;
                        // cout << "bandwith_cust_need" << " " << used << " " << endl;
                        outfile << "<" << sites[temp_qos].site_name << "," << used << ">";
                        if ( bandwidth_cust_need[i] != 0)
                            outfile << ",";
                        else
                            break;
                    }
                    else{
                        outfile << "<" << sites[temp_qos].site_name << "," << bandwidth_cust_need[i] << ">";
                        bandwidth_site_left[temp_qos] -= bandwidth_cust_need[i];
                        bandwidth_cust_need[i] = 0;
                        break;  
                    }
                    
                }
                else if(bandwidth_site_left[temp_qos] != 0){
                    if (bandwidth_cust_need[i] >= bandwidth_site_left[temp_qos]){
                        bandwidth_cust_need[i] -=bandwidth_site_left[temp_qos];
                        outfile << "<" << sites[temp_qos].site_name << "," << bandwidth_site_left[temp_qos] << ">";
                        bandwidth_site_left[temp_qos] = 0;
                         if ( bandwidth_cust_need[i] != 0)
                            outfile << ",";
                        else
                            break;
                    }
                    else{
                        
                        outfile << "<" << sites[temp_qos].site_name << "," << bandwidth_cust_need[i] << ">";
                        bandwidth_site_left[temp_qos] -= bandwidth_cust_need[i];
                        bandwidth_cust_need[i] =0 ;
                        if ( bandwidth_cust_need[i] != 0)
                            outfile << ",";
                        else
                            break;
                    }
                    
                }
            }
            outfile << endl;
            if (bandwidth_cust_need[i]>0)
                cout << "wrong!" << endl;
        }
    }
    return 0;
}