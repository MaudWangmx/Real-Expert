#include<iostream>
#include<istream>
#include<vector>
#include<sstream>
#include<fstream>
using namespace std;

typedef struct site{ //边缘节点
    int id;
    string site_name;
    int bandwidth;
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


int main(){
    vector<vector<string>> vect = read_csv("../data/site_bandwidth.csv"); //
    int id = 0;
    vector<site> sites; //边缘节点的集合
    vector<customer> customers; //客户的集合
    int Qos = read_qos("../data/config.ini");

    //遍历用 以下读取数据
    vector<string> temp_vect;
    vector<vector<string>>::iterator ite;
    vector<customer>::iterator itc;

    for (ite = ++vect.begin(); ite != vect.end(); ite++)
    {
        site new_site;
        temp_vect = *ite;
        new_site.id = id++;
        new_site.site_name = temp_vect[0];
        new_site.bandwidth = str_to_int(temp_vect[1]);
        sites.push_back(new_site);
        // for (vector<string>::iterator itee = temp_vect.begin(); itee != temp_vect.end(); itee++)
        //     cout << *itee << endl;
    }
    // for (vector<site>::iterator itor = sites.begin(); itor != sites.end(); itor++){
    //     cout << itor->site_name << "  " << itor->bandwidth << endl;
    // }
    vect = read_csv("../data/demand.csv");
    ite = vect.begin();
    temp_vect = *ite;
    id = 0;
    for (vector<string>::iterator itee = ++temp_vect.begin(); itee != temp_vect.end(); itee++){
        customers.push_back(customer(id++, *itee));
    }
    for (ite = ++vect.begin(); ite != vect.end(); ite++)
    {
        temp_vect = *ite;
        itc = customers.begin();
        for (vector<string>::iterator itee = ++temp_vect.begin(); itee != temp_vect.end(); itee++){
            itc->bandwidth_need.push_back(str_to_int(*itee));
            itc++;
        }
    }
    vect = read_csv("../data/qos.csv");
    int t = 0;
    for (ite = ++vect.begin(); ite != vect.end(); ite++){
        temp_vect = *ite;
        itc = customers.begin();
        for (vector<string>::iterator itee = ++temp_vect.begin(); itee != temp_vect.end(); itee++){
            if(str_to_int(*itee) <= Qos){
                itc->qos.push_back(t);
            } 
            // cout << str_to_int(*itee) << endl;
            itc++;
        }
        t++;

    }
    
    // for (itc=customers.begin(); itc != customers.end(); itc++){
    //     cout << itc->id << " " << itc->customer_name << endl;
    //     cout << "bandwidth_need:" << endl;
    //     for (vector<int>::iterator itor=itc->bandwidth_need.begin(); itor !=itc->bandwidth_need.end(); itor++){
    //         cout << *itor << endl;
    //     }
    //     cout << endl;
    //     cout << "qos:" << endl;
    //     for (vector<int>::iterator itor=itc->qos.begin(); itor !=itc->qos.end(); itor++){
    //         cout << *itor << endl;
    //     }
    // }
    // cout << Qos << endl;
    //数据读取完毕 以下处理数据
    return 0;
}