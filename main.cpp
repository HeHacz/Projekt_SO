#include <iostream>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/signal.h>
#include <thread>
#include <iostream>
#include <string>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <limits>
#include <random>
#include <vector>
#include <functional>
#include <cmath>
#include <oping.h>
#define DEFDATALEN      56
#define MAXIPLEN        60
#define MAXICMPLEN      76

char **adresy_IP;
char *zywe_adresy_IP;
int *porty;
int *otwarte_porty;
std::string *wynik_tracerouta;


auto generator_liczb_losowych(int koniec)
{
  auto aktualny_czas = std::chrono::system_clock::now().time_since_epoch().count();
  std::mt19937 silnik_losujacy_liczby(aktualny_czas);
  std::uniform_int_distribution<int> rozklad_liczb_losowych(0,koniec);
  auto l_losowa = std::bind(rozklad_liczb_losowych, silnik_losujacy_liczby);
  return l_losowa();
}

auto losuj_IP()
{
  std::string pom;
  pom = std::to_string(generator_liczb_losowych(255)) + "." + std::to_string(generator_liczb_losowych(255)) + "\
." + std::to_string(generator_liczb_losowych(255)) + "." + std::to_string(generator_liczb_losowych(255));
  char *adres_IP = &pom[0];
  return adres_IP;
}

auto losuj_port()
{
  return generator_liczb_losowych(1024);
}

bool ping(char *adres_IP)
{
  pingobj_t *ping;
  pingobj_iter_t *iter;
  //ping_host_add(ping, "8.8.8.8");
  if ((ping = ping_construct ()) == NULL)
    {
      fprintf (stderr, "ping_construct failed\n");
      return (1);
    }
  if (ping_host_add (ping, adres_IP ) < 0)
    {
      const char *errmsg = ping_get_error (ping);

      fprintf (stderr, "Adding host %c failed: %s\n", adres_IP, errmsg);
    }
  else
    {
      std::cout<<"ok - "<<adres_IP<<"\n";
    }	
}


int main()
{  std::cout<<generator_liczb_losowych(1)<<std::endl<<losuj_IP()<<std::endl<<losuj_port()<<std::endl<<ping(losuj_IP())<<std::endl; 
}
