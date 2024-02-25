#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using filesystem::path;

path operator""_p(const char* data, std::size_t sz) {
    return path(data, data + sz);
}

// напишите эту функцию
bool Preprocess(const path& in_file, const path& out_file, const vector<path>& include_directories)
{      
    ifstream infile(in_file);
    if(!infile.is_open())
    {        
        return false;
    }
    
    ofstream outfile(out_file, ios::app);     
    
    static regex reg1(R"/(\s*#\s*include\s*"([^"]*)"\s*)/");
    static regex reg2(R"/(\s*#\s*include\s*<([^>]*)>\s*)/");
    smatch m;
    
    string str;
    path p = in_file.parent_path();
    int num_str = 0;
    while(getline(infile,str))
    {       
        num_str++;
        if (regex_match(str, m, reg1)) 
        {            
            outfile.close();
            if(!Preprocess(path(p / path(m[1])),out_file,include_directories))
            {                
                bool a1=false;
                for(const auto& include_dir:include_directories)
                {                    
                    if(Preprocess(path(include_dir /path(m[1])),out_file,include_directories))
                    {
                        a1=true;
                        break;
                    }                    
                }
                if(!a1)
                {
                    cout << "unknown include file "s << m[1] << " at file " << in_file.string() << " at line "s << num_str << endl;
                    return false;
                }
            }
            
            outfile.open(out_file, ios::app);
        }
        
        else if (regex_match(str, m, reg2)) 
        {
            outfile.close();
            bool a2 = false;
            for(const auto& include_dir:include_directories)
            {                
                if(Preprocess(path(include_dir /path(m[1])),out_file,include_directories))
                {
                    a2=true;
                    break;
                }
            }
            if(!a2)
            {
                cout << "unknown include file "s << m[1] << " at file " << in_file.string() << " at line "s << num_str << endl;
                return false;
            }
            outfile.open(out_file, ios::app);
        }
        
        else if (!regex_match(str, m, reg2) && !regex_match(str, m, reg1))
        {                
            outfile << str << "\n";                      
        } 
    }
    
    return true;
}

string GetFileContents(string file) {
    ifstream stream(file);

    // конструируем string по двум итераторам
    return {(istreambuf_iterator<char>(stream)), istreambuf_iterator<char>()};
}

void Test() {
    error_code err;
    filesystem::remove_all("sources"_p, err);
    filesystem::create_directories("sources"_p / "include2"_p / "lib"_p, err);
    filesystem::create_directories("sources"_p / "include1"_p, err);
    filesystem::create_directories("sources"_p / "dir1"_p / "subdir"_p, err);

    {
        ofstream file("sources/a.cpp");
        file << "// this comment before include\n"
                "#include \"dir1/b.h\"\n"
                "// text between b.h and c.h\n"
                "#include \"dir1/d.h\"\n"
                "\n"
                "int SayHello() {\n"
                "    cout << \"hello, world!\" << endl;\n"
                "#   include<dummy.txt>\n"
                "}\n"s;
    }
    {
        ofstream file("sources/dir1/b.h");
        file << "// text from b.h before include\n"
                "#include \"subdir/c.h\"\n"
                "// text from b.h after include"s;
    }
    {
        ofstream file("sources/dir1/subdir/c.h");
        file << "// text from c.h before include\n"
                "#include <std1.h>\n"
                "// text from c.h after include\n"s;
    }
    {
        ofstream file("sources/dir1/d.h");
        file << "// text from d.h before include\n"
                "#include \"lib/std2.h\"\n"
                "// text from d.h after include\n"s;
    }
    {
        ofstream file("sources/include1/std1.h");
        file << "// std1\n"s;
    }
    {
        ofstream file("sources/include2/lib/std2.h");
        file << "// std2\n"s;
    }
    //Preprocess("sources"_p / "a.cpp"_p, "sources"_p / "a.in"_p,
    //                              {"sources"_p / "include1"_p,"sources"_p / "include2"_p});
    assert((!Preprocess("sources"_p / "a.cpp"_p, "sources"_p / "a.in"_p,
                                  {"sources"_p / "include1"_p,"sources"_p / "include2"_p})));

    ostringstream test_out;
    test_out << "// this comment before include\n"
                "// text from b.h before include\n"
                "// text from c.h before include\n"
                "// std1\n"
                "// text from c.h after include\n"
                "// text from b.h after include\n"
                "// text between b.h and c.h\n"
                "// text from d.h before include\n"
                "// std2\n"
                "// text from d.h after include\n"
                "\n"
                "int SayHello() {\n"
                "    cout << \"hello, world!\" << endl;\n"s;

    assert(GetFileContents("sources/a.in"s) == test_out.str());
}

int main() {
    Test();
    ifstream in("sources"_p / "a.in"_p);
    string s;
    for(;in;)
    {        
        getline(in,s);
        cout << s << "\n";
    }        
}
