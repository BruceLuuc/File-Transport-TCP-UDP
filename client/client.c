#include "client_work.h"

int main(){
    struct command cmd;
    struct ip_port ip;
    
    get_server_list(&ip);
    menu(&ip);
    get_input(&cmd, &ip);
    
    return 0;
}


