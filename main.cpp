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
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<unistd.h>
#include<pthread.h>
#include<poll.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<signal.h>
#include<sys/sem.h>
#include<poll.h>
#include<pthread.h>
#include<sys/select.h>
#include<sys/un.h>
#include <SFML/Network.hpp>

#define SA (struct sockaddr*)
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

int ping(char *adres_IP)
{
  pingobj_t *ping;
  pingobj_iter_t *iter;
  //ping_host_add(ping, "8.8.8.8");
  if ((ping = ping_construct ()) == NULL)
    {
      fprintf (stderr, "ping_construct failed\n");
      return (-1);
    }
  if (ping_host_add (ping, adres_IP ) < 0)
    {
      const char *errmsg = ping_get_error (ping);

      fprintf (stderr, "Adding host %s failed: %s\n", adres_IP, errmsg);
    }
  else
    {
    }
  int status = ping_send(ping);
  const char *count = ping_get_error (ping);
  std::cout<<"status> "<<status<<" , count: "<<count<<std::endl;
  if (status == -EINTR)
      {
	std::cout<<status<<std::endl;
	return (1);
      }
    else if (status < 0)
      {
	return(-1);
      }
  return(status);
}

unsigned short csum (unsigned short *buf, int nwords)
{
  unsigned long sum;
  for (sum = 0; nwords > 0; nwords--)
    sum += *buf++;
  sum = (sum >> 16) + (sum & 0xffff);
  sum += (sum >> 16);
  return ~sum;
}

auto traceroute(char *adres_IP)
{
  int sfd = socket (AF_INET, SOCK_RAW, IPPROTO_ICMP);
  char buf[4096] = { 0 };
  struct ip *ip_hdr = (struct ip *) buf;
  int hop = 0;

  int one = 1;
  const int *val = &one;
  if (setsockopt (sfd, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0)
    printf ("Cannot set HDRINCL!\n");

  struct sockaddr_in addr;
  addr.sin_port = htons (7);
  addr.sin_family = AF_INET;
  inet_pton (AF_INET, adres_IP, &(addr.sin_addr));
  while (1)
    {
      ip_hdr->ip_hl = 5;
      ip_hdr->ip_v = 4;
      ip_hdr->ip_tos = 0;
      ip_hdr->ip_len = 20 + 8;
      ip_hdr->ip_id = 10000;
      ip_hdr->ip_off = 0;
      ip_hdr->ip_ttl = hop;
      ip_hdr->ip_p = IPPROTO_ICMP;
      inet_pton (AF_INET, "46.105.30.91", &(ip_hdr->ip_src));
      inet_pton (AF_INET, adres_IP, &(ip_hdr->ip_dst));
      ip_hdr->ip_sum = csum ((unsigned short *) buf, 9);

      struct icmphdr *icmphd = (struct icmphdr *) (buf + 20);
      icmphd->type = ICMP_ECHO;
      icmphd->code = 0;
      icmphd->checksum = 0;
      icmphd->un.echo.id = 0;
      icmphd->un.echo.sequence = hop + 1;
      icmphd->checksum = csum ((unsigned short *) (buf + 20), 4);
      sendto (sfd, buf, sizeof(struct ip) + sizeof(struct icmphdr), 0, SA & addr,\
	      sizeof addr);
      char buff[4096] = { 0 };
      struct sockaddr_in addr2;
      socklen_t len = sizeof (struct sockaddr_in);
      recvfrom (sfd, buff, sizeof(buff), 0, SA & addr2, &len);
      struct icmphdr *icmphd2 = (struct icmphdr *) (buff + 20);
      if (icmphd2->type != 0)
	printf ("hop limit:%d Address:%s\n", hop, inet_ntoa (addr2.sin_addr));
      else
	{
	  printf ("Reached destination:%s with hop limit:%d\n",
		  inet_ntoa (addr2.sin_addr), hop);
	  exit (0);
	}

      hop++;
    }

  return 0;
}

static bool skaner_portow(int port , char *adres_IP)
{
  //std::string address = to_string(adres_IP);
  return (sf::TcpSocket().connect(adres_IP, port) == sf::Socket::Done);
}  

int main()
{
   
  char *adres_IP = losuj_IP();
  std::cout<<adres_IP<<std::endl<<losuj_port()<<std::endl<<"ping: "<<ping(adres_IP)<<std::endl;
  std::cout<<traceroute(adres_IP)<<std::endl;
    std::cout<<skaner_portow(losuj_port(), adres_IP)<<std::endl<<"google.cos: 80 "<<skaner_portow(55,"8.8.8.8")<<std::endl;
    return 0;
}
