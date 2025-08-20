// Copyright (C) IG @Jackcat1818.  All rights reserved.


#include <iostream>
#include <cpr/cpr.h>
#include <thread>
#include <chrono>


int encrypt(long long b, int e, int m);

const auto user_agent=cpr::Header{
    {"User-agent", "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/18.5 Safari/605.1.15"}
};

class type_handler{
    public:
        type_handler(std::string id, std::string pw){
            this->id=id;
            this->pw=pw;

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
        int w_cnt, cpm, time;

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
                    this->TypeHandler(0, 0);
                }
            }
            return;
        }
        void TypeHandler(int debug, int safe){
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
                {"type", 0},
                {"course_id", txt},
                {"n78bdbd373e111832", 12085},
                {"n61e771d9dd8eab3c", encrypt(w_cnt, 2411, 95477)},
                {"n043e1a918758a12f", 0}
            };

            if(safe==1) std::this_thread::sleep_for(std::chrono::seconds(65));

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
            std::cout << WritePageRes.text << std::endl;
            txt.clear();
        };

};


int main(){
    std::string str[7];

    str[1]="  _____          _     _                           _          _                    ";
    str[2]=" |  ___|   _ ___| |__ (_)_ __   __ _    __ _ _   _| |_ ___   | |_ _   _ _ __   ___ ";
    str[3]=" | |_ | | | / __| '_ \\| | '_ \\ / _` |  / _` | | | | __/ _ \\  | __| | | | '_ \\ / _ \\";
    str[4]=" |  _|| |_| \\__ \\ | | | | | | | (_| | | (_| | |_| | || (_) | | |_| |_| | |_) |  __/";
    str[5]=" |_|   \\__,_|___/_| |_|_|_| |_|\\__, |  \\__,_|\\__,_|\\__\\___/   \\__|\\__, | .__/ \\___|";
    str[6]="                               |___/                              |___/|_|         ";

    for(int i=1; i<=6; i++){
        std::cout << str[i] << std::endl;
    }

    std::string id, pw;
    std::cout << "id: ";
    std::cin >> id;
    std::cout << "pw: ";
    std::cin >> pw;

    auto typing=type_handler(id, pw);
    return 0;
}