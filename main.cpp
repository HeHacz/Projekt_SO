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
#include<mutex>
#include<condition_variable>
#include<atomic>
#include <ncurses.h>
#include<form.h>
#include <assert.h>

#define SA (struct sockaddr*)
struct Wynik
{
  std::string adres_IP[30];
};


std::vector<char*> adresy_IP;
std::vector<char*> zywe_adresy_IP;
std::vector<int> porty;
std::vector<int> otwarte_porty;
std::vector<Wynik> wynik_tracerouta;
std::vector<char*> martwe_adresy_IP;
std::vector<int> zablokowane_porty;
std::vector<char*> portable_IP;
std::mutex mutex_;
std::mutex mutex__;
std::mutex mutex___;
std::mutex adresy_IPGuard;
std::mutex zywe_adresy_IPGuard;
std::mutex portable_IPGuard;
std::mutex martwe_adresy_IPGuard;
std::mutex portyGuard;
std::mutex otwarte_portyGuard;
std::mutex wynik_traceroutaGuard;
std::mutex zablokowane_portyGuard;
std::condition_variable cv1;
std::condition_variable cv2;
std::condition_variable cv3;
std::condition_variable cv4;
std::atomic<int> count(0);

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
  count++;
  return generator_liczb_losowych(1024); 
}

int ping(char* adres_IP)
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
  //std::cout<<"status> "<<status<<" , count: "<<count<<adres_IP<<std::endl;
  if (status == -EINTR)
      {
	//std::cout<<status<<std::endl;
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

std::string* traceroute(char *adres_IP)
{
  int sfd = socket (AF_INET, SOCK_RAW, IPPROTO_ICMP);
  char buf[4096] = { 0 };
  struct ip *ip_hdr = (struct ip *) buf;
  int hop = 0;
  int hop2 = 1;
    
  std::string* wynik = new std::string[30];
   int one = 1;
   wynik[0] = adres_IP;
   
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
	{
	 //printf ("hop limit:%d Address:%s\n", hop, inet_ntoa (addr2.sin_addr));
	wynik[hop2] = inet_ntoa(addr2.sin_addr);
        }
      else
	{
	  //printf ("Reached destination:%s with hop limit:%d\n",
	  //	  inet_ntoa (addr2.sin_addr), hop);
	  return wynik;
	}
      hop2++;
      hop++;
    }

  return 0;
}

static bool skaner_portow(int port , char *adres_IP)
{
  //std::string address = to_string(adres_IP);
  return (sf::TcpSocket().connect(adres_IP, port) == sf::Socket::Done);
}

bool martwe_adresy_IP_Is_empty()
{
  if(martwe_adresy_IP.empty())
    return false;
  else
    return true;
}

bool zywe_adresy_IP_Is_empty()
{
  if(zywe_adresy_IP.empty())
    return false;
  else
    return true;
}

bool adresy_IP_Is_No_empty()
{
  if(adresy_IP.empty())
    return false;
  else
    return true;
}

bool adresy_IP_Is_empty()
{
  if(adresy_IP.empty())
    return true;
  else
    return false;
  
}

bool porty_Is_empty()
{
  if (porty.empty())
    return true;
  else
    return false;
}

bool portable_IP_Is_empty()
{
  if(portable_IP.empty()&&!porty_Is_empty())
    return false;
  else
    return true;
}


void losuj_IPThread()
{
  std::cout<< "IP"<<std::endl;
  std::unique_lock<std::mutex> lock(zywe_adresy_IPGuard);
  cv3.wait(lock, adresy_IP_Is_empty);
  
  //std::this_thread::sleep_for(std::chrono::seconds(2));
  //adresy_IPGuard.lock();
  std::lock_guard<std::mutex> lk_1(adresy_IPGuard);
  std::this_thread::sleep_for(std::chrono::milliseconds(110));
  adresy_IP.push_back(losuj_IP());
  //adresy_IPGuard.unlock();
  cv1.notify_one();
}

void losuj_PortThread()
{
  std::cout<< "port"<<std::endl;
  
  //std::this_thread::sleep_for(std::chrono::seconds(2));
  portyGuard.lock();
  //std::this_thread::sleep_for(std::chrono::seconds(2));
  
  porty.push_back(losuj_port());
  portyGuard.unlock();
  cv2.notify_one();
}

void pingThread()
{
 
  // std::cout<< "ping"<<std::endl;
  auto now = std::chrono::system_clock::now();
  std::unique_lock<std::mutex> lock(mutex_);
  cv1.wait(lock);
  std::lock(adresy_IPGuard, zywe_adresy_IPGuard, martwe_adresy_IPGuard, portable_IPGuard);
  std::lock_guard<std::mutex> lk_1(adresy_IPGuard, std::adopt_lock);
  std::lock_guard<std::mutex> lk_2(zywe_adresy_IPGuard, std::adopt_lock);
  std::lock_guard<std::mutex> lk_3(martwe_adresy_IPGuard, std::adopt_lock);
  std::lock_guard<std::mutex> lk_4(portable_IPGuard, std::adopt_lock);
  
  if (ping(adresy_IP.back()) > 0)
    {
      zywe_adresy_IP.push_back(adresy_IP.back());
      portable_IP.push_back(adresy_IP.back());
      adresy_IP.pop_back();
    }
  else
    {
      martwe_adresy_IP.push_back(adresy_IP.back());
      adresy_IP.pop_back();
    }
  cv2.notify_one();
  cv3.notify_one();
}

void tracertThread()
{
  // std::cout<< "tracert"<<std::endl;
  auto now = std::chrono::system_clock::now();
  std::unique_lock<std::mutex> lock(mutex__);
  // std::unique_lock<std::mutex> lock(mutex4);
  cv2.wait_until(lock, now + std::chrono::seconds(20) , zywe_adresy_IP_Is_empty);
  // cv5.wait(lock);
  std::lock(zywe_adresy_IPGuard, wynik_traceroutaGuard);
  std::lock_guard<std::mutex> lk_1(zywe_adresy_IPGuard, std::adopt_lock);
  std::lock_guard<std::mutex> lk_2(wynik_traceroutaGuard, std::adopt_lock);
  if(zywe_adresy_IP_Is_empty())
    {
  std::string *wyniki, *cos;
  Wynik w;
  wyniki = traceroute(zywe_adresy_IP.back());
  zywe_adresy_IP.pop_back();
  int i = 0;
  std::cout<<wyniki[i]<<std::endl;
  while(!wyniki[i].empty())
    {
     i++;
    }
  cos = new std::string[i];
  for (int j=1;j<i+1; j++)
    {
      w.adres_IP[j] = wyniki[j];
      //  std::cout<< j <<" == " <<w.adres_IP[j] << " == " << wyniki[j]<<std::endl;
    }
  
  wynik_tracerouta.push_back(w);
    }
}

void skan_portowThread()
{
  std::cout<< "skan_port"<<std::endl;
  auto now = std::chrono::system_clock::now();
  std::unique_lock<std::mutex> lock(zywe_adresy_IPGuard);
  cv4.wait_until(lock, now + std::chrono::seconds(30) ,portable_IP_Is_empty);
  std::lock(portable_IPGuard, portyGuard, otwarte_portyGuard, zablokowane_portyGuard);
  
  std::lock_guard<std::mutex> lk_1(portable_IPGuard, std::adopt_lock);
  std::lock_guard<std::mutex> lk_2(portyGuard, std::adopt_lock);
  std::lock_guard<std::mutex> lk_3(otwarte_portyGuard, std::adopt_lock);
  std::lock_guard<std::mutex> lk_4(zablokowane_portyGuard, std::adopt_lock);
  if(portable_IP_Is_empty())
    {
  if(skaner_portow(porty.back(), portable_IP.back()))
    {
      otwarte_porty.push_back(porty.back());
      porty.pop_back();
      portable_IP.pop_back();
      // std::cout<<"open: "<<otwarte_porty.back()<<std::endl;
      
    }
    else
    {
      zablokowane_porty.push_back(porty.back());
      porty.pop_back();
      portable_IP.pop_back();
      // std::cout<<"close: "<<zablokowane_porty.back()<<std::endl;
    }
  // std::cout<<"koniec"<<std::endl;
  //cv5.notyfy_one;
    }
}

auto ncurses()
{
  std::vector< std::vector< std::string > > tekst;
  std::vector<std::string> wiersz;
  int counter = 1;
  char top[] = "  Internet MAP 0.2  ";
  std::string tab[] = {"Żywe adresy IP", "Martwe adresy IP", "Wynik tracerouta", "Otwarte Porty", "Zablokowane Porty"};
  char ch;


  initscr();
  cbreak();
 
  keypad(stdscr, TRUE);
  
  int height;
  int width;
  getmaxyx( stdscr, height, width);
  refresh();
  //win = newwin(height, width, starty, startx);
  //wborder(win, '|', '|', '-', '-', '+', '+', '+', '+');
  //box(win, 0, 0);
  //wrefresh(win);
  start_color(); //2
  init_pair( 1, COLOR_RED, COLOR_WHITE ); //3
  init_pair( 2, COLOR_BLACK,COLOR_GREEN );
  attron( COLOR_PAIR( 1 ) );
  move ( height / 200,((width)/2 - sizeof(top) / 2) );
  printw( "%s", top );
   attroff(COLOR_PAIR( 1 ) );
  attron(COLOR_PAIR( 2 ) );
  // printw("%u", rzedy);
  move(height/32,0);
  for(int i = 0; i<5; i++)
    {
      for(int e = 0; e<(width-sizeof(tab))/height*4; e++)
	 printw(" ");
       printw("%s", tab[i].c_str() );
       if(i == 2)
	 {
       for(int e = 0; e<(width-sizeof(tab))/3; e++)
	 printw(" ");
	 }
    }
  attroff(COLOR_PAIR( 2 ) );
  while((ch = getch()) != KEY_F(1))
    {
      int j = 1;
      int z = 0, m = 0, zab = 0, o = 0;
      
      counter++;
    } 
  refresh();
  getch();  
  endwin();
  return 0;

}

int main()
{
  //zywe_adresy_IP.push_back("8.8.8.8");
  int lw = 11;
  std::string *wynik;
  auto curses = std::thread(ncurses);
  std::vector<std::thread> threada;
  for(int i = 0; i < lw; i++)
    {
      threada.push_back(std::thread(losuj_IPThread));
    }
  std::vector<std::thread> threadb;
  for(int i = 0; i < lw; i++)
    {
      threadb.push_back(std::thread(losuj_PortThread));
    }
  std::vector<std::thread> threadc;
  for(int i = 0; i < lw; i++)
    {
      threadc.push_back(std::thread(pingThread));
    }
    std::vector<std::thread> threadd;
  for(int i = 0; i < lw; i++)
    {
      threadd.push_back(std::thread(tracertThread));
    }
  std::vector<std::thread> threade;
  for(int i = 0; i < lw; i++)
    {
      threade.push_back(std::thread(skan_portowThread));
    }
  
  for(auto& thread : threada)
    {
      thread.join();
    }
  for(auto& thread : threadb)
       {
	 thread.join();
       }
  for(auto& thread : threadc)
    {
      thread.join();
    }
  for(auto& thread : threadd)
    {
      thread.join();
      }
  for(auto& thread : threade)
    {
      thread.join();
      }
  
  curses.join();
  
  
  
  
  
  
  //std::cout<<adres_IP<<std::endl<<losuj_port()<<std::endl<<"ping: "<<ping(adres_IP)<<std::endl;
//std::cout<<skaner_portow(losuj_port(), adres_IP)<<std::endl<<"google.cos: 80 \
//"<<skaner_portow(53,"8.8.8.8")<<std::endl;

// tracertThread();
  //int a = 0;
  //for(auto const& i: wynik_tracerouta)
  /*{
     std::cout<< a << ": "<< i.adres_IP[a] <<std::endl;
     a++;
   }
  //std::cout<<traceroute(adres_IP)<<std::endl;
 */std::cout<<"hasło okoń"<<std::endl;
return 0;
}
