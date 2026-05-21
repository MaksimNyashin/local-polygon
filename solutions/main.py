def main():
    t = int(input())
    a, b, c = (int(i) for i in input().split())
    t -= 2
    print(-t * (a + 2 * b + 3 * c))
    from time import sleep
    if t == 0:
        sleep(5)
    if t == 1:
        print(1/0)

main()
