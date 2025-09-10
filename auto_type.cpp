// Copyright (C) IG @Jackcat1818.  All rights reserved.


#include <iostream>
#include <cpr/cpr.h>
#include <thread>
#include <chrono>
#include <fstream>
#include <vector>

int encrypt(long long b, int e, int m);
int argv_handle(int argc, char** argv);
void bad_argument();

const auto user_agent=cpr::Header{
    {"User-agent", "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/18.5 Safari/605.1.15"}
};

class TypeHandler{
    public:
        TypeHandler(std::string id, std::string pw, int control_code, std::string file){
            this->control_code=control_code;
            this->id=id;
            this->pw=pw;

            if(control_code==1) FileHandler(file);
            else if(control_code==2) SettingHandler();
            else if(control_code==3) TestModeHandler();
            
            const auto SignInPayload=cpr::Multipart{
                {"id", id},
                {"pw", pw}
            };
            this->SignInRes=cpr::Post(
                cpr::Url("https://typing.fhjh.tp.edu.tw/connect.php"),
                SignInPayload,
                user_agent
            );
            if(id_pw_check()==false){
                return;
            }
            this->MainPageHandler();
        }
        
    private:
        int control_code;
        int w_cnt, cpm, time;

        int safe_c=1, debug_c=0;
        int mode_c=0;

        std::string id, pw;
        std::string class_id, txt;
        std::string sub_rand;
        std::string time_s;

        cpr::Response SignInRes;
        cpr::Response MainPageRes;
        cpr::Response TypePageRes;
        cpr::Response WritePageRes;

        cpr::Cookies PHPSessionID;

        int encrypt(long long b, int e, int m){
            int res=1;
            while(e>0){
                if(e%2==1){
                    res=(res*b)%m;
                }
                b=b*b%m;
                e=e/2;
            }
            return res;
        }
        bool id_pw_check(){
            char sign_in_c=SignInRes.text[SignInRes.text.find("url=")+4];
            if(sign_in_c=='i'){
                std::cerr << "[error] incorrect id or pw" << std::endl;
                return false;
            }else{
                std::cout << "[success]" << std::endl;
                return true;
            }
        };

        void ClassID_Finder(){
            int cnt=0;
            while(1){
                char class_id_char=MainPageRes.text[MainPageRes.text.find("class_id=")+9+cnt];
                if(class_id_char!='>'){
                    cnt++;
                    this->class_id+=class_id_char;
                }else{
                    break;
                }
            }
        }
        void SubRand_Finder(){
            int cnt=0;
            while(1){
                char sub_c=TypePageRes.text[TypePageRes.text.find("id=\"sub_rand\" value=\"")+21+cnt];
                if(sub_c!='\"'){
                    cnt++;
                    this->sub_rand+=sub_c;
                }else{
                    break;
                }
            }
        }
        void Time_Finder(){
            int cnt=0;
            while(1){
                char time_c=TypePageRes.text[TypePageRes.text.find("var typing_time = ")+18+cnt];
                if(cnt!=';'){
                    cnt++;
                    time_s+=time_c;
                }else{
                    break;
                }
            }
        }

        void MainPageHandler(){
            this->PHPSessionID=SignInRes.cookies;
            MainPageRes=cpr::Get(
                cpr::Url("https://typing.fhjh.tp.edu.tw/student/course.php"), 
                user_agent,
                PHPSessionID
            );
            std::cout << "[output] main page status code: "<< MainPageRes.status_code << std::endl;
            ClassID_Finder();
            std::cout << "[output] class id: " << this->class_id << std::endl;

            while(1){
                std::cout << "text (eg. E051): ";
                std::cin >> txt;
                std::cout << "score: ";
                std::cin >> cpm;

                if(MainPageRes.text.find(txt)>=18446744073709551){
                    std::cout << "[error] Invalid text: " << txt << std::endl;
                    std::exit(1);
                }else{
                    this->TypingPageHandler(debug_c, safe_c);
                }
            }
            return;
        }
        void TypingPageHandler(int debug, int safe){
            TypePageRes=cpr::Get(
                cpr::Url("https://typing.fhjh.tp.edu.tw/student/typing.php?course_id="+txt+"&class_id="+class_id),
                PHPSessionID,
                user_agent
            );

            if(debug==1) std::cout << TypePageRes.text << std::endl;
            SubRand_Finder();
            Time_Finder();
            time=std::stoi(time_s);
            std::cout << "[output] time: " << time << std::endl;
            std::cout << "[typing]..." << std::endl;

            w_cnt=(cpm*time/12);
            //std::cout << encrypt(194, 2411, 95477);

            const auto payload=cpr::Multipart{
                {"sub_rand", sub_rand},
                {"type", mode_c},
                {"course_id", txt},
                {"n78bdbd373e111832", encrypt(time, 2411, 95477)},
                {"n61e771d9dd8eab3c", encrypt(w_cnt, 2411, 95477)},
                {"n043e1a918758a12f", 0}
            };

            if(safe==1) std::this_thread::sleep_for(std::chrono::seconds(time+5));

            cpr::Header header=cpr::Header{
                {"Referer", "https://typing.fhjh.tp.edu.tw/student/typing.php?course_id="+txt+"&class_id="+class_id},
                {"User-Agent", "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/18.5 Safari/605.1.15"}
            };
            WritePageRes=cpr::Post(
                cpr::Url("https://typing.fhjh.tp.edu.tw/student/write.php"),
                header,
                payload,  
                this->PHPSessionID
            );
            if(debug==1) std::cout << WritePageRes.text << std::endl;
            ResultHandler(WritePageRes.text);
            txt.clear();
        };
        void FileHandler(std::string file){
            std::ifstream reader;
            reader.open(file);
            if(reader.fail()) std::cout << "[error] file: " << file << " not found" << std::endl;
            if(!reader.is_open()) std::cout << "[error] file: " << file << " not found" << std::endl;
            std::string data[2];
            reader >> data[0];
            reader >> data[1];

            int in0=data[0].find("debug:");
            int in1=data[1].find("safe:");
            if(in0>10000||in1>10000){
                std::cout << "[error] debug file argument error" << std::endl;
                return;
            }
            try{
                debug_c=data[0][in0+6]-'0';
                safe_c=data[1][in1+5]-'0';
                debug_c=debug_c>1? 1:debug_c;
                safe_c=safe_c>1? 1:safe_c;
                debug_c=debug_c<0? 0:debug_c;
                safe_c=safe_c<0? 0:safe_c;
            }catch(std::exception& e){
                std::cout << e.what() << std::endl;
                std::cout << "[error] debug file argument error" << std::endl;
            }
            reader.close();
        };
        void ResultHandler(std::string& response){
            int index=response.find("error=");
            if(index==-1) std::cout << "[typing] finish typing!" << std::endl;
            else{
                int error_code=response[index+6]-'0';
                switch(error_code){
                    case 1:
                        std::cout << "[error] too many error" << std::endl;
                        break;
                    case 2:
                        std::cout << "[error] too little word" << std::endl;
                        break;
                    case 3:
                        std::cout << "[error] something wrong" << std::endl;
                        break;
                    case 4:
                        std::cout << "[error] wpm too low" << std::endl;
                        break;
                    case 5:
                        std::cout << "[error] wpm too high" << std::endl;
                        break;
                    case 6:
                        std::cout << "[error] result repeated" << std::endl;
                        break;
                    case 7:
                        std::cout << "[error] time<240s" << std::endl;
                        break;
                    case 8:
                        std::cout << "[error] something wrong" << std::endl;
                        break;
                }
            }
        }
        void SettingHandler(){
            std::string r1, r2;
            std::cout << "[setting] do you want to see the raw responce? (y/n): ";
            std::cin >> r1;
            std::cout << "[setting] do you want to disable safe typing? (y/n): ";
            std::cin >> r2;
            if(r1=="y") debug_c=1;
            if(r2=="y") safe_c=0;
        }
        void TestModeHandler(){
            std::cout << "[WARNING] you just enter the test mode" << std::endl;
            std::cout << "[WARNING] this is NEVER TESTED before" << std::endl;
            std::cout << "[WARNING] do you want to continue(Y/n): ";
            char c;
            std::cin >> c;
            if(c=='Y'){
                std::cout << "[output] test mode entered" << std::endl;
                mode_c=1;
            }else{
                std::cout << "[output] normal mode entered" << std::endl;
            }
        }
};


int main(int argc, char** argv){
    std::string file="";
    int code=argv_handle(argc, argv);
    if(code==1) file=argv[2];
    else if(code==-1) bad_argument();

    std::string str[6];
    str[0]="  _____          _     _                           _          _                    ";
    str[1]=" |  ___|   _ ___| |__ (_)_ __   __ _    __ _ _   _| |_ ___   | |_ _   _ _ __   ___ ";
    str[2]=" | |_ | | | / __| '_ \\| | '_ \\ / _` |  / _` | | | | __/ _ \\  | __| | | | '_ \\ / _ \\";
    str[3]=" |  _|| |_| \\__ \\ | | | | | | | (_| | | (_| | |_| | || (_) | | |_| |_| | |_) |  __/";
    str[4]=" |_|   \\__,_|___/_| |_|_|_| |_|\\__, |  \\__,_|\\__,_|\\__\\___/   \\__|\\__, | .__/ \\___|";
    str[5]="                               |___/                              |___/|_|         ";

    for(int i=0; i<6; i++){
        std::cout << str[i] << std::endl;
    }

    std::string id, pw;
    std::cout << "id: ";
    std::cin >> id;
    std::cout << "pw: ";
    std::cin >> pw;

    TypeHandler typing(id, pw, code, file);
    return 0;
}

int argv_handle(int argc, char** argv){
    if(argc>1){
        std::string arg1=argv[1];
        if(argc==2){
            if(arg1=="-s") return 2;
            else if(arg1=="-t") return 3;
            else return -1;
        }
        else if(argc==3){
            if(arg1=="-d") return 1;
            else return -1;
        }
    }else{
        return 0;
    }
    return 0;
}

void bad_argument(){
    std::cout << "[error] bad argument" << std::endl;
    std::cout << "[output] continue normal procedure" << std::endl;
}
