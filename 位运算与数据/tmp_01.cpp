// 1. 幂运算
// 1.1 2的幂

bool solution_for_2(int x){
  if(x < 1){
    return false;
  }
  while(x % 2 == 0){
    x /= 2;
  }
  return (x == 1);
}

// 1.2 3的幂
bool solution_for_3(int x){
  if(x < 1){
  return false;
  }

  while(x % 3 == 0){
    x /= 3;
  }
  return (x == 1);
}

// 1.3 4 的幂
bool solution_for_4(int x){
  if(x < 1){
    return false;
  }
  while(x % 4 == 0){
    x /= 4;
  }
  return (x == 1);
}

// 1.4 pow(x, n)    采用递归+二分法
// y^n   
// 当 n 是偶数时 y^n = (y^n/2))^2
// 当 n 是奇数时 y^n = ((y^n/2))^2 * y    具体的 n =3   y^3 = (y^1)^2*y
int func(int x, int y){
  if(y == 0){
    return 1;
  }
  int tmp = func(x, y/2);
  return y % 2 == 0 ? tmp * tmp : tmp * tmp * x;
}
bool solution_func(int x, int n){
  unsigned int y = n;
  return n > 0 ? func(x, n) : 1.0 /func(x, -y);
}
