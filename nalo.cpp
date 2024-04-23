#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <string>
#include <map>
#include <cstdint>
#include <algorithm> 
#include <bitset>
#include <cctype>
#include <cstring>
#include <chrono>
#include <thread>
#include "serialib.h"
using namespace std;
bool error =false;
struct Contens{
    vector<uint8_t> send;
    vector<string> marker;
    vector<unsigned int> marker_line;
    vector<string> macro;
    vector<unsigned int> macro_value;
    int line_counter=0;
};
Contens contens;
map<string, unsigned int> mappings = { 
        {"nop", 0x00},
        {"bra", 0x01},
        {"beq", 0x02},
        {"bcy",	0x03},
        {"rea",	0x04},
        {"re0", 0x05},
        {"re1",	0x06},
        {"re2", 0x07},
        {"re3",	0x08},
        {"st:", 0x09},
        {":en", 0x0A},
        {"lda", 0x10},
        {"ldn", 0x20},
        {"sta", 0x30},
        {"adn", 0x40},
        {"ada", 0x50},
        {"sun", 0x60},
        {"sua", 0x70},
        {"ma1", 0x80},
        {"ma2", 0x90},
        {"mn1", 0xA0},
        {"mn2", 0xB0},
        {"ccf", 0xC0},
        {"ash", 0xD0},
        {"its", 0xE0},
        {"csf", 0xF0}
};

inline void ltrim(std::string &s);
inline void rtrim(std::string &s);
inline void trim(std::string &s);
inline std::string trim_copy(std::string s);
void write_help();
string str_lower(const string temp2);
int number_is_binary(const string temp, int temp2,const string file);
int number_is_hex(const string temp, int temp2,const string file);
int number_is_decimal(const string temp,int temp2,const string file);
int locate_macro(const string temp,int temp2,const string file);
int locate_marker(const string temp, int temp2,const string file);
int what_is_the_number(const string temp,int temp2,const string file);
int what_is_the_command(const string temp,int temp2,const string file);
bool create_binary(const string temp,const string temp_number, int temp2,const string file,bool err);
void locate_duplicates_macro(int line,const string file,const string macro);
void locate_duplicates_marker(int line,const string file,const string marker);
int first_run(const string file,int counter);
void second_run(const string file);
int write_debug(string file,int counter);
void serial_upload(string port);

int main(int argc, char **argv){

    if(str_lower(argv[1])=="-h"){
        write_help();
        return 0;
    }
    bool debug=false,force_upload=false,uploadport=false;
    string temp2=argv[argc-1];
    if(temp2.find('/') != string::npos){
        uploadport=true;
    }
    if(str_lower(argv[argc-1-uploadport])=="-d")
        debug=true;
    if(str_lower(argv[argc-1-debug-uploadport])=="-f"){
        force_upload=true;
        if(str_lower(argv[argc-1-force_upload-uploadport])=="-d"){
            debug=true;
        }
    }

    int temp=0;
    for(int i=1;i<argc-debug-force_upload-uploadport;i++){
        string temp3=argv[i];
        if(temp3.find(".nal")==string::npos){
            cerr << "\033[1;31merr 001: \033[0m"<< temp3 <<" is not a .nal file"<< endl;
            error=true;
        }
        temp=first_run(argv[i],temp);
    }
    for(int i=1;i<argc-debug-force_upload-uploadport;i++){
        second_run(argv[i]);
        if(argc-debug-force_upload-1!=i+1){
            contens.send[contens.send.size()-1]=0;
            
        }   
    }
    if((!error || force_upload) && uploadport){
        serial_upload(temp2);
    }
    if(debug){
        int k=0;
        for(int i=1;i<argc-debug-force_upload-uploadport;i++){
            cout<<"---------- "<<argv[i]<<" ----------"<<endl;
            k=write_debug(argv[i],k);
        }
    }

    return 0;
}

inline void ltrim(std::string &s){
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
inline void rtrim(std::string &s){
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

inline void trim(std::string &s){
    rtrim(s);
    ltrim(s);
}

inline std::string trim_copy(std::string s){
    trim(s);
    return s;
}
void write_help(){
	cout << "0x   hexadecimal   to write a number in hexadecimal write it like this: 0xF\n"
                "0b   binary        to write a number in binary write it like this: 0b0010\n"
                "ma:  macro start   you can use macros if you write after ma: a name with more then 3 letters like: number 5\n"
                ":me  macro end     you can use this macro in your code to use it write the name after a command like: lda number\n"
                "nop  0000 0000     does nothing\n"
                "bra  0000 0001     branch to a marker\n"
                "beq  0000 0010     branch if the same flag is true\n"
                "bcy  0000 0011     branch if the carry flag is true\n"
                "rea  0000 0100     read the number on the port and load it in the akkumulator\n"
                "re0  0000 0101     branch if the pin0 is high\n"
                "re1  0000 0110     branch if the pin1 is high\n"
                "re2  0000 0111     branch if the pin2 is high\n"
                "re3  0000 1000     branch if the pin3 is high\n"
                "st:  0000 1001     this command tells the cpu that a program is gonna be uploaded\n"
                ":en  0000 1010     this commadn tells the cpu that a program is finished uploaded\n"
                "lda  0001 xxxx     stand for load address is used to load a variable\n"
                "ldn  0010 xxxx     stand for load number it's used to insert new numbers\n"
                "sta  0011 xxxx     stand for stor in address it stores the number in the address\n"
                "adn  0100 xxxx     stand for add number it add the number\n"
                "ada  0101 xxxx     stand for add address it adds the contens of the address\n"
                "sun  0110 xxxx     stand for substract number\n"
                "sua  0111 xxxx     stand for substract address\n"
                "ma1  1000 xxxx     stand for multiplicat number first piece it hase to be split two because the number that commes out of a multiplication is 8 bit big\n"
                "ma2  1001 xxxx     stand for multiplicat number second piece\n"
                "mn1  1010 xxxx     stand for multiplicat address first piece\n"
                "mn2  1011 xxxx     stand for multiplicat address second piece\n"
                "ccf  1100 0000     stannd for clear carry flag\n"
                "ash  1101 0000     stand for akkumulator shift it's used to get the resault of a calculation\n"
                "its  1110 xxxx     stand for if its the same it compar two numbers it sets the same flag\n"
                "csf  1111 0000     stand for clear same flag"
        <<endl;

}
string str_lower(const string temp2){
    string temp;
    for(int i=0;i < temp2.size();i++){
            temp= temp + (char) tolower(temp2[i]);
    }
    return temp;   
}

int number_is_binary(const string temp, int temp2,const string file){
    try {
        stoi(temp.substr(2),nullptr,2);
    }catch (const std::invalid_argument& e) {
        cerr << "\033[1;31merr 006: \033[0m"<< file<<" : "<< temp2 <<"  "<< temp <<" numbers is not a binary number" << endl;
        error=true;
        return 0;
    } 
    if(temp.size() > 6){
        cerr << "\033[1;31merr 003: \033[0m"<< file <<" : "<< temp2 <<" binary number has to be in scope between 0000 and 1111" << endl;
        error=true;
        return 0;
    }
    return stoi(temp.substr(2),nullptr,2);
}

int number_is_hex(const string temp, int temp2,const string file){
    try {
        stoi(temp.substr(2),nullptr,16);
    }catch (const std::invalid_argument& e) {
        cerr << "\033[1;31merr 006: \033[0m"<< file<<" : "<< temp2 <<" numbers is not hexadecimal" << endl;
        error=true;
        return 0;
    }
    if(temp.size() > 4){
        cerr << "\033[1;31merr 003: \033[0m"<< file<<" : "<< temp2 <<" hexadecimal number has to be in scope between 0 and F" << endl;
        error=true;
        return 0;
    }
    return stoi(temp.substr(2),nullptr,16);
}

int number_is_decimal(const string temp,int temp2,const string file){
        try{
            stoi(temp);
        }catch (const std::invalid_argument& e) {
            cerr << "\033[1;31merr 006: \033[0m"<< file << " : " << temp2 <<"  "<< temp <<" numbers is not a number" << endl;
            error=true;
            return 0;
        }
        if(stoi(temp)>15||stoi(temp)<0){
            cerr << "\033[1;31merr 003: \033[0m"<<file<<" : "<< temp2 <<" number has to be in scope between 0 and 15" << endl;
            error=true;
            return 0;    
        }
        return stoi(temp);
}
int locate_macro(const string temp,int temp2,const string file){
    for(int i=0;i < contens.macro.size();i++){
        if(contens.macro[i]==str_lower(temp)){
            return contens.macro_value[i];        
        }
    }
    cerr << "\033[1;31merr 007: \033[0m"<< file<<" : "<< temp2 <<"  '"<< temp <<"' macro does not exist" << endl;
    error=true;
    return 0;
}
int locate_marker(const string temp, int temp2,const string file){
     for(int i=0;i < contens.marker.size();i++){
        if(contens.marker[i]==str_lower(temp)){
            return contens.marker_line[i];        
        }
    }
    cerr << "\033[1;31merr 008: \033[0m"<< file << " : " << temp2 <<"  '"<< temp <<"' marker does not exist" << endl;
    error=true;
   return 0; 
}

int what_is_the_number(const string temp,int temp2,const string file){
    if(temp.substr(0, 2) == "0b")
        return number_is_binary(temp,temp2,file);
    if(temp.substr(0, 2) == "0x")
        return number_is_hex(temp,temp2,file);
    istringstream iss(temp);
    double number;
    iss >> noskipws >> number; 
    if(iss.eof() && !iss.fail())
        return number_is_decimal(temp,temp2,file);
    if(temp.size()>3)
        return locate_macro(temp,temp2,file);
    return 0;
}
int what_is_the_command(const string temp,int temp2,const string file){
    if(mappings.find(str_lower(temp))==mappings.end()){
        cerr << "\033[1;31merr 004: \033[0m"<< file << " : " << temp2  <<" '"<< temp <<"' unknown command"<< endl;
        error=true;
        return 0;
    }
    return (int) mappings[temp];

}
bool create_binary(const string temp,const string temp_number, int temp2,const string file,bool err){
    int temp3 =what_is_the_command(temp,temp2,file);
    if(temp3 > 0 && temp3 != 4 && temp3 < 9){
        int temp4= locate_marker(temp_number,temp2,file)<<3;
        contens.send.push_back(temp3);
        contens.send.push_back(temp4>>8);
        contens.send.push_back(temp4);
        if(err){
            cerr << "\033[1;31merr 011: \033[0m"<< file << " : " << temp2  <<" '"<< temp <<"' branch command dont can repeatet write 'nop' between"<< endl;
        }
        return true;
    }
    else{
        temp3 = temp3 | what_is_the_number(temp_number,temp2,file);
        contens.send.push_back(temp3);
        return false;
    }

    return false;
}
void locate_duplicates_macro(int line,const string file,const string macro){
   for(int i=0;i<contens.macro.size();i++){
        if(contens.macro[i]==macro){

            cerr << "\033[1;31merr 010: \033[0m"<< file << " : " << line  <<" '"<< macro <<"' this macro is already used"<< endl;
            error=true;
        }
    }
    return;
}
void locate_duplicates_marker(int line,const string file,const string marker){
   for(int i=0;i<contens.marker.size();i++){
        if(contens.marker[i]==marker){

            cerr << "\033[1;31merr 002: \033[0m"<< file << " : " << line  <<" '"<< marker <<"' this marker is already used"<< endl;
            error=true;
        }
    }
    return;
}
int first_run(const string file,int counter){
    string line;
    int line_counter=0,bra_counter=0;
    bool macro=false;
    ifstream inputFile(file);
    while (getline(inputFile, line))
    { 
        line_counter++;
        stringstream ss(line);
        string string1, string2;
        string temp="";
        bool bra_error=false;
        
        ss >> string1 >> string2;
        temp=str_lower(string1);

        string empty = trim_copy(string1);

        if(string1.find('#') == string::npos && !empty.empty()){
            if(temp=="ma:")
                macro=true;
            if(temp==":me"){
                macro=false;
                counter--;
            }
            if(macro && temp!="ma:"){
                locate_duplicates_macro(line_counter,file,temp);
                contens.macro.push_back(temp);
                contens.macro_value.push_back(what_is_the_number(str_lower(string2),line_counter,file));
            }
            if(macro)
                counter--;
            if(!macro && temp.size()>3){
                locate_duplicates_marker(line_counter,file,temp);
                contens.marker.push_back(temp);
                contens.marker_line.push_back(counter-bra_counter);
                if(counter-bra_counter>320&&counter-bra_counter<352){
                    cerr << "\033[1;31merr 012: \033[0m"<< file << " : " << line_counter  <<" '"<< temp <<"' a marker cant be between 320 and 352 im programcounter"<< endl;
                    error=true;
                }
                bra_counter++;
                bra_error=true;
            }
            else if(!macro && temp!=":me"){
                int temp2=what_is_the_command(string1,line_counter,file);
                if(temp2>0 && temp2 != 4 && temp2<9){
                    counter += 2;
                    if(bra_error){
                        cerr << "\033[1;31merr 013: \033[0m"<< file << " : " << line_counter  <<" '"<< string1 <<"' a branch command shoud not come after a marker but 'nop' betweem"<< endl;
                        error=true;
                    }  
                }
                bra_error=false;
            }
                
        }
        counter++;
    }
    if(macro){
        cerr<<"\033[1;31merr 009: \033[0m"<<file<<" :me is missing"<<endl;
        error=true;
    }
    inputFile.close();
    return counter-bra_counter;         
        
}
void second_run(const string file){
    string line;
    int line_counter=0;
    bool macro=false;
    bool bra_err=false;
    ifstream inputFile(file);
    while (getline(inputFile, line))
    { 
        line_counter++;
        stringstream ss(line);
        string string1, string2;
        string temp="";
        
        ss >> string1 >> string2;
        temp=str_lower(string1);

        string empty = trim_copy(string1);

        if(string1.find('#') == string::npos && !empty.empty()){
            if(temp=="ma:")
                macro=true;
            if(temp==":me"){
                macro=false;
            }
            if(!macro && temp!=":me"&&temp.size()<=3){
                int temp2=what_is_the_command(temp,line_counter,file);
                bra_err=create_binary(temp,str_lower(string2),line_counter,file,bra_err);
            }
                
        }
    }
    inputFile.close();
    return;         
 
}
int write_debug(string file,int counter){
    string line;
    int line_counter=0;
    bool macro=false;
    ifstream inputFile(file);
    while(getline(inputFile,line)){
        line_counter++;
        stringstream ss(line);
        string string1, string2;
        string temp="";
        
        ss >> string1 >> string2;
        temp=str_lower(string1);

        string empty = trim_copy(string1);


        if(string1.find('#') == string::npos && !empty.empty()){
             if(temp=="ma:")
                macro=true;
            if(temp==":me"){
                macro=false;
            }
            if(!macro && temp!=":me"&&temp.size()==3){
                printf("%4d", line_counter); 
                cout<<":  "<<string1<<"  |  "<<bitset<4>(contens.send[counter]>>4)<<"  "<<bitset<4>(contens.send[counter])<<endl;
                counter++;
                int temp2=what_is_the_command(string1,line_counter,file);
                if(temp2>0 && temp2 != 4 && temp2<9){
                    cout<<"   *:  "<<"***"<<"  |  "<<bitset<4>(contens.send[counter]>>4)<<"  "<<bitset<4>(contens.send[counter])<<endl;
                    counter++;
                    cout<<"   *:  "<<"***"<<"  |  "<<bitset<4>(contens.send[counter]>>4)<<"  "<<bitset<4>(contens.send[counter])<<endl;
                    counter++;
                }
            }               
        
        }
    }
    inputFile.close();
    return counter;
}
void serial_upload(string port){
    serialib serial;
    if (serial.openDevice(port.c_str(), 300) != 1)
    {
        cerr<<"\033[1;31merr 005: \033[0m"<<port<<" could not be opend"<<endl;
//        return;
    }
    for(int i=0;i<contens.send.size();i++){
        uint8_t sending[] = {contens.send[i]};
        serial.writeBytes(sending,1);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    serial.closeDevice();
    return;
}

