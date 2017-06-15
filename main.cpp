#include <iostream>
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



int main()
{  std::cout<<generator_liczb_losowych(1)<<std::endl<<losuj_IP()<<std::endl<<losuj_port()<<std::endl; 
}
