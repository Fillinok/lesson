#include <cassert>
#include <iostream>
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
        q.type = QueryType::BusesForStop;
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
        os << "Stop "s << r.stop << ": "s;
        for(string s:r.buses)
        {
            os << s << " "s;
        }
        os << endl; 
    }
    else
    {
        os << "No stop"s;
    }
    
    return os;
}

struct StopsForBusResponse {
    map<string,vector<string>> sfbr;
};

ostream& operator<<(ostream& os, const StopsForBusResponse& r) {
    if(!r.sfbr.empty())
    {
        for(const auto& [stop,buses]: r.sfbr)
        {
            os << "Stop "s << stop << ": "s;
            for(string s : buses)
            {
                os << s << " "s;
            }
        }
    }
    else
    {
        os << "No bus"s << endl;
    }
    return os;
}

struct AllBusesResponse {
    map<string,vector<string>> abr;
};

ostream& operator<<(ostream& os, const AllBusesResponse& r) {
    if(!r.abr.empty())
    {
        for(const auto& [bus,stops]:r.abr)
        {
            os << endl;
            os << "Bus "s << bus << ": "s;
            for(string s:stops)
            {
                os << s << " "s;
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
            buses_[bus]=stops;
            for(const string& s : stops)
            {
               buses_to_stops_[s].push_back(bus); 
            }            
        }
        all_buses_response_.abr.insert({bus,stops});
    }

    BusesForStopResponse GetBusesForStop(const string& cur_stop) const {
        BusesForStopResponse buses_for_stop_response;
        for(const auto& [stop,buses] : buses_to_stops_)
        {
            if(cur_stop == stop)
            {
                buses_for_stop_response.stop=stop;
                for(string s: buses)
                {
                    buses_for_stop_response.buses.push_back(s);
                }                
            }
        }
        return buses_for_stop_response;
    }

    StopsForBusResponse GetStopsForBus(const string& cur_bus) const {        
        StopsForBusResponse st; 
        vector<string> temp;
        for(const auto& [bus,stops]:all_buses_response_.abr) //выбираем автобус с остановками
        {
            if(cur_bus == bus) // Если такой автобус есть
            {
                for(string s: stops) // смотрим каждую остановку
                {
                    for(const auto& [bu,sto]:all_buses_response_.abr)
                    {
                        for(string sqt:sto)
                        {
                            if(sqt == s)
                            {
                                temp.push_back(bu);
                            }                            
                        }
                    }
                    st.sfbr.insert({s,temp});
                }
            }
        }
        return st;
    }

    AllBusesResponse GetAllBuses() const {        
        return all_buses_response_;
    }
    
    private:
    map<string, vector<string>> buses_;
    map<string, vector<string>> buses_to_stops_;
    map<string, vector<string>> stops_to_buses_;
    AllBusesResponse all_buses_response_;
    
};


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
