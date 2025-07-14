// 1. 幂运算
// 1.1 2的幂

bool solution(int x){
  if(x < 1){
    return false;
  }
  while(x % 2 == 0){
    x /= 2;
  }
  return (x == 1);
}
