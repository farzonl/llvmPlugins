int /*multiEnterLoop*/main()
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

/*int main()
{
    return 0;
}*/