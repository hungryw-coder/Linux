#include <iostream>
#include <mysql/mysql.h>
#include <random>

using std::cout;
using std::endl;

char* GenRandomString(int length)  {  
    int flag, i;  
    char* string;  
    srand((unsigned) time(NULL ));  
    if ((string = (char*) malloc(length)) == NULL )  {  
        printf("malloc failed!flag:14\n");  
        return NULL ;  
    }  
    for (i = 0; i < length+1; i++)  {  
        flag = rand() % 3;  
        switch (flag)  {  
            case 0:  string[i] = 'A' + rand() % 26;  break;  
            case 1:  string[i] = 'a' + rand() % 26;  break;  
            case 2:  string[i] = '0' + rand() % 10;  break;  
            default:  string[i] = 'x';  break;  
        }
    }
    string[length] = '\0';
    return string;  
} 


// 随机盐值生成
std::string generateSalt(int length = 8) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, sizeof(alphanum) - 2);
    
    std::string salt;
    for (int i = 0; i < length; ++i) {
        salt += alphanum[dis(gen)];
    }
    return salt;
}

int main()
{   
    cout << generateSalt() << endl;
    cout << GenRandomString(8) << endl;
    return 0;
}

