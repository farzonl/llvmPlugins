#include <stdio.h>

int ifElseTest()
{
  int a = 4;
  if(a%2 == 0)
  {
    int b = a;
  }
  else
  {
    int c = a;
  }
  return a;
}

int switchTest()
{
  int a = 4;
  int b = 0;
  switch (a)
  {
    case 1:
      b = a;
      break;
    case 2:
      b = a;
      break;
    case 3:
      b = a;
      break;
    case 4:
      b = a;
      break;
  }
  return b;
}

int main(int argc,char**argv)
{
  int a = 1;       //Sequential instructions          |
	int b = 2;       //-----------------------          |
                   //                                 | BLOCK 1
	if (b == 2)      //Jump instruction                 |
	{
		++b;           //Jump target                      | BLOCK 2
	}
	
	int c = 3;       //Sequential instructions          |
	int d = 4;       //-----------------------          | BLOCK 3
	
	while (a < 5)    //Jump instruction and jump target | BLOCK 4
	{
		++a;           //Jump target                      | BLOCK 5
	}
	
	int e = 5;       //Sequential instructions |
	int f = 6;       //----------------------- | BLOCK 6
}

/*int main()
{
  int i = 0;
start: printf("start");
  while(i < 10)
  {
    printf("count is %d\n",i);
    i++;
    goto start;
  }
}*/
/*
int main(int argc,char**argv)
{
  
  int i,j,k,t=0;
   for (i=0;i<10;i++)
  {
     for (j=0;j<10;j++)
    {
       for (k=0;k<10;k++)
      {
         t++;
        
      }
      
    }
     for (j=0;j<10;j++)
    {
       t++;
      
    }
    
  }
  for (i=0;i<20;i++)
    
    {
       for (j=0;j<20;j++)
      {
         t++;
        
      }
       for (j=0;j<20;j++)
      {
         t++;
        
      }
      
    }
   return t;
  
}*/

