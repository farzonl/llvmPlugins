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
                   //                                 | BLOCK 1                   | CFG EDGE 1
	if (b == 2)      //Jump instruction                 |
	{
		++b;           //Jump target                      | BLOCK 2
	}
	                 //                                                             | CFG EDGE 2
	int c = 3;       //Sequential instructions          |
	int d = 4;       //-----------------------          | BLOCK 3
	                 //                                                             | CFG EDGE 3,4 (end of block and loop entry)
	while (a < 5)    //Jump instruction and jump target | BLOCK 4 | LOOP BLOCK 1
	{                //                                                             | CFG EDGE 5   (loop body)
		++a;           //Jump target                      | BLOCK 5 | LOOP BLOCK 2
	}                //                                                             | CFG EDGE 6 (loop exit)
	
	int e = 5;       //Sequential instructions |
	int f = 6;       //----------------------- | BLOCK 6                            | CFG EDGE 7
}

int whileLoopTest()
{
  int i = 0;           // | BLOCK 1
  while(i < 10)        // | BLOCK 2   | LOOP BLOCK 1
  {
    i++;               // | BLOCK 3   | LOOP BLOCK 2
  }
  return 0;            // | BLOCK 4
}

int whileLoopTestGOTO()
{
  int i = 0;
  START:       
  while(i < 10)    
  {
    i++;
    if(i == 5)
    {
      goto START;
    }
  }
  return 0;
}   

int threewhileLoopTest()
{
  int i = 0;           // | BLOCK 1
  while(i < 10)        // | BLOCK 2   | LOOP BLOCK 1
  {
    i++;               // | BLOCK 3   | LOOP BLOCK 2
  }
  i = 0;               // | BLOCK 4
  while(i < 10)        // | BLOCK 5   | LOOP BLOCK 3
  {
    i++;               // | BLOCK 6   | LOOP BLOCK 4
  }
  i = 0;               // | BLOCK 7
  while(i < 10)        // | BLOCK 8   | LOOP BLOCK 5
  {
    i++;               // | BLOCK 9   | LOOP BLOCK 6
  }
  return 0;            // | BLOCK 10
}

int nestedwhileLoopTest()
{
  int i = 0;           // | BLOCK 1
  while(i < 10)        // | BLOCK 2   | LOOP BLOCK 1
  {
    int j = 0;         // | BLOCK 3   | LOOP BLOCK 2
    while (j < 5)      // | BLOCK 4   | LOOP BLOCK 3
    {
      j++;             // | BLOCK 5   | LOOP BLOCK 4
    }
    i++;               // | BLOCK 6
  }
  return 0;            // | BLOCK 7
}

int twoforLoopTest()
{
  int a = 0;
  for (int i = 0; i < 5; i++)
  {
    a = (a+1) + a;
  }
  for (int i = 0; i < 5; i++)
  {
    a = (a+1) + a;
  }
  return a;
}

int nestedforLoopTest()
{
  int a = 0;
  for (int i = 0; i < 5; i++)
  {
    for (int j = 0; j < 5; j++)
    {
      a = (a+1) + a;
    }
  }
  
  return a;
}

int nestedforLoopBreakTest()
{
  int a = 0;
  for (int i = 0; i < 5; i++)
  {
    for (int j = 0; j < 5; j++)
    {
      break;
    }
  }
  
  return a;
}

int twoforLoopBreakTest()
{
  int a = 0;
  for (int i = 0; i < 5; i++)
  {
    break;
  }
  for (int i = 0; i < 5; i++)
  {
    break;
  }
  return a;
}

int whileLoopBreakTest()
{
  int i = 0;
  while(i < 10)
  {
    break;
  }
  return 0;
}


// test case for 3.1.1
int nestedForLoopsExtreme()
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
}

//test cases for 3.1.3
int returnInsideForLoopTest()
{
  for (int i=0;i<20;i++)
  {
    return 3;
  }
  return 0;
}

int returnInsideForLoopTest2()
{
  for (int i=0;i<20;i++)
  {
    if(i ==2)
      return 3;
  }
  return 0;
}

int multiReturnInsideForLoopTest2()
{
  for (int i=0;i<20;i++)
  {
    if(i ==4)
      return 5;

    if (i == 2)
      return 3;

    if(i ==1)
      return 2;
  }
  return 0;
}

int nestedforLoopBreakThenReturnTest()
{
  int a = 0;
  for (int i = 0; i < 5; i++)
  {
    for (int j = 0; j < 5; j++)
    {
      if (j == 2)
        break;
    }
    if(i == 4)
      return 5;
  }
  return 0;
}
int multinestedforLoopReturnTest()
{
  int a = 0;
  for (int i = 0; i < 5; i++)
  {
    for (int j = 0; j < 5; j++)
    {
      if(i ==4)
      return 5;

      if (i == 2)
        return 3;

      if(i ==1)
        return 2;
    }
  }
  return 0;
}

int multinestedforLoopBreakTest()
{
  int a = 0;
  for (int i = 0; i < 5; i++)
  {
    for (int j = 0; j < 5; j++)
    {
      if(i == 4)
        break;

      if (i == 2)
        break;

      if(i == 1)
        break;
    }
  }
  return 0;
}

int dependence()
{
  int A = 1;
  int B = 2;
  int C = A + B;
  int D = C - A;

  D = B * D;
  D = A + B;
  int E = 0;
  E = E + 1;
  B = A + B;
  E = C - A;
  A = B * D;
  B = A - D;
  return 0;
}

int multiEnterLoop()
{
        int i;

        i = 0;
        if (i < 10)
                goto part2;
part1:
        if (i > 10)
                goto part3;
        goto part2;

part2:
        i++;
        goto part1;

part3:
        return 0;
}