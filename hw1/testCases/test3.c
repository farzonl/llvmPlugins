int main()
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