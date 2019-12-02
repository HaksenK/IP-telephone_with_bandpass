#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/rational.hpp>
#include <iostream>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <cassert>
#include <boost/operators.hpp>
#include <string>

//Bint と long は衝突する。

using namespace std;
namespace mp = boost::multiprecision;
// 任意長整数型
using Bint = mp::cpp_int;

long extGCD(long a, long b, long &x, long &y) {
    if (b == 0) {
        x = 1;
        y = 0;
        return a;
    }
    Bint d = extGCD(b, a%b, y, x);
    y -= a/b * x;
    return (long)d;
}



long gcd(long a, long b){
  if(a<b){
    long temp=a;
    a=b;
    b=temp;
  }
  long c;
  while(a%b!=0){
    c=a%b;
    a=b;
    b=c;
  }
  return b;
}

long pow_mod(long a, long d, long n){  //a^d%n
  int digit=0;
  Bint ans=1;
  Bint temp=a;

  while(d >= ((Bint)1 << digit)){//cast注意!castする場所も。
    if(d&((Bint)1<<digit)){
      ans=ans*temp%n;
    }
    temp=temp*temp%n;
    digit++;
  }
  return (long)ans;
}

int main(int argc, char **argv){
    //sは使っちゃだめ
    int num=1000;
    // long p=1419978377;
    // long q=1420314517;
    long p=2031910591;
    long q=2109172253;
    long n=p*q;
    long l=(p-1)*(q-1);
    long e,d;
    long index=2;
    while(1){
      if(gcd(l, (long)index)==1){
        e=index;
        break;
      }
      index++;
    }

    long N;
    extGCD(e, l, d, N);
    if(d<0){
      d=d+l;
    }



    if(argc!=3 && argc!=4){
        perror("./net_phone\n");
        exit(1);
    }
    int port;
    if(argc==3){
        port = atoi(argv[1]);
    }
    if(argc==4){
        port = atoi(argv[2]);
    }
    int s, ss;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if(argc==3){
        printf("serve\n");
        ss=socket(PF_INET, SOCK_STREAM, 0);
        if(ss==-1){
            perror("serve_socket == -1");
            exit(1);
        }
        addr.sin_addr.s_addr = INADDR_ANY;
        int bin=bind(ss, (struct sockaddr *)&addr, sizeof(addr));
        if(bin==-1){
            perror("bind == -1");
            exit(1);
        }
        listen(ss, 10);

        struct sockaddr_in client_addr;
        socklen_t len=sizeof(struct sockaddr_in);
        s=accept(ss, (struct sockaddr *)&client_addr, &len);
        if(s==-1){
            perror("accept\n");
            exit(1);
        }
    }

    if(argc==4){
        printf("client\n");
        s=socket(PF_INET, SOCK_STREAM, 0);
        if(s==-1){
            perror("client_socket == -1");
            exit(1);
        }
        inet_aton(argv[1], &addr.sin_addr);
        int ret=connect(s,(struct sockaddr *)&addr, sizeof(addr));
        if(ret==-1){
            perror("connect == -1");
            exit(1);
        }
    }

    long data_send[num];
    long data_send_rsa[num];
    long data_recv_rsa[num];
    long data_recv[num];

    string cmd = "rec -t raw -b 16 -c 1 -e s -r 44100 - | ./bandpass 8192 200 1500 ";
    cmd += argv[argc-1];
    FILE *fp=popen(cmd.c_str(), "r");
    if(fp==NULL){
        perror("popen\n");
        exit(1);
    }
    while(fread(data_send, sizeof(long), num, fp)!=EOF){
        //暗号化 M<nである必要がある。
        for(int i=0; i<num; i++){
            data_send_rsa[i]=pow_mod(data_send[i], e, n);
        }
        int sen=send(s, data_send_rsa, sizeof(long)*num, 0);
        if(sen==-1){
            perror("serv_send\n");
        }
        int rec=recv(s, data_recv_rsa, sizeof(long)*num, 0);
        //復号
        for(int i=0; i<num; i++){
            data_recv[i]=pow_mod(data_recv_rsa[i], d, n);
        }
        if(rec==-1){
            perror("serv_recv\n");
            exit(1);
        }
        if(rec==0){
            break;
        }
        write(1, data_recv, rec);
    }

    if(argc==2){
        close(ss);
    }
    pclose(fp);
    close(s);

    return 0;
}
