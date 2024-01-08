#include <cassert>
#include <iostream>
#include <sstream>
#include <map>
#include <string>
#include <vector>

using namespace std;

enum class QueryType {
    NewBus,
    BusesForStop,
    StopsForBus,
    AllBuses,
};

struct Query {
    QueryType type;
    string bus;
    string stop;
    vector<string> stops;
};
int Count(const vector<string>& str, const string cur_str) 
{
    int x=0;
    for(string s:str)
    {
        if(s == cur_str)
        {
            x++;
        }
    }
    return x;
}

istream& operator>>(istream& is, Query& q) 
{   
    string str_type;
    q.stops.clear();
    is >> str_type;
    if(str_type == "NEW_BUS")
    {
        q.type = QueryType::NewBus;
        is >> q.bus >> q.stop;
        for(int x=0;x<stoi(q.stop);x++)
        {
            string str;
            is >> str;
            q.stops.push_back(str);
        }
    }
    else if (str_type == "BUSES_FOR_STOP")
    {
        q.type = QueryType::BusesForStop;
        is >> q.stop;
    }
    else if (str_type == "STOPS_FOR_BUS")
    {
        q.type = QueryType::StopsForBus;
        is >> q.bus;
    }
    else if (str_type == "ALL_BUSES")
    {
        q.type = QueryType::AllBuses;
        return is;
    }
    
    return is;
}

struct BusesForStopResponse {
    string stop;
    vector<string> buses;
};

ostream& operator<<(ostream& os, const BusesForStopResponse& r) {
    if(!r.stop.empty())
    {        
        for(string s:r.buses)
        {
            if(s != r.buses[r.buses.size()-1])
            {
                os << s << " "s;
            }
            else
            {
                os << s;
            } 
        }         
    }
    else
    {
        os << "No stop"s;
    }
    
    return os;
}

struct StopsForBusResponse {
    string cur_bus;
    vector<pair<string,vector<string>>> stop_stops;
};

ostream& operator<<(ostream& os, const StopsForBusResponse& r) {
    if(!r.cur_bus.empty())
    {
        for(const auto& x : r.stop_stops)
        {
            os << "Stop "s << x.first << ": "s;
            if(!x.second.empty())
            {
                for(string s : x.second)
                {
                    if(s != x.second[x.second.size()-1])
                    {
                        os << s << " "s;
                    }
                    else
                    {
                        os << s;
                    }                    
                }  
            }
            else
            {
                os << "no interchange"s;
            }
            
            if(x != r.stop_stops[r.stop_stops.size()-1])
            {
                os << endl;
            }                                   
        }
    }
    else
    {
        os << "No bus"s;
    }
    return os;
}

struct AllBusesResponse {
    map<string,vector<string>> abr;
};

ostream& operator<<(ostream& os, const AllBusesResponse& r) {
    if(!r.abr.empty())
    {        
        int asd = r.abr.size()-1;
        int x=0;
        for(const auto& [bus,stops]:r.abr)
        {           
            os << "Bus "s << bus << ": "s;
            for(string s:stops)
            {
                if(s != stops[stops.size()-1])
                {
                    os << s << " "s;
                }
                else
                {
                    os << s;
                }
            }
            if(x<asd)
            {
                os << endl;
                x++;
            }
        }  
    }
    else
    {
        os << "No buses"s;
    }
    
    return os;
}

class BusManager {
public:
    void AddBus(const string& bus, const vector<string>& stops) 
    {        
        if(buses_.count(bus) == 0)
        {            
            buses_2_.push_back({bus,stops});
            buses_[bus]=stops;            
            for(const string& s : stops)
            {
               buses_to_stops_[s].push_back(bus); 
            }            
        }
        all_buses_response_.abr.insert({bus,stops});
    }

    BusesForStopResponse GetBusesForStop(const string& stop) const {
        BusesForStopResponse buses_for_stop_response;
        for(const auto& [cur_stop,buses] : buses_to_stops_)
        {
            if(cur_stop == stop)
            {
                buses_for_stop_response.stop=cur_stop;
                for(string s: buses)
                {
                    buses_for_stop_response.buses.push_back(s);
                }                
            }
        }
        return buses_for_stop_response;
    }

    StopsForBusResponse GetStopsForBus(const string& bus) const {        
        StopsForBusResponse st;
        vector<string> temp; //список остановок искомого автобуса        
        for(const auto& [key,value]:buses_2_)
        {
            if(key == bus)
            {
                temp=value;
                st.cur_bus=key;
            }
        }
        for(string s: temp)
            {
            vector<string> temp2;
                for(const auto& [key,value]:buses_2_)
                {
                    if(Count(value,s) > 0 && bus!=key)
                    {
                        temp2.push_back(key);
                    }
                }
            st.stop_stops.push_back({s,temp2});
            }       
        
        return st;
    }

    AllBusesResponse GetAllBuses() const {        
        return all_buses_response_;
    }
    
    private:
    map<string, vector<string>> buses_;
    vector<pair<string, vector<string>>> buses_2_;
    map<string, vector<string>> buses_to_stops_;    
    AllBusesResponse all_buses_response_;
    
};

void TestPointOutput() {
    AllBusesResponse p;
    p.abr = {{"32k"s,{"Vnukovo"s,"Skolkovo"s}}} ;   
    ostringstream output;
    output << p;    
    assert(output.str() == "Bus 32k: Vnukovo Skolkovo"s );    
}
void TestPointInput() {
    istringstream input;
    // Метод str(строка) у istringstream позволяет задать содержимое, которое будет считываться из istringstream
    input.str("BUSES_FOR_STOP Marushkino"s);
    Query p;
    input >> p;
    assert(p.type == QueryType::BusesForStop);
    assert(p.stop == "Marushkino"s);  
}

int main() {
    
    int query_count;
    Query q;

    cin >> query_count;    

    BusManager bm;
    for (int i = 0; i < query_count; ++i) {
        cin >> q;
        switch (q.type) {
            case QueryType::NewBus:                
                bm.AddBus(q.bus, q.stops);                
                break;                
            case QueryType::BusesForStop:                
                cout << bm.GetBusesForStop(q.stop) << endl;
                break;
            case QueryType::StopsForBus:                
                cout << bm.GetStopsForBus(q.bus) << endl;                
                break;
            case QueryType::AllBuses:                
                cout << bm.GetAllBuses() << endl;                
                break;
        }
    }
}
