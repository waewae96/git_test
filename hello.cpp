#include<iostream>

using namespace std;

int main(void){
  int sum = 0;
  for(int i = 0; i < 10; i++){
    sum += i+1;
  }
  cout << sum << endl;
  return 0;
}
