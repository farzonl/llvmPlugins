void func1() {
  int i;
  while (i < 10)
  {
    i++;
  }
}

void func2() {
  int i = 0;
  while (i < 10)
  {
    int j;
    i++;
    int h = j + i;
  }
}

void func3() {
  int i;
  int n = 0;
  if (n < 1)
  {
    i = 0;
  }
  int j = i + 1;
}

void func4() {
  int i;
  int n = 0;
  if (n > 1)
  {
    i = 0;
  }
  int j = i + 1;
}

void busyFunc() {
  int a = 0;
  int b = 2;
  int c = 1;
  a = b + c;
  if (a < b) {
      b = a - c;
  } else {
      while (a > b) {
          c = b + c;
          if (a < b) {
            c = a * b;
            int f = a - b;
          }
          else
          {
            while(a > b) {
              int f = b + c;
            }
          }
          int g = a + b;
      }
  }
  int h = a - c;
  int f = b + c;
}

int main() {

  int a;
  int b = 0;
  int c = 1;
  int d = b + c;
  int e = b + a;
  int f = c + a;
  return c;
}
